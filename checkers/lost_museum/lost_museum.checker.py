#!/usr/bin/env python3.11
import random
from string import ascii_letters

import requests.exceptions
from gornilo import Checker, PutRequest, GetRequest, CheckRequest, Verdict

from api import Api
from consts import users, jokes

PORT = 18888

checker = Checker()


def get_username() -> str:
    return random.choice(users) + get_random_string(5)


def get_joke() -> tuple[str, str]:
    joke = random.choice(jokes)
    theme = joke["theme"] + get_random_string(5)
    text = joke["text"]
    return theme, text


def get_random_string(n: int) -> str:
    return ''.join(random.choice(ascii_letters) for _ in range(n))


@checker.define_check
def check(req: CheckRequest) -> Verdict:
    try:
        api = Api(req.hostname, PORT)

        resp = api.front()
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with frontend")

        username, password = get_username(), get_random_string(20)

        resp = api.register(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with register")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on register handler")

        resp = api.register(username, get_random_string(20))
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with register user that already exist")

        resp = api.login(username, password)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with login")

        token = resp.json().get("token", "")
        if token == "":
            return Verdict.MUMBLE("no token on login handler")

        resp = api.login(get_random_string(20), password)
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with login user that not exists")

        resp = api.login(username, get_random_string(20))
        if resp.status_code != 403:
            return Verdict.MUMBLE("something went wrong with login with invalid password")

        status = get_random_string(5)
        theme, text = get_joke()

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

        resp = api.get_joke(token, username, get_random_string(20))
        if resp.status_code != 404:
            return Verdict.MUMBLE("something went wrong with get non-existence joke")

        theme2, text2 = get_joke()

        status = "public"

        resp = api.create_joke(token, status, theme2, text2)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create another joke")

        resp = api.get_joke(token, username, theme2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke")

        data = resp.json()
        if "status" not in data or "theme" not in data or "text" not in data:
            return Verdict.MUMBLE("missing fields when get joke")

        resp = api.list_jokes(token)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong when get jokes list")

        data = resp.json().get("data", [])
        for joke in data:
            if not ((joke.get("theme", "") == theme and joke.get("text", "") == text)
                    or (joke.get("theme", "") == theme2 and joke.get("text", "") == text2)):
                return Verdict.MUMBLE("invalid data in jokes list")

        username2, password2 = get_username(), get_random_string(20)

        resp = api.register(username2, password2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with register")

        token2 = resp.json().get("token", "")
        if token2 == "":
            return Verdict.MUMBLE("no token on register handler")

        status = "private"

        resp = api.create_joke(token2, status, theme, text)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create joke (user 2)")

        resp = api.theme_jokes(token2, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with theme jokes")

        data = resp.json().get("data", [])
        if len(data) != 1 or data[0]["theme"] != theme or data[0]["text"] != text:
            return Verdict.MUMBLE("invalid data in theme jokes list")

        resp = api.create_joke(token2, status, theme2, text2)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create joke (user 2)")

        resp = api.theme_jokes(token2, theme2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with theme jokes")

        data = resp.json().get("data", [])
        if len(data) < 2 or \
                data[0]["theme"] != theme2 or \
                data[0]["text"] != text2 or \
                data[1]["theme"] != theme2 or \
                data[1]["text"] != text2:
            return Verdict.MUMBLE("invalid data in theme jokes list")

        resp = api.friend_request(token, username2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with friend request")

        resp = api.friend_request(token, get_random_string(20))
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with friend request when user is not exists")

        resp = api.requests_list(token2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with requests list")

        data = resp.json().get("data", [])
        if len(data) < 1 or data[0] != username:
            return Verdict.MUMBLE("invalid data in requests list")

        resp = api.accept_request(token2, username)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with accept request")

        resp = api.accept_request(token2, get_random_string(20))
        if resp.status_code != 400:
            return Verdict.MUMBLE("something went wrong with accept request when user not exists")

        resp = api.friends_list(token)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with friends list")

        data = resp.json().get("data", [])
        if len(data) < 1 or data[0] != username2:
            return Verdict.MUMBLE("invalid data in friends list")

        resp = api.friends_list(token2)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with friends list")

        data = resp.json().get("data", [])
        if len(data) < 1 or data[0] != username:
            return Verdict.MUMBLE("invalid data in friends list")

        resp = api.get_joke(token2, username, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke (from user 2)")

        data = resp.json()
        if "status" not in data or "theme" not in data or "text" not in data \
                or data["theme"] != theme or data["text"] != text:
            return Verdict.MUMBLE("missing fields when get joke")

        resp = api.theme_jokes(token2, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with theme jokes")

        data = resp.json().get("data", [])
        if len(data) < 2 or data[0]["theme"] != theme or data[0]["text"] != text \
                or data[1]["theme"] != theme or data[1]["text"] != text:
            return Verdict.MUMBLE("invalid data in theme jokes list (friends)")

    except requests.JSONDecodeError:
        return Verdict.MUMBLE("invalid json")
    except requests.ConnectionError:
        return Verdict.DOWN("connection error")
    except requests.Timeout:
        return Verdict.DOWN("timeout error")
    except requests.RequestException:
        return Verdict.MUMBLE("http error")
    return Verdict.OK()


@checker.define_put(vuln_num=1, vuln_rate=1)
def put(req: PutRequest) -> Verdict:
    try:
        api = Api(req.hostname, PORT)

        username, password = get_username(), get_random_string(20)

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

        status = "private"
        theme, _ = get_joke()

        resp = api.create_joke(token, status, theme, req.flag)
        if resp.status_code != 201:
            return Verdict.MUMBLE("something went wrong with create joke")

        resp = api.get_joke(token, username, theme)
        if resp.status_code != 200:
            return Verdict.MUMBLE("something went wrong with get joke")

        data = resp.json()
        if "status" not in data or "theme" not in data or "text" not in data:
            return Verdict.MUMBLE("missing fields when get joke")
    except requests.JSONDecodeError:
        return Verdict.MUMBLE("invalid json")
    except requests.ConnectionError:
        return Verdict.DOWN("connection error")
    except requests.Timeout:
        return Verdict.DOWN("timeout error")
    except requests.RequestException:
        return Verdict.MUMBLE("http error")
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
    except requests.JSONDecodeError:
        return Verdict.MUMBLE("invalid json")
    except requests.ConnectionError:
        return Verdict.DOWN("connection error")
    except requests.Timeout:
        return Verdict.DOWN("timeout error")
    except requests.RequestException:
        return Verdict.MUMBLE("http error")
    return Verdict.OK()


if __name__ == '__main__':
    checker.run()
