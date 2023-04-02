import os.path
import time
import traceback
import uuid

from gornilo.http_clients import requests_with_retries

from checkers.bookster.utils import generate_name, generate_email, generate_password


class Api:
    def __init__(self, host, port, token=None):
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
    print(status, data)
    print(data['uid'])
    status, data = api.get_book(data['uid'])
    print(status, data)
    time.sleep(5)
    status, data = api.get_book(data['uid'])
    print(status, data)


