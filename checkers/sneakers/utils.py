import random
import string

from gornilo import Verdict

SEED = string.ascii_letters + string.digits


def get_rnd_string(length: int = 32):
    return ''.join(random.choice(SEED) for _ in range(length))


def get_rnd_int(min_val=0, max_val: int = 2):
    return random.randint(min_val, max_val)


def raise_data_exc(obj, actual):
    raise VerdictDataException(Verdict.CORRUPT(
        f"Received incorrect {obj}."),
        f"Actual was: {actual}.")


def raise_not_found_exc(obj, actual):
    raise VerdictNotFoundException(Verdict.CORRUPT(
        f"Could not find recently added {obj}."),
        f"Actual was: {actual}.")


class VerdictHttpException(Exception):
    def __init__(self, verdict=None, message: str = None):
        self.verdict = verdict
        self.message = message

    def __str__(self):
        return f"{str(self.verdict._public_message)}||{self.message}"


class VerdictDataException(Exception):
    def __init__(self, verdict=None, message: str = None):
        self.verdict = verdict
        self.message = message

    def __str__(self):
        return f"{str(self.verdict._public_message)}||{self.message}"


class VerdictNotFoundException(Exception):
    def __init__(self, verdict=None, message: str = None):
        self.verdict = verdict
        self.message = message

    def __str__(self):
        return f"{str(self.verdict._public_message)}||{self.message}"
