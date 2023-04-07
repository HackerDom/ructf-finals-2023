#!/usr/bin/env python3

import re
import json
import hashlib
import dataclasses
from typing import Self

import numpy as np


class ProtocolError(Exception):
    pass


class ValidationError(Exception):
    pass


class Ciphertext(np.matrix):
    def serialize(self: Self) -> str:
        obj = self.tolist()

        matrix = [
            [str(x) for x in row] for row in obj
        ]

        return json.dumps(matrix)

    @staticmethod
    def parse(data: str) -> 'Ciphertext':
        try:
            obj = json.loads(data)
        except Exception:
            raise ProtocolError('invalid ciphertext format')

        if not isinstance(obj, list):
            raise ProtocolError('invalid ciphertext format')

        matrix = []
        width = -1

        for element in obj:
            row = []

            if not isinstance(element, list):
                raise ProtocolError('invalid ciphertext format')

            if width < 0:
                width = len(element)

            if len(element) != width:
                raise ProtocolError('invalid ciphertext format')

            for value in element:
                if not isinstance(value, str):
                    raise ProtocolError('invalid ciphertext format')

                try:
                    row.append(int(value, 10))
                except Exception:
                    raise ProtocolError('invalid ciphertext format')

            matrix.append(row)

        return Ciphertext(matrix, dtype = object)


@dataclasses.dataclass
class Keyspace:
    n: int
    m: int
    modulo: int

    @staticmethod
    def parse(data: str) -> 'Keyspace':
        parts = re.findall(r'\d+', data)

        if len(parts) != 3:
            raise ProtocolError('invalid keyspace format')

        try:
            n = int(parts[0])
            m = int(parts[1])
            modulo = int(parts[2])

            return Keyspace(n, m, modulo)
        except Exception:
            raise ProtocolError('invalid keyspace format')
        
    def hash(self: Self) -> str:
        data = ','.join(
            [str(self.n), str(self.m), str(self.modulo)],
        )

        return hashlib.sha256(data.encode()).hexdigest()
