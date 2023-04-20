#!/usr/bin/env python3

import gornilo.http_clients
import json

from dataclasses import dataclass


TOKEN_HEADER_NAME = 'X-DCS-Token'


@dataclass
class CreateResult:
    error: str
    token: str


@dataclass
class GetResult:
    error: str
    executeResult: str
    description: str


class DCSProtocolError(Exception):
    pass


class DCSValidationError(Exception):
    pass


def check_token(token: str):
    if len(token) != 32:
        raise DCSValidationError('invalid token format')

    for c in token:
        if c not in '1234567890abcdef':
            raise DCSValidationError('invalid token format')


class DCSClient:
    def __init__(self, host: str, port: int, token: str = None):
        self._addr = f'http://{host}:{port}'
        self._session = gornilo.http_clients.requests_with_retries(
            status_forcelist=(400, 404, 500, 502, 503))

        if token is not None:
            self._session.headers.update({TOKEN_HEADER_NAME: token})

    def _route(self, path: str) -> str:
        return f'{self._addr}/{path}'

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        return self._session.__exit__(exc_type, exc_value, exc_traceback)

    def create(self, code: str, description: str) -> CreateResult:
        self._token = None

        j = {'code': code, 'description': description}
        rsp = self._session.post(self._route('api/compute'), json=j)

        if rsp.status_code == 400:
            return CreateResult(f'{rsp.content.decode("utf-8")}', '')

        if rsp.status_code != 201:
            raise DCSProtocolError(
                f'unexpected status code: {rsp.status_code}')

        if TOKEN_HEADER_NAME not in rsp.headers:
            raise DCSProtocolError(
                f'unexpected status code: {rsp.status_code}')
        token = rsp.headers[TOKEN_HEADER_NAME]

        self._session.headers.update({TOKEN_HEADER_NAME: token})

        return CreateResult('', token)

    def get(self, token: str = None) -> GetResult:
        if token is not None:
            self._session.headers.update({TOKEN_HEADER_NAME: token})

        if TOKEN_HEADER_NAME not in self._session.headers:
            return GetResult('no token set (get before create or previous create failed?)', '', '')

        rsp = self._session.get(self._route('api/compute'))

        if rsp.status_code != 200:
            raise DCSProtocolError(
                f'unexpected status code: {rsp.status_code}')

        try:
            j = json.loads(rsp.content)
        except json.JSONDecodeError:
            raise DCSValidationError('invalid json')

        if j["status"] == "success":
            desc = j["description"]
            if desc is None:
                raise DCSValidationError('invalid json')
            value = j["computed_value"]
            if len(value) > 200:
                raise DCSValidationError('too long computed value')
            if value is None:
                raise DCSValidationError('invalid json')
            return GetResult('', value, desc)

        if j["status"] == "error":
            desc = j["description"]
            if desc is None:
                raise DCSValidationError('invalid json')
            msg = j["message"]
            if len(msg) > 300:
                raise DCSValidationError('too long message')
            if msg is None:
                raise DCSValidationError('invalid json')
            return GetResult('', msg, desc)

        raise DCSProtocolError('invalid json')


if __name__ == '__main__':
    with DCSClient('localhost', 7654) as c:
        print(c.create(
            'fun f() { return 3.1415927; } fun main() { return f(); }', 'this is description'))
        print(c.get())
