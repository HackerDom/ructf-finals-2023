#!/usr/bin/env python3

import random
import string
from typing import Callable

import gornilo
import requests

import client


port = 17173
checker = gornilo.NewChecker()


def random_string(length: int) -> str:
    alphabet = string.ascii_letters + string.digits
    symbols = random.choices(alphabet, k = length)

    return ''.join(symbols)


def do_put(request: gornilo.PutRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    username = random_string(16)
    password = random_string(32)

    if not api.register(username, password):
        return gornilo.Verdict.MUMBLE('failed to register')

    if not api.login(username, password):
        return gornilo.Verdict.MUMBLE('failed to login')

    message = request.flag.encode()
    ciphertext_id = api.encrypt(username, password, message)

    flag_id = '|'.join([username, password, ciphertext_id])
    public_flag_id = '|'.join([username, ciphertext_id])

    return gornilo.Verdict.OK_WITH_FLAG_ID(public_flag_id, flag_id)


def do_get(request: gornilo.GetRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    username, password, ciphertext_id = request.flag_id.split('|')

    if not api.login(username, password):
        return gornilo.Verdict.MUMBLE('failed to login')

    ciphertext = api.get_ciphertext(ciphertext_id)

    if ciphertext is None:
        return gornilo.Verdict.CORRUPT('failed to get ciphertext')

    message = api.decrypt(username, password, ciphertext)

    if message.strip() != request.flag.encode().strip():
        return gornilo.Verdict.CORRUPT('invalid flag')

    return gornilo.Verdict.OK()


def do_check(request: gornilo.CheckRequest) -> gornilo.Verdict:
    # TODO

    return gornilo.Verdict.OK()


def wrap_exceptions(
        action: Callable[[gornilo.CheckRequest], gornilo.Verdict],
        request: gornilo.CheckRequest,
) -> gornilo.Verdict:
    try:
        return action(request)
    except client.ProtocolError as error:
        return gornilo.Verdict.MUMBLE(f'protocol error: {error}')
    except requests.ConnectionError:
        return gornilo.Verdict.DOWN('connection error')
    except requests.Timeout:
        return gornilo.Verdict.DOWN('timeout error')
    except requests.RequestException:
        return gornilo.Verdict.MUMBLE('http error')


@checker.define_vuln('flag_id is username|ciphertext_id')
class FirelordChecker(gornilo.VulnChecker):
    @staticmethod
    def put(request: gornilo.PutRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_put, request)

    @staticmethod
    def get(request: gornilo.GetRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_get, request)

    @checker.define_check
    def check(request: gornilo.CheckRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_check, request)


if __name__ == '__main__':
    checker.run()
