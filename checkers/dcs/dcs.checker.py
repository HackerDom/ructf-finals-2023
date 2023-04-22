#!/usr/bin/env python3.11

import gornilo
import dataclasses
import json
import subprocess
import requests
import hashlib
import random
import string
import logging

import api

from typing import Callable

checker = gornilo.NewChecker()

PORT = 7654


@dataclasses.dataclass
class TestSuite:
    code: str
    result: str
    error: str


@dataclasses.dataclass
class FlagId:
    token: str
    evaluation_result: str

    def serialize(self) -> str:
        return json.dumps({'token': self.token, 'evaluation_result': self.evaluation_result})

    @staticmethod
    def parse(text: str) -> 'FlagId':
        j = json.loads(text)
        if 'token' not in j:
            raise api.DCSProtocolError(f'invalid json {text}')
        if 'evaluation_result' not in j:
            raise api.DCSProtocolError(f'invalid json {text}')

        return FlagId(j['token'], j['evaluation_result'])


def generate_test_suite(mode: str) -> TestSuite:
    j = json.loads(subprocess.check_output(
        f'./dcs_test_generator {mode} 2>/dev/null', shell=True, timeout=0.5))

    return TestSuite(j['code'], j['result'], j['error'])


def sha256(s: str) -> str:
    return hashlib.sha256(s.encode('utf-8')).hexdigest()


def random_string(length: int, alpha: str = string.ascii_letters + string.digits) -> str:
    symbols = random.choices(alpha, k=length)

    return ''.join(symbols)


def do_put(request: gornilo.PutRequest) -> gornilo.Verdict:
    with api.DCSClient(request.hostname, PORT) as c:
        test_suite = generate_test_suite('get')
        logging.info(f'test: {test_suite} description: {request.flag}')
        if test_suite.error != '':
            raise api.DCSValidationError('test generator created error suite')

        res = c.create(test_suite.code, request.flag)

        logging.info(f'create = {res}')

        if res.error != '':
            return gornilo.Verdict.MUMBLE('unexpected compilation error')

        logging.info(f'token: {res.token}')

        flag_id = FlagId(res.token, test_suite.result)

        return gornilo.Verdict.OK_WITH_FLAG_ID(sha256(res.token), flag_id.serialize())


def do_get(request: gornilo.GetRequest) -> gornilo.Verdict:
    with api.DCSClient(request.hostname, PORT) as c:
        flag_id = FlagId.parse(request.flag_id)
        logging.info(f'flag_id: {flag_id}')

        res = c.get(flag_id.token)
        logging.info(f'get = {res}')

        if res.error == 'invalid flag':
            return gornilo.Verdict.CORRUPT('lost flag')

        if res.error != '':
            raise api.DCSProtocolError(f'unexpected GET error: {res.error}')

        if request.flag != res.description:
            return gornilo.Verdict.CORRUPT('invalid flag')

        if flag_id.evaluation_result.strip() != res.executeResult.strip():
            return gornilo.Verdict.MUMBLE('invalid evaluation result')

        if sha256(flag_id.token) != request.public_flag_id:
            return gornilo.Verdict.MUMBLE('invalid public flag id')

        return gornilo.Verdict.OK()


def do_check(request: gornilo.CheckRequest) -> gornilo.Verdict:
    with api.DCSClient(request.hostname, PORT) as c:
        test_suite = generate_test_suite('check')
        description = random_string(random.randint(64, 128))

        logging.info(f'test: {test_suite}, flag: {description}')

        res = c.create(test_suite.code, description)

        logging.info(f'create = {res}')

        if res.error != '' or test_suite.error != '':
            if test_suite.error != res.error:
                return gornilo.Verdict.MUMBLE('invalid compilation error text')
            return gornilo.Verdict.OK()

        res = c.get()

        logging.info(f'get = {res}')

        if res.error == 'invalid flag':
            return gornilo.Verdict.CORRUPT('lost flag')
        
        if res.error != '':
            raise api.DCSProtocolError(f'unexpected GET error: {res.error}')

        if res.description != description:
            return gornilo.Verdict.CORRUPT('invalid flag')

        if test_suite.result not in (res.error, res.executeResult):
            return gornilo.Verdict.MUMBLE('invalid execution result')

        return gornilo.Verdict.OK()


def wrap_exceptions(
        action: Callable[[gornilo.CheckRequest], gornilo.Verdict],
        request: gornilo.CheckRequest,
) -> gornilo.Verdict:
    try:
        return action(request)
    except api.DCSProtocolError as error:
        return gornilo.Verdict.MUMBLE(f'protocol error: {error}')
    except api.DCSValidationError as error:
        return gornilo.Verdict.MUMBLE(f'validation error: {error}')
    except requests.ConnectionError:
        return gornilo.Verdict.DOWN('connection error')
    except requests.Timeout:
        return gornilo.Verdict.DOWN('timeout error')
    except requests.RequestException:
        return gornilo.Verdict.MUMBLE('http error')


@checker.define_check
def check(request: gornilo.CheckRequest) -> gornilo.Verdict:
    return wrap_exceptions(do_check, request)


@checker.define_vuln('flag_id is sha256(token)')
class DCSChecker(gornilo.VulnChecker):
    @staticmethod
    def put(request: gornilo.PutRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_put, request)

    @staticmethod
    def get(request: gornilo.GetRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_get, request)


if __name__ == '__main__':
    checker.run()
