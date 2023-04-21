import requests
from requests import Response
from gornilo.http_clients import requests_with_retries
from utils import raise_http_error, raise_invalid_http_code_error


class Client:
    def __init__(self, host, port=6969):
        self.api_url = f"http://{host}:{port}/"

    def register_museum(self, name: str, password: str) -> str:
        resp = self._send_request(
            "register",
            requests_with_retries().post,
            json_data={"name": name, "password": password}
        ).json()
        return resp.get("token", "")

    def login_museum(self, id: str, password: str) -> str:
        resp = self._send_request(
            "login",
            requests_with_retries().post,
            json_data={"id": id, "password": password}
        ).json()

        return resp.get("token", "")

    def get_museum(self, id: str):
        return self._send_request(
            f"museums/{id}",
            requests_with_retries().get,
        ).json()

    def get_museums_list(self):
        return self._send_request(
            f"museums",
            requests_with_retries().get,
        ).json()

    def create_exhibit(self, title: str, description: str, metadata: str, token: str):
        return self._send_request(
            f"museum/exhibits",
            requests_with_retries().post,
            json_data={
                "title": title,
                "description": description,
                "metadata": metadata,
            },
            auth_token=token,
        ).json()

    def get_exhibit(self, id: str, token: str):
        return self._send_request(
            f"museum/exhibits/{id}",
            requests_with_retries().get,
            auth_token=token,
        ).json()

    def get_exhibit_list_by_search(self, search: str, token: str):
        return self._send_request(
            f"museum/exhibits?search={search}",
            requests_with_retries().get,
            auth_token=token,
        ).json()

    def _send_request(self, url, request, json_data=None, auth_token=None) -> Response:
        headers = {"token": auth_token} if auth_token else None
        abs_url = self.api_url + url
        try:
            response = request(url=abs_url + "a", json=json_data, headers=headers)
        except requests.ConnectionError:
            raise raise_http_error("Connection error")
        except requests.Timeout:
            raise raise_http_error("Timeout error")
        except requests.RequestException:
            raise raise_http_error("HTTP error")

        if response.status_code == 200:
            return response
        verdict_msg = f"Invalid status code: {response.status_code} for url {url}"
        msg = f"response: {response.text[:200]}\nsend: json={json_data} headers={headers}"
        raise raise_invalid_http_code_error(verdict_msg, msg)
