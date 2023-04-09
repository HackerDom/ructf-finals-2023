#!/usr/bin/env python3

import random
import string
import dataclasses
from typing import Self, Tuple, Callable

import gornilo
import requests

import model
import client


port = 17173
checker = gornilo.NewChecker()

MATRIX_SIZE = 6


@dataclasses.dataclass
class FlagId:
    username: str
    password: str
    ciphertext_id: str
    hashed_keyspace: str

    def serialize(self: Self) -> str:
        return '|'.join(
            [self.username, self.password, self.ciphertext_id, self.hashed_keyspace],
        )

    @staticmethod
    def parse(data: str) -> 'FlagId':
        parts = data.split('|')

        if len(parts) != 4:
            raise Exception('invalid flag_id format')

        username, password, ciphertext_id, hashed_keyspace = parts

        return FlagId(username, password, ciphertext_id, hashed_keyspace)

    def public_id(self: Self) -> str:
        return '|'.join([self.username, self.ciphertext_id])


def int_to_bytes(n: int) -> bytes:
    length = (n.bit_length() + 7) // 8

    return n.to_bytes(length, 'big')


def bytes_to_int(data: bytes) -> int:
    return int.from_bytes(data, 'big')


def random_string(length: int, alpha: str = string.ascii_letters + string.digits) -> str:
    symbols = random.choices(alpha, k = length)

    return ''.join(symbols)


def generate_credentials() -> Tuple[str, str]:
    username = random_string(16)
    password = random_string(32)

    return (username, password)


def validate_ciphertext_id(ciphertext_id: str) -> None:
    if any(symbol not in string.hexdigits for symbol in ciphertext_id):
        raise model.ValidationError('invalid ciphertext_id format')


def validate_ciphertext(keyspace: model.Keyspace, ciphertext: model.Ciphertext) -> None:
    if ciphertext.shape != (keyspace.n, keyspace.m):
        raise model.ValidationError('keyspace does not contain ciphertext')
    
    for x in range(MATRIX_SIZE):
        for y in range(MATRIX_SIZE):
            if not 0 <= ciphertext[x, y] < keyspace.modulo:
                raise model.ValidationError('keyspace does not contain ciphertext')
    
    if ciphertext.shape != (MATRIX_SIZE, MATRIX_SIZE):
        raise model.ValidationError('invalid dimensions of ciphertext')


def do_put(request: gornilo.PutRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    username, password = generate_credentials()

    if not api.register(username, password):
        return gornilo.Verdict.MUMBLE('failed to register')

    if not api.login(username, password):
        return gornilo.Verdict.MUMBLE('failed to login')
    
    keyspace = api.get_keyspace(username)

    if keyspace is None:
        return gornilo.Verdict.MUMBLE('failed to get keyspace')

    message = request.flag.encode()
    ciphertext_id = api.encrypt(username, password, message)

    validate_ciphertext_id(ciphertext_id)

    flag_id = FlagId(username, password, ciphertext_id, keyspace.hash())

    return gornilo.Verdict.OK_WITH_FLAG_ID(flag_id.public_id(), flag_id.serialize())


def do_get(request: gornilo.GetRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    flag_id = FlagId.parse(request.flag_id)

    if not api.login(flag_id.username, flag_id.password):
        return gornilo.Verdict.MUMBLE('failed to login')
    
    keyspace = api.get_keyspace(flag_id.username)

    if keyspace is None:
        return gornilo.Verdict.MUMBLE('failed to get keyspace')
    
    if keyspace.hash() != flag_id.hashed_keyspace:
        return gornilo.Verdict.MUMBLE('keyspace was changed')

    ciphertext = api.get_ciphertext(flag_id.ciphertext_id)

    if ciphertext is None:
        return gornilo.Verdict.MUMBLE('failed to get ciphertext')
    
    validate_ciphertext(keyspace, ciphertext)

    message = api.decrypt(flag_id.username, flag_id.password, ciphertext)

    if message.strip() != request.flag.encode().strip():
        return gornilo.Verdict.CORRUPT('invalid flag')

    k1 = random.randrange(0, keyspace.modulo)
    k2 = 2

    message_number = bytes_to_int(message)

    expected_number = (
        k1 * message_number + pow(message_number, k2, keyspace.modulo)
    ) % keyspace.modulo

    check_matrix = (
        k1 * ciphertext + pow(ciphertext, k2)
    ) % keyspace.modulo

    actual_number = bytes_to_int(
        api.decrypt(flag_id.username, flag_id.password, check_matrix),
    )

    if expected_number != actual_number:
        raise model.ValidationError('cipher is broken')

    return gornilo.Verdict.OK()


def do_check(request: gornilo.CheckRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    username, password = generate_credentials()
    
    message1 = random_string(random.randint(64, 128))
    message2 = random_string(random.randint(64, 128))
    message3 = random_string(random.randint(64, 128))

    if not api.register(username, password):
        return gornilo.Verdict.MUMBLE('failed to register')

    if not api.login(username, password):
        return gornilo.Verdict.MUMBLE('failed to login')
    
    keyspace = api.get_keyspace(username)

    if keyspace is None:
        return gornilo.Verdict.MUMBLE('failed to get keyspace')
    
    ciphertext1 = api.get_ciphertext(
        api.encrypt(username, password, message1),
    )

    ciphertext2 = api.get_ciphertext(
        api.encrypt(username, password, message2),
    )

    ciphertext3 = api.get_ciphertext(
        api.encrypt(username, password, message3),
    )

    for ciphertext in [ciphertext1, ciphertext2, ciphertext3]:
        if ciphertext is None:
            return gornilo.Verdict.MUMBLE('failed to get ciphertext')
        
        validate_ciphertext(keyspace, ciphertext)
    
    k1 = random.randrange(0, keyspace.modulo)
    k2 = random.randrange(0, keyspace.modulo)
    k3 = random.randrange(0, keyspace.modulo)

    message1_number = bytes_to_int(message1.encode())
    message2_number = bytes_to_int(message2.encode())
    message3_number = bytes_to_int(message3.encode())

    expected_number = (
        k1 * message1_number + k2 * message2_number + k3 * message3_number
    ) % keyspace.modulo

    check_matrix = (
        k1 * ciphertext1 + k2 * ciphertext2 + k3 * ciphertext3
    ) % keyspace.modulo

    actual_number = bytes_to_int(
        api.decrypt(username, password, check_matrix),
    )

    if expected_number != actual_number:
        raise model.ValidationError('cipher is broken')

    return gornilo.Verdict.OK()


def wrap_exceptions(
        action: Callable[[gornilo.CheckRequest], gornilo.Verdict],
        request: gornilo.CheckRequest,
) -> gornilo.Verdict:
    try:
        return action(request)
    except model.ProtocolError as error:
        return gornilo.Verdict.MUMBLE(f'protocol error: {error}')
    except model.ValidationError as error:
        return gornilo.Verdict.MUMBLE(f'validation error: {error}')
    except requests.ConnectionError:
        return gornilo.Verdict.DOWN('connection error')
    except requests.Timeout:
        return gornilo.Verdict.DOWN('timeout error')
    except requests.RequestException:
        return gornilo.Verdict.MUMBLE('http error')


@checker.define_vuln('flag_id is username|ciphertext_id')
class SolarisChecker(gornilo.VulnChecker):
    @staticmethod
    def put(request: gornilo.PutRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_put, request)

    @staticmethod
    def get(request: gornilo.GetRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_get, request)

    @checker.define_check
    def check(request: gornilo.CheckRequest) -> gornilo.Verdict:
        return wrap_exceptions(do_check, request)


if __name__ == '__main__':
    checker.run()
