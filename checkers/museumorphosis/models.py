
class Museum:
    def __init__(self, id: str, name: str, token: str, password: str = None):
        self.id = id
        self.name = name
        self.password = password
        self.token = token

    @staticmethod
    def new(name: str, password: str):
        return Museum("", name, "", password)

    def compare(self, museum_json):
        return self.id == museum_json.get("id") and self.name == museum_json.get("name")


class Exhibit:
    def __init__(self, id: str, title: str, description: str, metadata: str):
        self.id = id
        self.title = title
        self.description = description
        self.metadata = metadata

    @staticmethod
    def new(title: str, description: str):
        return Exhibit("", title, description, "metadata")

    def compare(self, exhibit_json):
        return (
            self.id == exhibit_json.get("id") and
            self.title == exhibit_json.get("title") and
            self.description == exhibit_json.get("description") and
            self.metadata == exhibit_json.get("metadata")
        )
