import random
from uuid import uuid4

from utils import get_rnd_string


class Artist:
    def __init__(self, artist_id: str, name, token: str):
        self.artist_id = artist_id
        self.name = name
        self.token = token

    @staticmethod
    def new():
        return Artist(str(uuid4()), get_rnd_string(15), None)

    def compare(self, artist_json):
        return self.artist_id == artist_json["id"]["value"] and \
               self.name == artist_json["name"] and \
               self.token == artist_json["token"]["value"]


class Sneakers:
    sneakers_collections = ["Velvet", "NeuralVelvet"]

    def __init__(self,
                 sneakers_id: str,
                 owner: Artist,
                 collection: str,
                 description: str,
                 token: str):
        self.sneakers_id = sneakers_id
        self.owner = owner
        self.collection = collection
        self.description = description
        self.token = token

    @staticmethod
    def new(owner: Artist, description: str):
        return Sneakers(
            None,
            owner,
            random.choice(Sneakers.sneakers_collections),
            description,
            None
        )
