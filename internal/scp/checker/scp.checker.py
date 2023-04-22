#!/usr/bin/env python3.11

import json
import random
import re
import string
import time
import requests

from gornilo import NewChecker, CheckRequest, GetRequest, PutRequest, Verdict, VulnChecker
from gornilo.http_clients import requests_with_retries

checker = NewChecker()

flag_re = re.compile(r'\w{31}=')


def random_str():
    return "".join(random.choices(string.digits + string.ascii_letters, k=random.randint(1, 11)))


def get_token(url: str):
    welcome = url + "welcome"
    user_data = {
        "login": random_str(),
        "password": random_str()
    }
    resp = _retry(requests.post, 10, url=welcome, json=user_data)
    # resp = requests_with_retries().post(welcome, json=user_data)
    while resp.status_code == 409:
        user_data = {
            "login": random_str(),
            "password": random_str()
        }
        resp = requests_with_retries(1).post(welcome, json=user_data)
    if resp.status_code == 200:
        return resp.text
    return None


@checker.define_check
async def check_service(request: CheckRequest) -> Verdict:
    try:
        url = f"http://{request.hostname}:8081/"

        # checking welcome [get]

        resp = _retry(requests.get, 10, url=url + "welcome")
        # resp = requests_with_retries().get(url + "welcome")
        if resp.status_code != 200:
            return Verdict.MUMBLE("Verdict: MUMBLE bad welcome")

        # checking user creation

        token = get_token(url)
        if not token:
            return Verdict.MUMBLE("Verdict: MUMBLE could not create user")

        # checking create new document

        n = random.randint(1, 1000)
        doc_flag = f'doc_scp{n}_{random_str()} <- "{random_str()}";'
        doc_fields = []
        for i in range(random.randint(2, 3)):
            field = random_str()
            doc_field = f'doc_scp{n}_{field} <- "{random_str()}";'
            sec = ''
            if random.randint(0, 1):
                sec = '{(scp' + str(n) + f', "{field}")' + ' -> secret};'
            doc_fields.append(doc_field)
            doc_fields.append(sec)
        save = '@docId <- {(scp' + str(n) + ') -> save};'
        result = '{(@docId) -> result};'
        ql = doc_flag + ''.join(doc_fields) + save + result
        resp = _retry(requests.post, 10, url=url, json={"token": token, "query": ql})
        # resp = requests_with_retries(1).post(url, json={"token": token, "query": ql})
        if resp.status_code == 400:
            return Verdict.MUMBLE("Verdict: MUMBLE could not get list of docs")
        if resp.status_code == 500:
            return Verdict.MUMBLE("Verdict: MUMBLE got internal server error")
        if resp.status_code != 200:
            return Verdict.MUMBLE("Verdict: MUMBLE unexpected status code " + str(resp.status_code))

        # checking list of docs

        ql = '''
        @docs <- {(0, 50) -> list};
        {(@docs) -> result};
        '''
        resp = _retry(requests.post, 10, url=url, json={"token": token, "query": ql})
        # resp = requests_with_retries().post(url, json={"token": token, "query": ql})
        if resp.status_code == 400:
            return Verdict.MUMBLE("Verdict: MUMBLE could not get list of docs")
        if resp.status_code == 500:
            return Verdict.MUMBLE("Verdict: MUMBLE got internal server error")
        if resp.status_code != 200:
            return Verdict.MUMBLE("Verdict: MUMBLE unexpected status code " + str(resp.status_code))
        resp_data = resp.json()
        if resp_data["type"] != "PLAIN":
            return Verdict.MUMBLE("Verdict: MUMBLE weird answer")
        raw_list = resp_data["body"].strip()
        if not raw_list:
            return Verdict.MUMBLE("Verdict: MUMBLE empty list of documents")
        docs_with_users = []
        for pair in raw_list.split("\n"):
            doc, user = pair.split(":")[0].strip(), pair.split(":")[1].strip()
            docs_with_users.append((doc, user))
        doc_with_user = random.choice(docs_with_users)

        # checking random document
        doc_id = doc_with_user[0]
        doc_id_var = f'@docId <- {doc_id};'
        doc_getting = '@doc <- {(@docId) -> get};'
        res = '{(@doc) -> result};'
        ql = doc_id_var + doc_getting + res
        resp = _retry(requests.post, 10, url=url, json={"token": token, "query": ql})
        # resp = requests_with_retries().post(url, json={"token": token, "query": ql})
        if resp.status_code == 200:
            resp_data = resp.json()
            if resp_data["type"] != "HTML":
                return Verdict.MUMBLE("Verdict: MUMBLE got weird document")
            return Verdict.OK()
        if resp.status_code == 400:
            return Verdict.MUMBLE("Verdict: MUMBLE could not get random doc")
        if resp.status_code == 500:
            return Verdict.MUMBLE("Verdict: MUMBLE got internal server error")
        return Verdict.MUMBLE("Verdict: MUMBLE unexpected status code " + str(resp.status_code))

    except Exception as e:
        return Verdict.DOWN("timeout")


@checker.define_vuln("")
class GameChecker(VulnChecker):
    @staticmethod
    def get(request: GetRequest) -> Verdict:
        try:
            url = f"http://{request.hostname}:8081/"
            flag = request.flag
            flag_id_json = json.loads(request.flag_id)
            _id = flag_id_json["id"]
            doc_id = f'@docId <- {_id};'
            token = flag_id_json["token"]
            doc = '@doc <- {(@docId) -> get};'
            res = '{(@doc) -> result};'
            ql = doc_id + doc + res
            data = {
                "token": token,
                "query": ql
            }
            resp = _retry(requests.post, 10, url=url, json=data)
            # resp = requests_with_retries().post(url, json=data)
            if resp.status_code == 400:
                return Verdict.MUMBLE("Verdict: MUMBLE could not execute query")
            if resp.status_code == 500:
                return Verdict.MUMBLE("Verdict: MUMBLE got internal server error")
            if resp.status_code != 200:
                return Verdict.MUMBLE("Verdict: MUMBLE unexpected status code " + str(resp.status_code))
            resp_data = resp.json()
            if resp_data.get("type", "") != "HTML":
                return Verdict.CORRUPT("Verdict: CORRUPT got weird document")
            resp_body = resp_data.get("body")
            if not resp_body:
                return Verdict.CORRUPT("Verdict: CORRUPT response without body")
            possible_flags = re.findall(flag_re, resp_body)
            if not possible_flags:
                return Verdict.CORRUPT("Verdict: CORRUPT could not find my secret")
            for possible_flag in possible_flags:
                if possible_flag == flag:
                    return Verdict.OK()
            return Verdict.CORRUPT("Verdict: CORRUPT could not find my secret")
        except Exception as e:
            return Verdict.DOWN("Verdict: DOWN could not read secrets" + str(e))

    @staticmethod
    def put(request: PutRequest) -> Verdict:
        try:
            url = f"http://{request.hostname}:8081/"
            token = get_token(url)
            if not token:
                return Verdict.CORRUPT("Verdict: CORRUPT could not create user")

            flag = request.flag
            n = random.randint(1, 1000)
            doc_flag = f'doc_scp{n}_secret <- "{flag}";'
            sec_flag = '{(scp' + str(n) + ', "secret") -> secret};'
            doc_fields = []
            for i in range(random.randint(2, 3)):
                field = random_str()
                doc_field = f'doc_scp{n}_{field} <- "{random_str()}";'
                sec = ''
                if random.randint(0, 1):
                    sec = '{(scp' + str(n) + f', "{field}")' + ' -> secret};'
                doc_fields.append(doc_field)
                doc_fields.append(sec)
            save = '@docId <- {(scp' + str(n) + ') -> save};'
            result = '{(@docId) -> result};'
            ql = doc_flag + sec_flag + ''.join(doc_fields) + save + result
            data = {
                "token": token,
                "query": ql
            }
            resp = _retry(requests.post, 10, url=url, json=data)
            # resp = requests_with_retries(1).post(url, json=data)
            if resp.status_code == 400:
                return Verdict.MUMBLE("Verdict: MUMBLE could not execute query")
            if resp.status_code == 500:
                return Verdict.CORRUPT("Verdict: CORRUPT got internal server error")
            if resp.status_code != 200:
                return Verdict.CORRUPT("Verdict: CORRUPT unexpected status code " + str(resp.status_code))
            resp_data = resp.json()
            doc_id = resp_data["body"]
            if not doc_id:
                return Verdict.MUMBLE("Verdict: MUMBLE could not get document id")
            return Verdict.OK_WITH_FLAG_ID(doc_id, json.dumps({"id": doc_id, "token": token}))
            # return Verdict.OK_WITH_FLAG_ID(doc_id, token)
        except Exception as e:
            return Verdict.DOWN("Verdict: DOWN could not write secrets" + str(e))


def _retry(method, i, **kwargs):
    if i == 1:
        return method(**kwargs)
    try:
        resp = method(**kwargs)
        if resp.status_code != 200:
            time.sleep(0.5)
            return _retry(method, i - 1, **kwargs)
        return resp
    except Exception:
        time.sleep(0.5)
        return _retry(method, i - 1, **kwargs)


if __name__ == "__main__":
    checker.run()
