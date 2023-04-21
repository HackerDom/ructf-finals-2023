#!/usr/bin/env python3.11
import random
from string import ascii_letters

import requests.exceptions
from gornilo import Checker, PutRequest, GetRequest, CheckRequest, Verdict

from api import Api

PORT = 18888

checker = Checker()


def get_random_string() -> str:
    return ''.join(random.choice(ascii_letters) for _ in range(20))


@checker.define_check
def check(req: CheckRequest) -> Verdict:
    try:
        api = Api(req.hostname, PORT)

        resp = api.front()
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with frontend")

        username, password = get_random_string(), get_random_string()

        resp = api.register(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with register")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on register handler")

        resp = api.login(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with login")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on login handler")

        status, theme, text = "zzzz", get_random_string(), get_random_string()

        resp = api.create_joke(token, status, theme, text)
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with create joke with invalid status")

        status = "private"

        resp = api.create_joke(token, status, theme, text)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create joke")

        resp = api.create_joke(token, status, theme, text)
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with create joke that already exists")

        resp = api.get_joke(token, username, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke")

        data = resp.json()
        if "status" not in data or "theme" not in data or "text" not in data:
            return Verdict.MUMBLE("missing fields when get joke")

        resp = api.get_joke(token, username, get_random_string())
        if resp.status_code != 404:
            return Verdict.MUMBLE("something went wrong with get non-existence joke")

        theme2, text2 = get_random_string(), get_random_string()

        status = "public"

        resp = api.create_joke(token, status, theme2, text2)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create another joke")
    except requests.exceptions.RetryError:
        return Verdict.DOWN("service down")
    return Verdict.OK()


@checker.define_put(vuln_num=1, vuln_rate=1)
def put(req: PutRequest) -> Verdict:
    try:
        api = Api(req.hostname, PORT)

        username, password = get_random_string(), get_random_string()

        resp = api.register(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with register")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on register handler")

        resp = api.login(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with login")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on login handler")

        status, theme = "private", get_random_string()

        resp = api.create_joke(token, status, theme, req.flag)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create joke")

        resp = api.get_joke(token, username, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke")

        data = resp.json()
        if "status" not in data or "theme" not in data or "text" not in data:
            return Verdict.MUMBLE("missing fields when get joke")
    except requests.exceptions.RetryError:
        return Verdict.DOWN("service down")
    return Verdict.OK_WITH_FLAG_ID(f"{username}+{theme}", token)


@checker.define_get(vuln_num=1)
def get(req: GetRequest) -> Verdict:
    try:
        api = Api(req.hostname, PORT)

        token = req.flag_id

        username, theme = req.public_flag_id.split('+')[:2]

        resp = api.get_joke(token, username, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke")

        data = resp.json()
        if "text" not in data or data["text"] != req.flag:
            return Verdict.CORRUPT("wrong text")
    except requests.exceptions.RetryError:
        return Verdict.DOWN("service down")
    return Verdict.OK()


if __name__ == '__main__':
    checker.run()
