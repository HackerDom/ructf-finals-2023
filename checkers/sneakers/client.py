import requests
from gornilo import Verdict
from gornilo.http_clients import requests_with_retries
from requests import Response

from models import Artist, Sneakers
from utils import raise_not_found_exc, raise_data_exc, VerdictHttpException


class Client:
    def __init__(self, host, port=7777):
        self.api_prefix = f"http://{host}:{port}/"

    def create_artist_with_checks(self, artist: Artist) -> Artist:
        resp = self.send_create_artist(artist).json()

        result = None
        try:
            artist_id = resp["id"]["value"]
            name = resp["name"]
            token = resp["token"]["value"]
            result = Artist(artist_id, name, token)
        except Exception:
            raise_data_exc("Artist", resp)

        received_artist = self.send_get_artist(result.token).json()

        if not result.compare(received_artist):
            raise_data_exc("Artist", received_artist)

        return result

    def create_sneakers_with_checks(self, sneakers_with_flag: Sneakers) -> Sneakers:
        resp = self.send_create_sneakers(sneakers_with_flag).json()

        try:
            sneakers_id = resp["id"]["value"]
            owner = Artist(
                resp["owner"]["id"]["value"],
                resp["owner"]["name"],
                resp["owner"]["token"]["value"]
            )
            collection = resp["collection"]
            descr = resp["description"]
            token = resp["downloadToken"]["value"]
            result = Sneakers(sneakers_id, owner, collection, descr, token)
            return result
        except Exception:
            raise_data_exc("Masterpiece", resp)

    def download_sneakers_image(self, download_token: str):
        return self.send_show_image(download_token)

    def get_gallery_available(self):
        self.send_get_available().json()

    def get_gallery_exhibits(self, filters: list, ids_to_search: list, artist_token: str):
        return self.send_get_exhibits(filters, ids_to_search, artist_token).json()

    def send_create_artist(self, artist: Artist):
        return self._send_request(
            "artist",
            self._post(),
            json_data={"Id": {"Value": f"{artist.artist_id}"}, "Name": artist.name})

    def send_get_artist(self, artist_token: str):
        return self._send_request(
            "artist",
            self._get(),
            api_key=artist_token)

    def send_create_sneakers(self, sneakers: Sneakers):
        return self._send_request(
            "sneakers/masterpiece",
            self._post(),
            json_data={"Collection": sneakers.collection, "Description": sneakers.description},
            api_key=sneakers.owner.token)

    def send_show_image(self, download_token: str):
        return self._send_request(
            f"gallery/show/{download_token}",
            self._get())

    def send_get_available(self):
        return self._send_request(
            f"gallery/available",
            self._get())

    def send_get_exhibits(self, filters: list, ids_to_search: list, artist_token: str):
        return self._send_request(
            f"gallery/exhibits",
            self._post(),
            json_data={"Conditions": filters, "IdsToSearch": ids_to_search},
            api_key=artist_token)

    def _post(self):
        return requests_with_retries(status_forcelist=(500, 502)).post

    def _get(self):
        return requests_with_retries(status_forcelist=(500, 502)).get

    def _send_request(self, url, request, json_data=None, api_key=None, params=None) -> Response:
        headers = {"Authorization": f"{api_key}"} if api_key is not None else None

        absolute_url = self.api_prefix + url

        try:
            response = request(url=absolute_url, json=json_data, headers=headers, params=params)
        except requests.ConnectionError:
            raise VerdictHttpException(Verdict.DOWN("Connection error."))
        except requests.Timeout:
            raise VerdictHttpException(Verdict.DOWN("Timeout."))
        except requests.RequestException:
            raise VerdictHttpException(Verdict.MUMBLE("Http error."))

        if response.status_code == 200:
            return response

        text = response.text
        message = f"Invalid status code: {response.status_code}."
        private_msg = f"resp: {text}. was send: url={url}; json={json_data}; headers={headers}; params={params}"

        if response.status_code == 404:
            return raise_not_found_exc("object", private_msg)

        if "AuthenticationException" in text:
            raise_not_found_exc("Artist", api_key)

        raise VerdictHttpException(Verdict.MUMBLE(message), private_msg)
