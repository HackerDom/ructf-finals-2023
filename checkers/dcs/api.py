#!/usr/bin/env python3

import gornilo.http_clients

from dataclasses import dataclass
from typing import Self


MAX_CODE_SIZE = 10_000
MAX_DESCRIPTION_SIZE = 10_000


@dataclass
class CreateResult:
    error: str
    token: str


@dataclass
class GetResult:
    error: str
    executeResult: str
    description: str


class DCSClient:
    def __init__(self: Self, host: str, port: int):
        self._addr = f'http://{host}:{port}'

    def create(self: Self, code: str, description: str) -> CreateResult:
        with gornilo.http_clients.requests_with_retries() as session:
            with session.post() as r:
                pass

    def get(self: Self, token: str) -> GetResult:
        pass
