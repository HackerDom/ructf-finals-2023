#!/usr/bin/env python3.11

import random
import string
import dataclasses
from typing import Self, Tuple, Iterable, Callable

import gornilo
import requests

import model
import client


port = 17171
checker = gornilo.NewChecker()


@dataclasses.dataclass
class FlagId:
    username: str
    password: str
    note_title: str

    def serialize(self: Self) -> str:
        return '|'.join(
            [self.username, self.password, self.note_title],
        )

    @staticmethod
    def parse(data: str) -> 'FlagId':
        parts = data.split('|')

        if len(parts) != 3:
            raise Exception('invalid flag_id format')

        username, password, note_title = parts

        return FlagId(username, password, note_title)

    def public_id(self: Self) -> str:
        return self.note_title


def random_string(length: int, alpha: str = string.ascii_lowercase + string.digits) -> str:
    symbols = random.choices(alpha, k = length)

    return ''.join(symbols)


def generate_credentials() -> Tuple[str, str]:
    username = random_string(16)
    password = random_string(32)

    return (username, password)


def check_note_object(
        note: model.Note,
        title: str,
        visible: bool,
        content: str | None,
        viewers: Iterable[str] | None,
        owner: str,
) -> None:
    if note.title != title:
        return gornilo.Verdict.MUMBLE('invalid note title')
    
    if note.visible != visible:
        return gornilo.Verdict.MUMBLE('invalid note visible')
    
    if content is not None:
        if note.content != content:
            return gornilo.Verdict.MUMBLE('invalid note content')
    else:
        if note.content is not None:
            return gornilo.Verdict.MUMBLE('invalid note content')
    
    if viewers is not None:
        if note.viewers is None or set(note.viewers) != set(viewers):
            return gornilo.Verdict.MUMBLE('invalid note viewers')
    else:
        if note.viewers is not None:
            return gornilo.Verdict.MUMBLE('invalid note viewers')

    if note.owner != owner:
        return gornilo.Verdict.MUMBLE('invalid note owner')
    

def check_user_object(
        user: model.User,
        username: str,
        owned_notes: Iterable[str],
        shared_notes: Iterable[str] | None,
) -> None:
    if user.name != username:
        return gornilo.Verdict.MUMBLE('invalid user name')

    if set(user.owned_notes) != set(owned_notes):
        return gornilo.Verdict.MUMBLE('invalid user owned_notes')
    
    if shared_notes is not None:
        if user.shared_notes is None or set(user.shared_notes) != set(shared_notes):
            return gornilo.Verdict.MUMBLE('invalid user shared_notes')
    else:
        if user.shared_notes is not None:
            return gornilo.Verdict.MUMBLE('invalid user shared_notes')


def do_put(request: gornilo.PutRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    username, password = generate_credentials()

    if not api.user_register(username, password):
        return gornilo.Verdict.MUMBLE('failed to register')
    
    note_title = random_string(16)

    if not api.note_create(note_title, False, request.flag):
        return gornilo.Verdict.MUMBLE('failed to create note')

    if not api.user_logout():
        return gornilo.Verdict.MUMBLE('failed to logout')

    flag_id = FlagId(username, password, note_title)

    return gornilo.Verdict.OK_WITH_FLAG_ID(flag_id.public_id(), flag_id.serialize())


def do_get(request: gornilo.GetRequest) -> gornilo.Verdict:
    api = client.Api(request.hostname, port)

    flag_id = FlagId.parse(request.flag_id)

    if not api.user_login(flag_id.username, flag_id.password):
        return gornilo.Verdict.MUMBLE('failed to login')

    note = api.note_get(flag_id.note_title)

    if note.content != request.flag:
        return gornilo.Verdict.CORRUPT('invalid flag')

    return gornilo.Verdict.OK()


def do_check(request: gornilo.CheckRequest) -> gornilo.Verdict:
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


@checker.define_vuln('flag_id is note_title')
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