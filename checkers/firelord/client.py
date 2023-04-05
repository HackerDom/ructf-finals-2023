#!/usr/bin/env python3

import re
import json
import dataclasses
from typing import Self, Dict

import numpy as np
import gornilo.http_clients


class ProtocolError(Exception):
    pass


@dataclasses.dataclass
class Response:
    code: int
    content: str


@dataclasses.dataclass
class Keyspace:
    n: int
    m: int
    modulo: int


def http_request(
        method: str, url: str, params: Dict[str, str] | None, body: bytes | None,
) -> Response:
    session = gornilo.http_clients.requests_with_retries()

    response = session.request(
        method = method,
        url = url,
        params = params,
        data = body,
        allow_redirects = False,
        stream = True,
    )

    if int(response.headers.get('Content-Length', '0')) > (1 << 20):
        raise ProtocolError('body size is too big')

    chunks = []
    total_length = 0

    for chunk in response.iter_content(1 << 10, decode_unicode = False):
        chunks.append(chunk)
        total_length += len(chunk)

        if total_length > (1 << 20):
            raise ProtocolError('body size is too big')

    content = b''.join(chunks).strip()

    return Response(response.status_code, content)


def http_get_request(url: str, params: Dict[str, str] = {}) -> Response:
    return http_request('GET', url, params = params, body = b'')


def http_post_request(url: str, params: Dict[str, str] = {}, body: bytes = b'') -> Response:
    return http_request('POST', url, params = params, body = body)


def parse_ciphertext(content: bytes) -> np.matrix:
    try:
        obj = json.loads(content)
    except Exception:
        raise ProtocolError('invalid format of ciphertext')

    if not isinstance(obj, list):
        raise ProtocolError('invalid format of ciphertext')

    matrix = []
    width = -1

    for element in obj:
        row = []

        if not isinstance(element, list):
            raise ProtocolError('invalid format of ciphertext')

        if width < 0:
            width = len(element)

        if len(element) != width:
            raise ProtocolError('invalid format of ciphertext')

        for value in element:
            if not isinstance(value, str):
                raise ProtocolError('invalid format of ciphertext')

            try:
                row.append(int(value, 10))
            except Exception:
                raise ProtocolError('invalid format of ciphertext')

        matrix.append(row)

    return np.matrix(matrix, dtype = object)


def serialize_ciphertext(ciphertext: np.matrix) -> bytes:
    obj = ciphertext.tolist()

    matrix = [
        [str(x) for x in row] for row in obj
    ]

    return json.dumps(matrix).encode()


def parse_keyspace(content: bytes) -> Keyspace:
    try:
        keyspace = content.decode()
    except Exception:
        raise ProtocolError('invalid format of keyspace')

    parts = re.findall(r'\d+', keyspace)

    if len(parts) != 3:
        raise ProtocolError('invalid format of keyspace')

    try:
        n = int(parts[0])
        m = int(parts[1])
        modulo = int(parts[2])

        return Keyspace(n, m, modulo)
    except Exception:
        raise ProtocolError('invalid format of keyspace')


def parse_id(content: bytes) -> str:
    try:
        return content.decode()
    except Exception:
        raise ProtocolError('invalid format of id')


class Api:
    def __init__(self: Self, hostname: str, port: int) -> None:
        self.url = f'http://{hostname}:{port}'

    def register(self: Self, username: str, password: str) -> bool:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/user/register', params = params)

        if response.code == 201 and response.content == b'registered':
            return True

        if response.code == 409 and response.content == b'user already exists':
            return False

        raise ProtocolError('invalid response on user/register')

    def login(self: Self, username: str, password: str) -> bool:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/user/login', params = params)

        if response.code == 200 and response.content == b'logged in':
            return True

        if response.code == 401 and response.content == b'invalid username or password':
            return False

        raise ProtocolError('invalid response on user/login')

    def get_ciphertext(self: Self, id: str) -> np.matrix | None:
        params = {
            'id': id,
        }

        response = http_get_request(self.url + '/api/storage/ciphertext', params = params)

        if response.code == 200 and len(response.content) > 0:
            return parse_ciphertext(response.content)

        if response.code == 404 and response.content == b'ciphertext not found':
            return None

        raise ProtocolError('invalid response on storage/ciphertext')

    def get_keyspace(self: Self, username: str) -> Keyspace | None:
        params = {
            'username': username,
        }

        response = http_get_request(self.url + '/api/storage/keyspace', params = params)

        if response.code == 200 and len(response.content) > 0:
            return parse_keyspace(response.content)

        if response.code == 404 and response.content == b'key not found':
            return None

        raise ProtocolError('invalid response on storage/keyspace')

    def encrypt(self: Self, username: str, password: str, message: bytes) -> str:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/crypto/encrypt', params = params, body = message)

        if response.code == 201 and len(response.content) > 0:
            return parse_id(response.content)

        raise ProtocolError('invalid response on crypto/encrypt')

    def decrypt(self: Self, username: str, password: str, ciphertext: np.matrix) -> bytes:
        params = {
            'username': username,
            'password': password,
        }

        body = serialize_ciphertext(ciphertext)

        response = http_post_request(self.url + '/api/crypto/decrypt', params = params, body = body)

        if response.code == 200 and len(response.content) > 0:
            return response.content

        raise ProtocolError('invalid response on crypto/decrypt')
