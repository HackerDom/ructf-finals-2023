#!/usr/bin/env python3

import json
import dataclasses
from typing import Self, Dict, Any

import gornilo.http_clients

import model


MAX_BODY_SIZE = 1 << 20 # 1 MiB
CHUNK_SIZE = 1 << 10 # 1 KiB

TOKEN_HEADER_NAME = 'X-Token'


@dataclasses.dataclass
class Response:
    code: int
    content: Dict[str, Any]


class Api:
    def __init__(self: Self, hostname: str, port: int) -> None:
        self.url = f'http://{hostname}:{port}'
        self.token: str | None = None

    def user_get(self: Self, username: str) -> model.User | None:
        url = self.url + f'/users/profile/{username}'

        response = self.http_request('GET', url)

        if isinstance(response, Response) and response.code == 200:
            return model.User.parse(response.content)

        if isinstance(response, model.ServiceError) and response.name == 'UserNotFoundError':
            return None

        raise model.ProtocolError('invalid response on user/profile')

    def user_register(self: Self, username: str, password: str) -> bool:
        url = self.url + '/users/register'
        body = {
            'name': username,
            'password': password,
        }

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name == 'AlreadyExistsError':
            return False

        raise model.ProtocolError('invalid response on user/register')

    def user_login(self: Self, username: str, password: str) -> bool:
        url = self.url + '/users/login'
        body = {
            'name': username,
            'password': password,
        }

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name == 'InvalidCredentialsError':
            return False

        raise model.ProtocolError('invalid response on user/login')

    def user_logout(self: Self) -> bool:
        url = self.url + '/users/logout'
        body = {}

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        raise model.ProtocolError('invalid response on user/logout')

    def note_get(self: Self, title: str) -> model.Note | None:
        url = self.url + f'/notes/{title}'

        response = self.http_request('GET', url)

        if isinstance(response, Response) and response.code == 200:
            return model.Note.parse(response.content)

        if isinstance(response, model.ServiceError) and response.name == 'NoteNotFoundError':
            return None

        raise model.ProtocolError('invalid response on note/get')

    def note_create(self: Self, title: str, visible: bool, content: str) -> bool:
        url = self.url + '/notes'
        body = {
            'title': title,
            'visible': visible,
            'content': content,
        }

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name == 'AlreadyExistsError':
            return False

        raise model.ProtocolError('invalid response on note/create')

    def note_share(self: Self, title: str, viewer: str) -> bool:
        url = self.url + f'/notes/{title}/share'
        body = {
            'viewer': viewer,
        }

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name in ('OwnerMismatchError', 'UserNotFoundError'):
            return False

        raise model.ProtocolError('invalid response on note/share')

    def note_deny(self: Self, title: str, viewer: str) -> bool:
        url = self.url + f'/notes/{title}/deny'
        body = {
            'viewer': viewer,
        }

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name in ('OwnerMismatchError', 'UserNotFoundError'):
            return False

        raise model.ProtocolError('invalid response on note/deny')

    def note_destroy(self: Self, title: str) -> bool:
        url = self.url + f'/notes/{title}/destroy'
        body = {}

        response = self.http_request('POST', url, body)

        if isinstance(response, Response) and response.code == 200:
            return True

        if isinstance(response, model.ServiceError) and response.name in ('OwnerMismatchError'):
            return False

        raise model.ProtocolError('invalid response on note/destroy')

    def http_request(
            self: Self, method: str, url: str, body: Dict[str, Any] = {},
    ) -> Response | model.ServiceError:
        session = gornilo.http_clients.requests_with_retries(
            status_forcelist = (500, 502),
        )

        response = session.request(
            method = method,
            url = url,
            json = body,
            allow_redirects = False,
            stream = True,
            headers = {
                TOKEN_HEADER_NAME: self.token,
            },
        )

        self.token = response.headers.get(TOKEN_HEADER_NAME)

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

        data = b''.join(chunks)

        try:
            content = json.loads(data)
        except Exception:
            raise model.ProtocolError('failed to parse json response')

        if 'error' in content:
            error = content['error']

            if not isinstance(error, dict):
                raise model.ProtocolError('failed to parse error')

            return model.ServiceError.parse(error)

        return Response(response.status_code, content)
