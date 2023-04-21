from gornilo.http_clients import requests_with_retries
from requests import Response


class Api:
    def __init__(self, host, port=18888):
        self.base_url = f'http://{host}:{port}'
        self.session = requests_with_retries(status_forcelist=(500, 502))

    def front(self):
        return self.session.get(f"{self.base_url}")

    def register(self, username: str, password: str) -> Response:
        return self.session.post(f"{self.base_url}/api/register", data={"username": username, "password": password})

    def login(self, username: str, password: str) -> Response:
        return self.session.post(f"{self.base_url}/api/login", data={"username": username, "password": password})

    def create_joke(self, token: str, status: str, theme: str, text: str) -> Response:
        return self.session.post(f"{self.base_url}/api/jokes", data={"status": status, "theme": theme, "text": text},
                                 headers={"Authorization": f"Bearer {token}"})

    def get_joke(self, token: str, username: str, theme: str) -> Response:
        return self.session.get(f"{self.base_url}/api/jokes", data={"username": username, "theme": theme},
                                headers={"Authorization": f"Bearer {token}"})

    def list_jokes(self, token: str) -> Response:
        return self.session.get(f"{self.base_url}/api/jokes/list", headers={"Authorization": f"Bearer {token}"})

    def theme_jokes(self, token: str, theme: str) -> Response:
        return self.session.get(f"{self.base_url}/api/jokes/list/{theme}", headers={"Authorization": f"Bearer {token}"})

    def friends_list(self, token: str) -> Response:
        return self.session.get(f"{self.base_url}/api/friends", headers={"Authorization": f"Bearer {token}"})

    def requests_list(self, token: str) -> Response:
        return self.session.get(f"{self.base_url}/api/friends/requests", headers={"Authorization": f"Bearer {token}"})

    def friend_request(self, token: str, to: str) -> Response:
        return self.session.post(f"{self.base_url}/api/friends/request", data={"to": to},
                                 headers={"Authorization": f"Bearer {token}"})

    def accept_request(self, token: str, fr: str) -> Response:
        return self.session.post(f"{self.base_url}/api/friends/accept", data={"from": fr},
                                 headers={"Authorization": f"Bearer {token}"})
