import jwt
from gornilo import Verdict
from jwt import decode


def raise_wrong_data_exc(obj, actual):
    raise VerdictException(
        Verdict.CORRUPT(f"Received incorrect object {obj}"),
        f"Actual object is: {actual}"
    )


def raise_recently_not_found_exc(obj, actual):
    raise VerdictException(
        Verdict.CORRUPT(f"Recently added object {obj} not found"),
        f"Actual object is: {actual}"
    )


def raise_http_error():
    raise VerdictException(
        Verdict.DOWN("HTTP error")
    )


def raise_invalid_http_code_error(verdict_msg, message):
    raise VerdictException(
        Verdict.MUMBLE(verdict_msg),
        message
    )


class VerdictException(Exception):
    def __init__(self, verdict=None, message=None):
        self.verdict = verdict
        self.message = message

    def __str__(self):
        return f"{str(self.verdict._public_message)} : {self.message}"


def get_id_from_token(token: str):
    return jwt.decode(token, algorithms='HS256', options={"verify_signature": False}).get("id")
