#!/usr/bin/env python3

import dataclasses
from typing import Self, Dict

import gornilo.http_clients

import model


MAX_BODY_SIZE = 1 << 20 # 1 MiB
CHUNK_SIZE = 1 << 10 # 1 KiB

PHP_EOL = b'\n'


@dataclasses.dataclass
class Response:
    code: int
    content: bytes


def http_request(
        method: str, url: str, params: Dict[str, str], body: bytes,
) -> Response:
    session = gornilo.http_clients.requests_with_retries(
        status_forcelist=(500, 502),
    )

    response = session.request(
        method = method,
        url = url,
        params = params,
        data = body,
        allow_redirects = False,
        stream = True,
    )

    try:
        content_length = int(response.headers.get('Content-Length', '0'))
    except Exception:
        raise model.ProtocolError('invalid http headers')

    if content_length > MAX_BODY_SIZE:
        raise model.ProtocolError('body size is too big')

    chunks = []
    total_length = 0

    for chunk in response.iter_content(CHUNK_SIZE, decode_unicode = False):
        chunks.append(chunk)
        total_length += len(chunk)

        if total_length > MAX_BODY_SIZE:
            raise model.ProtocolError('body size is too big')

    content = b''.join(chunks)

    if not content.endswith(PHP_EOL):
        raise model.ProtocolError('no newline at the end of body')
    
    content = content[:-len(PHP_EOL)]

    return Response(response.status_code, content)


def http_get_request(url: str, params: Dict[str, str] = {}) -> Response:
    return http_request('GET', url, params = params, body = b'')


def http_post_request(url: str, params: Dict[str, str] = {}, body: bytes = b'') -> Response:
    return http_request('POST', url, params = params, body = body)


class Api:
    def __init__(self: Self, hostname: str, port: int) -> None:
        self.url = f'http://{hostname}:{port}'

    def register(self: Self, username: str, password: str) -> bool:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/user/register', params = params)

        if response.code == 201 and b'registered' in response.content:
            return True

        if response.code == 409 and b'user already exists' in response.content:
            return False

        raise model.ProtocolError('invalid response on user/register')

    def login(self: Self, username: str, password: str) -> bool:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/user/login', params = params)

        if response.code == 200 and b'logged in' in response.content:
            return True

        if response.code == 401 and b'invalid username or password' in response.content:
            return False

        raise model.ProtocolError('invalid response on user/login')

    def get_ciphertext(self: Self, id: str) -> model.Ciphertext | None:
        params = {
            'id': id,
        }

        response = http_get_request(self.url + '/api/storage/ciphertext', params = params)

        if response.code == 200 and len(response.content) > 0:
            try:
                data = response.content.decode()
            except Exception:
                raise model.ProtocolError('invalid ciphertext format')

            return model.Ciphertext.parse(data)

        if response.code == 404 and b'ciphertext not found' in response.content:
            return None

        raise model.ProtocolError('invalid response on storage/ciphertext')

    def get_keyspace(self: Self, username: str) -> model.Keyspace | None:
        params = {
            'username': username,
        }

        response = http_get_request(self.url + '/api/storage/keyspace', params = params)

        if response.code == 200 and len(response.content) > 0:
            try:
                data = response.content.decode()
            except Exception:
                raise model.ProtocolError('invalid keyspace format')

            return model.Keyspace.parse(data)

        if response.code == 404 and b'key not found' in response.content:
            return None

        raise model.ProtocolError('invalid response on storage/keyspace')

    def encrypt(self: Self, username: str, password: str, message: bytes) -> str:
        params = {
            'username': username,
            'password': password,
        }

        response = http_post_request(self.url + '/api/crypto/encrypt', params = params, body = message)

        if response.code == 201 and len(response.content) > 0:
            try:
                data = response.content.decode()
            except Exception:
                raise model.ProtocolError('invalid ciphertext_id format')
            
            return data

        raise model.ProtocolError('invalid response on crypto/encrypt')

    def decrypt(self: Self, username: str, password: str, ciphertext: model.Ciphertext) -> bytes:
        params = {
            'username': username,
            'password': password,
        }

        body = ciphertext.serialize()

        response = http_post_request(self.url + '/api/crypto/decrypt', params = params, body = body)

        if response.code == 200 and len(response.content) > 0:
            return response.content

        raise model.ProtocolError('invalid response on crypto/decrypt')
