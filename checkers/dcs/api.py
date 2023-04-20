#!/usr/bin/env python3

import gornilo.http_clients
import json

from dataclasses import dataclass
from typing import Self


TOKEN_HEADER_NAME = 'X-BCS-Token'


@dataclass
class CreateResult:
    error: str
    token: str


@dataclass
class GetResult:
    error: str
    executeResult: str
    description: str


class DCSClientSession:
    def __init__(self: Self, host: str, port: int, token: str = None):
        self._addr = f'http://{host}:{port}'
        self._session = gornilo.http_clients.requests_with_retries()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self._session.__exit__(exc_type, exc_value, exc_traceback)
        return True

    def _route(self: Self, path: str) -> str:
        return f'{self._addr}/{path}'

    def create(self: Self, code: str, description: str) -> CreateResult:
        j = {code: code, description: description}
        rsp = self._session.post(self._route('/api/compute'), json=j)

        if rsp.status_code == 400:
            return CreateResult(f'error: {rsp.content.decode("utf-8")}', '')

        if rsp.status_code == 500:
            return CreateResult(f'internal server error', '')

        if rsp.status_code != 201:
            return CreateResult(f'unexpected status code: {rsp.status_code}', '')

        token = rsp.headers[TOKEN_HEADER_NAME]
        if token is None:
            return CreateResult(f'invalid headers', '')

        self._session.headers.update({TOKEN_HEADER_NAME: token})

        return CreateResult('', token)

    def get(self: Self, token: str = None) -> GetResult:
        if token is not None:
            self._session.headers.update({TOKEN_HEADER_NAME: token})

        if self._session.headers[TOKEN_HEADER_NAME] is None:
            return GetResult('no token set (get before create?)', '', '')

        rsp = self._session.get(self._route('/api/compute'))

        if rsp.status_code == 400:
            return GetResult('invalid token', '', '')

        if rsp.status_code == 500:
            return GetResult('internal server error', '', '')

        if rsp.status_code == 503:
            return GetResult('server busy', '', '')

        if rsp.status_code != 200:
            return GetResult(f'unexpected status code: {rsp.status_code}', '', '')

        try:
            j = json.loads(rsp.content)
        except json.JSONDecodeError:
            return GetResult('invalid json', '', '')

        if j["status"] == "success":
            desc = j["description"]
            if desc is None:
                return GetResult('invalid json', '', '')
            value = j["computed_value"]
            if value is None:
                return GetResult('invalid json', '', '')
            return GetResult('', desc, value)

        if j["status"] == "error":
            msg = j["message"]
            if msg is None:
                return GetResult('invalid json', '', '')
            return GetResult(msg, '', '')

        return GetResult('invalid json', '', '')
