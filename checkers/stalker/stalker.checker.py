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
        note: model.Note | None,
        title: str,
        visible: bool,
        content: str | None,
        viewers: Iterable[str] | None,
        owner: str,
) -> None:
    if note is None:
        raise model.ValidationError('failed to get note')

    if note.title != title:
        raise model.ValidationError('invalid note title')

    if note.visible != visible:
        raise model.ValidationError('invalid note visible')

    if content is not None:
        if note.content != content:
            raise model.ValidationError('invalid note content')
    else:
        if note.content is not None:
            raise model.ValidationError('invalid note content')

    if viewers is not None:
        if note.viewers is None or not set(viewers).issubset(set(note.viewers)):
            raise model.ValidationError('invalid note viewers')
    else:
        if note.viewers is not None:
            raise model.ValidationError('invalid note viewers')

    if note.owner != owner:
        raise model.ValidationError('invalid note owner')


def check_user_object(
        user: model.User | None,
        username: str,
        owned_notes: Iterable[str],
        shared_notes: Iterable[str] | None,
) -> None:
    if user is None:
        raise model.ValidationError('failed to get user')

    if user.name != username:
        raise model.ValidationError('invalid user name')

    if not set(owned_notes).issubset(set(user.owned_notes)):
        raise model.ValidationError('invalid user owned_notes')

    if shared_notes is not None:
        if user.shared_notes is None or not set(shared_notes).issubset(set(user.shared_notes)):
            raise model.ValidationError('invalid user shared_notes')
    else:
        if user.shared_notes is not None:
            raise model.ValidationError('invalid user shared_notes')


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

    if note is None:
        return gornilo.Verdict.CORRUPT('note with flag not found')

    if note.content != request.flag:
        return gornilo.Verdict.CORRUPT('invalid flag')

    if not api.user_logout():
        return gornilo.Verdict.MUMBLE('failed to logout')

    return gornilo.Verdict.OK()


def do_check(request: gornilo.CheckRequest) -> gornilo.Verdict:
    api1 = client.Api(request.hostname, port)
    username1, password1 = generate_credentials()

    if not api1.user_register(username1, password1):
        return gornilo.Verdict.MUMBLE('failed to register')

    check_user_object(
        api1.user_get(username1),
        username1, [], [],
    )

    note1_title = random_string(16)
    note1_visible = True
    note1_content = random_string(32)

    if not api1.note_create(note1_title, note1_visible, note1_content):
        return gornilo.Verdict.MUMBLE('failed to create note')

    check_note_object(
        api1.note_get(note1_title),
        note1_title, note1_visible, note1_content, [], username1,
    )

    note2_title = random_string(16)
    note2_visible = False
    note2_content = random_string(32)

    if not api1.note_create(note2_title, note2_visible, note2_content):
        return gornilo.Verdict.MUMBLE('failed to create note')

    check_note_object(
        api1.note_get(note2_title),
        note2_title, note2_visible, note2_content, [], username1,
    )

    check_user_object(
        api1.user_get(username1),
        username1, [note1_title, note2_title], [],
    )

    api2 = client.Api(request.hostname, port)
    username2, password2 = generate_credentials()

    if not api2.user_register(username2, password2):
        return gornilo.Verdict.MUMBLE('failed to register')

    check_note_object(
        api2.note_get(note1_title),
        note1_title, note1_visible, note1_content, None, username1,
    )

    check_note_object(
        api2.note_get(note2_title),
        note2_title, note2_visible, None, None, username1,
    )

    if not api1.note_share(note2_title, username2):
        return gornilo.Verdict.MUMBLE('failed to share note')

    check_note_object(
        api2.note_get(note2_title),
        note2_title, note2_visible, note2_content, None, username1,
    )

    check_note_object(
        api1.note_get(note2_title),
        note2_title, note2_visible, note2_content, [username2], username1,
    )

    if not api1.note_deny(note2_title, username2):
        return gornilo.Verdict.MUMBLE('failed to deny note')

    check_note_object(
        api2.note_get(note2_title),
        note2_title, note2_visible, None, None, username1,
    )

    check_note_object(
        api1.note_get(note2_title),
        note2_title, note2_visible, note2_content, [], username1,
    )

    if not api1.note_destroy(note2_title):
        return gornilo.Verdict.MUMBLE('failed to destroy note')

    if api2.note_get(note2_title) is not None:
        return gornilo.Verdict.MUMBLE('note is not destroyed')

    api3 = client.Api(request.hostname, port)

    check_note_object(
        api3.note_get(note1_title),
        note1_title, note1_visible, note1_content, None, username1,
    )

    check_user_object(
        api3.user_get(username1),
        username1, [note1_title], None,
    )

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