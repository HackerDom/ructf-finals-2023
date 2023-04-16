from gornilo import Verdict

from models import Artist, Sneakers
from requests import Response
from gornilo.http_clients import requests_with_retries
from utils import raise_not_found_exc, raise_data_exc, VerdictHttpException


class Client:
    def __init__(self, host, port=7777):
        self.api_prefix = f"http://{host}:{port}/"

    def create_artist_with_checks(self, artist: Artist) -> Artist:
        resp = self.send_create_artist(artist).json()

        # NOTE: maybe from_json() with Verdict.Corrupt
        artist_id = resp["id"]["value"]
        name = resp["name"]
        token = resp["token"]["value"]
        result = Artist(artist_id, name, token)

        received_artist = self.send_get_artist(token).json()

        if not result.compare(received_artist):
            raise_data_exc("Artist", received_artist)

        return result

    def create_sneakers_with_checks(self, sneakers_with_flag: Sneakers) -> Sneakers:
        resp = self.send_create_sneakers(sneakers_with_flag).json()

        # NOTE: maybe add from_json() with Verdict.Corrupt
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

        # NOTE: maybe fetch sneakers and compare

        return result

    def download_sneakers_image(self, download_token: str):
        return self.send_show_image(download_token)

    def get_gallery_available(self):
        self.send_get_available().json()

    def get_gallery_exhibits(self, filters: list, ids_to_search: list, artist_token: str):
        return self.send_get_exhibits(filters, ids_to_search, artist_token).json()

    def send_create_artist(self, artist: Artist):
        return self._send_request(
            "artist",
            requests_with_retries().post,
            json_data={"Id": {"Value": f"{artist.artist_id}"}, "Name": artist.name})

    def send_get_artist(self, artist_token: str):
        return self._send_request(
            "artist",
            requests_with_retries().get,
            api_key=artist_token)

    def send_create_sneakers(self, sneakers: Sneakers):
        return self._send_request(
            "sneakers/masterpiece",
            requests_with_retries().post,
            json_data={"Collection": sneakers.collection, "Description": sneakers.description},
            api_key=sneakers.owner.token)

    def send_show_image(self, download_token: str):
        return self._send_request(
            f"gallery/show/{download_token}",
            requests_with_retries().get)

    def send_get_available(self):
        return self._send_request(
            f"gallery/available",
            requests_with_retries().get)

    def send_get_exhibits(self, filters: list, ids_to_search: list, artist_token: str):
        return self._send_request(
            f"gallery/exhibits",
            requests_with_retries().post,
            json_data={"Conditions": filters, "IdsToSearch": ids_to_search},
            api_key=artist_token)

    def _send_request(self, url, request, json_data=None, api_key=None, params=None) -> Response:
        headers = {"Authorization": f"{api_key}"} if api_key is not None else None

        absolute_url = self.api_prefix + url

        try:
            response = request(url=absolute_url, json=json_data, headers=headers, params=params)
        except Exception as e:
            raise VerdictHttpException(Verdict.DOWN("HTTP error."))

        if response.status_code != 200:
            text = response.text
            message = f"Invalid status code: {response.status_code} for {url}."
            private_msg = f"resp: {text}. was send: json={json_data};headers={headers};params={params}"

            if "AuthenticationException" in text:
                raise_not_found_exc("Artist", api_key)

            raise VerdictHttpException(Verdict.MUMBLE(message), private_msg)

        return response
