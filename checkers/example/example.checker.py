#!/usr/bin/env python3.11
import json
import random
import sys
import traceback

import requests
from gornilo import CheckRequest, Verdict, PutRequest, GetRequest, VulnChecker, NewChecker

checker = NewChecker()
PORT = 2222
CHECK_URL_PATTERN = "http://{hostname}:{port}/check"
PUT_URL_PATTERN = "http://{hostname}:{port}/put"
GET_URL_PATTERN = "http://{hostname}:{port}/get?username={username}&token={token}"


class ErrorChecker:
    def __init__(self):
        self.verdict = Verdict.OK()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        if exc_type in {requests.exceptions.ConnectionError}:
            self.verdict = Verdict.DOWN("Service is down")

        elif exc_type in {requests.exceptions.HTTPError}:
            self.verdict = Verdict.MUMBLE("Incorrect http code")

        if exc_type:
            print(exc_type)
            print(exc_value.__dict__)
            traceback.print_tb(exc_traceback, file=sys.stdout)
        return True


@checker.define_check
async def check_service(request: CheckRequest) -> Verdict:
    with ErrorChecker() as ec:
        check_url = CHECK_URL_PATTERN.format(hostname=request.hostname, port=PORT)
        r = requests.get(check_url, timeout=10)
        r.raise_for_status()

    return ec.verdict


@checker.define_vuln("flag_id is an username")
class CryptoChecker(VulnChecker):
    @staticmethod
    def put(request: PutRequest) -> Verdict:
        with ErrorChecker() as ec:
            rand_int = random.randint(0, 10000000)
            username = f"user_{rand_int}"
            token = f"token_{rand_int}"

            put_url = PUT_URL_PATTERN.format(hostname=request.hostname, port=PORT)
            data = {
                "flag": request.flag,
                "username": username,
                "token": token,
            }
            r = requests.post(put_url, timeout=10, json=data)
            r.raise_for_status()

            flag_id = json.dumps(data)
            ec.verdict = Verdict.OK_WITH_FLAG_ID(username, flag_id)
        return ec.verdict

    @staticmethod
    def get(request: GetRequest) -> Verdict:
        with ErrorChecker() as ec:
            flag_id_json = json.loads(request.flag_id)

            username = flag_id_json['username']
            token = flag_id_json['token']

            get_url = GET_URL_PATTERN.format(hostname=request.hostname, port=PORT, username=username, token=token)
            r = requests.get(get_url, timeout=10)
            r.raise_for_status()
            if r.text != request.flag:
                print(f"Wrong flag: {r.text} != {request.flag}")
                ec.verdict = Verdict.CORRUPT('Wrong flag')

        return ec.verdict


if __name__ == '__main__':
    checker.run()
