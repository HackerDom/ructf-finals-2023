#!/usr/bin/env python3.11

import json
import traceback

from gornilo import CheckRequest, Verdict, PutRequest, GetRequest, NewChecker, VulnChecker
from client import Client
from data_generator import DataGenerator
from utils import VerdictException, get_id_from_token, raise_wrong_data_exc

PORT = 6969

checker = NewChecker()

data_gen = DataGenerator()


@checker.define_check
def check_service(request: CheckRequest) -> Verdict:
    client = Client(request.hostname)
    museum, exhibit = data_gen.random_data()

    try:
        print(">>Checking Museum registration...")
        token = client.register_museum(museum.name, museum.password)
        museum.id = get_id_from_token(token)
        museum.token = token

        print(">>Checking Museum login...")
        _ = client.login_museum(museum.id, museum.password)

        print(">>Checking Museum data...")
        museum_json = client.get_museum(museum.id)
        if not museum.compare(museum_json):
            raise raise_wrong_data_exc("Museum", museum_json)

        print(">>Checking Museum List available...")
        museums_json = client.get_museums_list()
        if not museums_json.get("museum_id_list"):
            raise raise_wrong_data_exc("Museum List", museums_json)

        print(">>Checking Exhibit creating...")
        exhibit_json = client.create_exhibit(exhibit.title, exhibit.description, exhibit.metadata, token)
        exhibit.id = exhibit_json.get("id", "")
        if not exhibit.compare(exhibit_json):
            raise raise_wrong_data_exc("Exhibit", exhibit_json)

        print(">>Checking Exhibit get...")
        exhibit_json = client.get_exhibit(exhibit.id, token)
        if not exhibit.compare(exhibit_json):
            raise raise_wrong_data_exc("Exhibit", exhibit_json)

        print(">>Checking Exhibits list...")
        exhibits_json = client.get_exhibit_list_by_search("", token)
        if len(exhibits_json) > 1 or "exhibits" not in exhibits_json:
            raise raise_wrong_data_exc("Exhibits list", exhibits_json)
        if not exhibit.compare(exhibits_json["exhibits"][0]):
            raise raise_wrong_data_exc("Exhibit", exhibits_json["exhibits"][0])

        return Verdict.OK()
    except VerdictException as e:
        print(e)
        return e.verdict
    except Exception as e:
        traceback.print_exc()
        return Verdict.MUMBLE("Somthing went wrong in CHECK")


@checker.define_vuln("flag_id is uuid")
class MuseumorphosisChecker(VulnChecker):

    @staticmethod
    def get(request: GetRequest) -> Verdict:
        client = Client(request.hostname)
        flag = request.flag

        try:
            flag_id = json.loads(request.flag_id)
            museum_id, exhibit_id = request.public_flag_id.split(':')

            print(">>Get exhibit by public flag id...")
            exhibit_json = client.get_exhibit(exhibit_id, flag_id["museum_token"])
            actual_flag = exhibit_json.get("metadata", "")
            if actual_flag == flag:
                print(f">>Successfully got flag '{flag}'")
                return Verdict.OK()
            print("Exhibit metadata doesn't contains a correct flag")
            print(f"expected: '{flag}' but got '{actual_flag}'")

            return Verdict.CORRUPT("Flag is missing!")
        except VerdictException as e:
            print(e)
            return e.verdict
        except Exception as e:
            traceback.print_exc()
            return Verdict.MUMBLE("Couldn't get flag!")

    @staticmethod
    def put(request: PutRequest) -> Verdict:
        client = Client(request.hostname)
        flag = request.flag
        museum, exhibit = data_gen.random_data()

        try:
            print(f">>Register Museum: {museum.name}")
            token = client.register_museum(museum.name, museum.password)
            museum_id = get_id_from_token(token)

            print(f">>Create exhibit: {exhibit.title}")
            exhibit.metadata = flag
            exhibit_json = client.create_exhibit(exhibit.title, exhibit.description, exhibit.metadata, token)
            exhibit.id = exhibit_json.get("id", "")
            if not exhibit.compare(exhibit_json):
                raise raise_wrong_data_exc("Exhibit", exhibit_json)
            exhibit.id = exhibit_json["id"]
            flag_id = json.dumps({
                "museum_token": token,
            })

            print(f">>Saved flag {flag}")
            print(f">>Saved flag_id {flag_id}")

            public_flag_id = f"{museum_id}:{exhibit.id}"
            return Verdict.OK_WITH_FLAG_ID(public_flag_id, flag_id)
        except VerdictException as e:
            print(e)
            return e.verdict
        except Exception as e:
            traceback.print_exc()
            return Verdict.MUMBLE("Couldn't put flag!")


if __name__ == '__main__':
    checker.run()
