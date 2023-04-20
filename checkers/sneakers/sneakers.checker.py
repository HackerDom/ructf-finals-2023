#!/usr/bin/env python3.11

import json
import traceback
from io import BytesIO

import piexif
from PIL import Image
from gornilo import Verdict, PutRequest, GetRequest, CheckRequest, VulnChecker, NewChecker

from client import Client
from models import Artist, Sneakers
from utils import VerdictDataException, VerdictHttpException, VerdictNotFoundException, get_rnd_string, raise_data_exc, \
    get_rnd_int

checker = NewChecker()


def check_reducer_color_filters(client, sneakers, ids_to_search):
    value = get_rnd_int(1, 250)

    first_filter = {"ComparisonOperator": "Greater", "ChannelToCompare": "G", "Operand": value}
    second_filter = {"ComparisonOperator": "Greater", "ChannelToCompare": "G", "Operand": value + 2}
    filters = [
        {"Type": "Color", "JsonString": json.dumps(first_filter)},
        {"Type": "Color", "JsonString": json.dumps(second_filter)}
    ]

    available_json = client.get_gallery_exhibits(filters, ids_to_search, sneakers.owner.token)
    applied = int(available_json["filtersApplied"])
    if applied != 2:
        raise_data_exc("Applied filters", f"Applied {applied}")

    sneakers_from_service = available_json["items"][0]
    if sneakers_from_service["downloadToken"]["value"] != sneakers.token or \
            sneakers_from_service["id"]["value"] != sneakers.sneakers_id:
        public_message = f"sneakers_token '{sneakers.token}'; " \
                         f"sneakers_id '{sneakers.sneakers_id}';" \
                         f" {json.dumps(available_json)}"
        raise_data_exc("Gallery items", public_message)


def check_reducer_collection_filters(client: Client, sneakers: Sneakers, ids_to_search):
    collection = sneakers.collection.capitalize().replace("velvet", "Velvet")
    serialized_filter = json.dumps({"Collection": collection})
    filters = [
        {"Type": "Collection", "JsonString": serialized_filter},
        {"Type": "Collection", "JsonString": serialized_filter}
    ]

    available_json = client.get_gallery_exhibits(filters, ids_to_search, sneakers.owner.token)
    applied = int(available_json["filtersApplied"])
    if applied != 2:
        raise_data_exc("Applied filters", f"Applied {applied}")

    sneakers_from_service = available_json["items"][0]
    if sneakers_from_service["downloadToken"]["value"] != sneakers.token or \
            sneakers_from_service["id"]["value"] != sneakers.sneakers_id:
        public_message = f"sneakers_token '{sneakers.token}'; " \
                         f"sneakers_id '{sneakers.sneakers_id}';" \
                         f" {json.dumps(available_json)}"
        raise_data_exc("Gallery items", public_message)


def check_exhibits(client: Client, sneakers: Sneakers):
    ids_to_search = [{"Value": sneakers.sneakers_id}]
    print("Checking reducing collection filters...")
    check_reducer_collection_filters(client, sneakers, ids_to_search)

    print("Checking reducing color filters...")
    check_reducer_color_filters(client, sneakers, ids_to_search)


@checker.define_check
def check(request: CheckRequest) -> Verdict:
    client = Client(request.hostname)
    artist = Artist.new()

    try:
        print("Checking Artist controllers...")
        artist = client.create_artist_with_checks(artist)

        print("Checking Sneakers controllers...")
        sneakers = Sneakers.new(artist, get_rnd_string())
        sneakers = client.create_sneakers_with_checks(sneakers)

        print("Checking Gallery available...")
        client.get_gallery_available()

        print("Checking Gallery exhibits...")
        check_exhibits(client, sneakers)

        print("Checking Gallery download...")
        client.download_sneakers_image(sneakers.token)

        print("Что за тяги, такие бархатные тяги, ребята.")
        return Verdict.OK()
    except VerdictDataException as e:
        print(e)
        return e.verdict
    except VerdictHttpException as e:
        print(e)
        return e.verdict
    except VerdictNotFoundException as e:
        print(e)
        return e.verdict
    except Exception as e:
        traceback.print_exc()
        return Verdict.MUMBLE("Smth went wrong during CHECK.")


@checker.define_vuln('Flag is sneakers.')
class SneakersChecker(VulnChecker):
    @staticmethod
    def put(request: PutRequest) -> Verdict:
        client = Client(request.hostname)
        flag = request.flag

        artist = Artist.new()

        try:
            print(f"Create Artist: {artist.artist_id}.")
            artist = client.create_artist_with_checks(artist)
            sneakers_with_flag = Sneakers.new(artist, flag)

            print(f"Create Sneakers.")
            sneakers_with_flag = client.create_sneakers_with_checks(sneakers_with_flag)

            print(f"Saved flag {flag}.")
            flag_id = json.dumps({
                "sneakers_token": sneakers_with_flag.token,
                "artist_token": artist.token
            })

            print(f"Saved flag_id {flag_id}.")

            return Verdict.OK_WITH_FLAG_ID(sneakers_with_flag.sneakers_id, flag_id)
        except VerdictDataException as e:
            print(e)
            return e.verdict
        except VerdictHttpException as e:
            print(e)
            return e.verdict
        except Exception as e:
            traceback.print_exc()
            return Verdict.MUMBLE("Couldn't put flag!")

    @staticmethod
    def get(request: GetRequest) -> Verdict:
        client = Client(request.hostname)
        flag = request.flag
        flag_id = json.loads(request.flag_id)
        sneakers_token = flag_id["sneakers_token"]

        try:
            image = client.download_sneakers_image(sneakers_token)
            image_description = SneakersChecker.extract_description(image.content)

            if image_description == flag:
                print(f"Successfully got flag '{image_description}'.")
                return Verdict.OK()

            print("Image description doesn't contain a correct flag.")
            print(f"expected: '{flag}' but was: '{image_description}'")

            return Verdict.CORRUPT("Flag is missing!")
        except VerdictDataException as e:
            print(e)
            return e.verdict
        except VerdictHttpException as e:
            print(e)
            return e.verdict
        except Exception as e:
            traceback.print_exc()
            return Verdict.MUMBLE("Couldn't get flag!")

    @staticmethod
    def extract_description(image_content_bytes) -> str:
        image = Image.open(BytesIO(image_content_bytes))

        exif_dict = piexif.load(image.info["exif"])

        if "0th" not in exif_dict:
            return ""

        flag = exif_dict["0th"][piexif.ImageIFD.ImageDescription]
        return str(flag, 'utf-8')


if __name__ == '__main__':
    checker.run()

    #from gornilo.models.action_names import GET, PUT, CHECK
    #HOST = "localhost"
    #FLAG = "FLAG"
    #STATIC_FLAG_ID = ""
    #checker.run(PUT, HOST, "flag_id", FLAG, 1)
    #checker.run(GET, HOST, STATIC_FLAG_ID, FLAG, 1)
    #checker.run(CHECK, HOST, "flag_id", FLAG, 1)
