import json

from uuid import uuid4
from random import choice
from models import Museum, Exhibit


class DataGenerator:
    def __init__(self, json_file: str = "data.json"):
        self.data = None
        with open(json_file) as f:
            self.data = json.load(f)

    def random_data(self) -> (Museum, Exhibit):
        elem = choice(self.data)

        return (
            Museum.new(elem["museum"], str(uuid4()) + str(uuid4())),
            Exhibit.new(elem["exhibitName"], elem["exhibitDescription"])
        )
