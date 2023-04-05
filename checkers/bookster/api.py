import os.path
import time
import traceback
import uuid
from urllib.parse import urlparse

import requests.exceptions
from gornilo.http_clients import requests_with_retries

from utils import generate_name, generate_email, generate_password, get_hash_stream, get_hash


class AuthError(Exception):
    pass

class ParseError(Exception):
    pass

class Api:
    def __init__(self, host, port, token=None):
        self._host = host
        self._port = port
        self._addr = f'http://{host}:{port}'
        self._session = requests_with_retries()
        if token:
            self._session.headers.update({'Authorization': f'Token {token}'})

    def send_request(self, uri, method="get", json=None):
        if method == "get":
            rsp = self._session.get(
                f"{self._addr}{uri}",
                json=json
            )
        elif method == "post":
            rsp = self._session.post(
                f"{self._addr}{uri}",
                json=json
            )
        else:
            raise Exception("Method not found")
        try:
            return rsp.status_code, rsp.json()
        except:
            traceback.print_exc()
            return rsp.status_code, {}

    def signup(self, email, username, password):
        return self.send_request('/auth/users/', "post", json={
                  "email": email,
                  "username": username,
                  "password": password
                })

    def login(self, user, password):
        status, data = self.send_request("/auth/token/login/", "post", json={
                'username': user,
                'password': password
            })
        if status == 200:
            if "auth_token" not in data:
                raise AuthError("No auth_token in response")
            self._session.headers.update({'Authorization':  f'Token {data["auth_token"]}'})
        return status, data

    def get_book(self, uid: str):
        return self.send_request(f"/api/books/{uid}", "get")

    def create_book(self, title, text, video_local_file, send_video_filename):

        video_file = open(video_local_file, 'rb')
        files = [
            ('video', (send_video_filename, video_file, 'video/octet-stream')),
            ('text', ('bookster_book.txt', text, 'text/plain'))
        ]
        payload = {'title ': title}
        rsp = self._session.post(
            f"{self._addr}/api/books/",
            data=payload, files=files
        )
        try:
            return rsp.status_code, rsp.json()
        except:
            traceback.print_exc()
            return rsp.status_code, {}

    def get_index(self):
        resp = self._session.get(
            f"{self._addr}/index.html",
        )
        return resp.status_code, resp.text

    def get_file(self, url, max_size=1024 * 1024 * 1024):
        try:
            parsed = urlparse(url)
            if parsed.scheme != 'http':
                raise ParseError(f"Can't parse url: {url}")
            if parsed.netloc != f"{self._host}:{self._port}":
                raise ParseError(f"Can't parse url: {url}")
            if not parsed.path.startswith('/media/'):
                raise ParseError(f"Can't parse url: {url}")
            if parsed.query:
                raise ParseError(f"Can't parse url: {url}")
            resp = self._session.get(url, stream=True)
            if resp.status_code != 200:
                return resp.status_code, None
            hashed = get_hash_stream(resp, max_size)
            return resp.status_code, hashed
        except Exception as e:
            raise ParseError(f"Can't parse url: {url}") from e


if __name__ == '__main__':
    api = Api('localhost', 8900)
    name = generate_name()
    email = generate_email()
    password = generate_password()
    status, data = api.signup(email, name, password)
    print(status, data)
    status, data = api.login(name, password)
    print(status, data)
    status, data = api.get_index()
    print(status, data)
    test_video = os.path.join('videos', 'sample_1.avi')
    status, data = api.create_book('test', 'test flag', test_video, 'test.mp4')
    uid = data['uid']
    print(status, data)
    print(uid)
    status, data = api.get_book(uid)
    print(status, data)
    status, data = api.get_file(data['video'], os.path.getsize(test_video))
    print(status, data)
    file_hash = get_hash(test_video)
    print(file_hash == data)
    time.sleep(5)
    status, data = api.get_book(uid)
    print(status, data)
    status, data = api.get_file(data['video_preview'])
    print(status, data)


