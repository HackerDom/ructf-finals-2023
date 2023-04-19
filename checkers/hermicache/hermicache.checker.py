#!/usr/bin/env python3
import json
import random
import sys
import traceback
import functools

import requests
from gornilo import CheckRequest, Verdict, PutRequest, GetRequest, VulnChecker, NewChecker

import compute
import generators
import models

checker = NewChecker()
PORT = 8080

AUTH_URL_PATTERN = "http://{hostname}:{port}/{method}"
CREATE_FIELD_URL_PATTERN = "http://{hostname}:{port}/field"
GET_FIELD_URL_PATTERN = "http://{hostname}:{port}/field/{uuid}"
COMPUTE_URL_PATTERN = "http://{hostname}:{port}/compute?field_uuid={field_uuid}&arg={arg}"
LIST_FIELDS_URL_PATTERN = "http://{hostname}:{port}/list_fields"


def make_auth_request(method, hostname, username, password):
    auth_url = AUTH_URL_PATTERN.format(hostname=hostname, port=PORT, method=method)
    data = {
        "username": username,
        "password": password,
    }
    r = requests.post(auth_url, timeout=10, json=data)
    r.raise_for_status()
    return r.cookies.get_dict()


make_login_request = functools.partial(make_auth_request, "login")
make_register_request = functools.partial(make_auth_request, "register")


def make_create_field_request(hostname, cookies, field):
    field_url = CREATE_FIELD_URL_PATTERN.format(hostname=hostname, port=PORT)
    r = requests.post(field_url, timeout=10, cookies=cookies, json=field)
    r.raise_for_status()
    uuid = r.json()['uuid']
    return uuid


def make_get_field_request(hostname, cookies, uuid):
    field_url = GET_FIELD_URL_PATTERN.format(hostname=hostname, port=PORT, uuid=uuid)
    r = requests.get(field_url, timeout=10, cookies=cookies)
    r.raise_for_status()
    return r.json()


def make_compute_request(hostname, cookies, field_uuid, arg):
    field_url = COMPUTE_URL_PATTERN.format(hostname=hostname, port=PORT, field_uuid=field_uuid, arg=arg)
    r = requests.get(field_url, timeout=10, cookies=cookies)
    r.raise_for_status()
    return r.json()


def make_list_fields_request(hostname, cookies):
    field_url = LIST_FIELDS_URL_PATTERN.format(hostname=hostname, port=PORT)
    r = requests.get(field_url, timeout=10, cookies=cookies)
    r.raise_for_status()
    return r.json()


class ErrorChecker:
    def __init__(self):
        self.verdict = Verdict.OK()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        if exc_type in {requests.exceptions.ConnectionError, requests.exceptions.ReadTimeout}:
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
        username = generators.gen_username()
        password = generators.gen_password()

        cookies = make_register_request(request.hostname, username, password)
        palindrome_field = generators.gen_palindrome_field()

        uuid = make_create_field_request(request.hostname, cookies, palindrome_field)

        if not isinstance(uuid, (str, int)):
            ec.verdict = Verdict.MUMBLE("p: invalid uuid: {}".format(uuid))
            return ec.verdict

        real_result = make_compute_request(request.hostname, cookies, uuid, "")
        expected_result = {
            "res": compute.get_longest_palindrome(palindrome_field['content']),
        }

        if real_result != expected_result:
            print(f"incorrect result: {expected_result} != {real_result}")
            ec.verdict = Verdict.MUMBLE("incorrect result")
            return ec.verdict

        recurrent_field = generators.gen_recurrent_field()
        uuid = make_create_field_request(request.hostname, cookies, recurrent_field)
        if not isinstance(uuid, (str, int)):
            print(f"invalid field uuid: {type(uuid)}, {uuid}")
            ec.verdict = Verdict.MUMBLE("r: invalid uuid: {}".format(uuid))
            return ec.verdict

        cache = {}
        for check_index in range(10):
            print(f'check #{check_index}')
            arg = random.randint(30, 40)
            real_result = make_compute_request(request.hostname, cookies, uuid, arg)
            expected_result = {
                "res": compute.compute_recurrence_relation_formula_by_field(recurrent_field, arg, cache)
            }
            if real_result != expected_result:
                print(f"incorrect result #{check_index}: {expected_result} != {real_result}")
                ec.verdict = Verdict.MUMBLE(f"incorrect result #{check_index}")
                return ec.verdict

    return ec.verdict


@checker.define_vuln("flag_id is an username")
class CryptoChecker(VulnChecker):
    @staticmethod
    def put(request: PutRequest) -> Verdict:
        with ErrorChecker() as ec:
            username = generators.gen_username()
            password = generators.gen_password()

            cookies = make_register_request(request.hostname, username, password)

            field = generators.gen_palindrome_field(request.flag)
            uuid = make_create_field_request(request.hostname, cookies, field)
            if not isinstance(uuid, (str, int)):
                print(f"invalid field uuid: {type(uuid)}, {uuid}")
                ec.verdict = Verdict.MUMBLE("r: invalid uuid: {}".format(uuid))
                return ec.verdict

            data = {
                "field_uuid": uuid,
                "username": username,
                "password": password,
            }
            flag_id = json.dumps(data)
            ec.verdict = Verdict.OK_WITH_FLAG_ID(username, flag_id)
        return ec.verdict

    @staticmethod
    def get(request: GetRequest) -> Verdict:
        with ErrorChecker() as ec:
            flag_id_json = json.loads(request.flag_id)
            username = flag_id_json["username"]
            password = flag_id_json["password"]
            expected_uuid = flag_id_json["field_uuid"]

            cookies = make_login_request(request.hostname, username, password)

            field_uuids = make_list_fields_request(request.hostname, cookies)
            if not isinstance(field_uuids, list):
                print(f"invalid field_uuids: {type(field_uuids)}, {field_uuids}")
                ec.verdict = Verdict.MUMBLE("invalid field uuid list: {}".format(field_uuids))
                return ec.verdict

            for field_uuid in field_uuids:
                if not isinstance(field_uuid, int):
                    print(f"invalid field_uuid: {type(field_uuid)}, {field_uuid}")
                    ec.verdict = Verdict.MUMBLE("invalid field uuid: {}".format(field_uuid))
                    return ec.verdict

            if expected_uuid not in field_uuids:
                print(f"field uuids {field_uuids} does not contain expected uuid {expected_uuid}")
                ec.verdict = Verdict.CORRUPT("field list does not contain expected uuid")
                return ec.verdict

            real_field = make_get_field_request(request.hostname, cookies, expected_uuid)
            if not isinstance(real_field, dict) \
                    or not set(real_field.keys()).issubset({"content", "owner", "type"}) \
                    or any(map(lambda x: not isinstance(x, str), real_field.values())):
                print(f"invalid field: {real_field}")
                ec.verdict = Verdict.CORRUPT("invalid field")
                return ec.verdict

            if real_field['content'] != request.flag:
                print(f"different flags, expected: {request.flag}, real: {real_field}")
                ec.verdict = Verdict.CORRUPT("corrupt flag")
                return ec.verdict

        return ec.verdict


if __name__ == '__main__':
    checker.run()
