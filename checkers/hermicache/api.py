import functools
import requests

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
