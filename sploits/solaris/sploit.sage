#!/usr/bin/env sage

import re
import sys
import json
import dataclasses
from typing import Optional, Iterable

import requests


IP = sys.argv[1] if len(sys.argv) > 1 else None
PORT = 17173

FLAG_ID = sys.argv[2] if len(sys.argv) > 2 else None


@dataclasses.dataclass
class Keyspace:
    n: int
    m: int
    modulus: int


class Client:
    def __init__(self, ip: str, port: int) -> None:
        self.url = f'http://{ip}:{port}'

    def get_keyspace(self, username: str) -> Optional[Keyspace]:
        url = f'{self.url}/api/storage/keyspace'
        params = {
            'username': username,
        }

        resp = requests.get(url, params = params)

        if resp.status_code != 200:
            return None

        return self.parse_keyspace(resp.text)

    def get_ciphertext(self, ciphertext_id: str) -> Optional[Matrix]:
        url = f'{self.url}/api/storage/ciphertext'
        params = {
            'id': ciphertext_id,
        }

        resp = requests.get(url, params = params)

        if resp.status_code != 200:
            return None

        return self.parse_ciphertext(resp.text)
    
    def parse_keyspace(self, content: str) -> Keyspace:
        parts = re.findall(r'\d+', content)

        n = int(parts[0])
        m = int(parts[1])
        modulus = int(parts[2])

        return Keyspace(n, m, modulus)
    
    def parse_ciphertext(self, content: str) -> Matrix:
        obj = json.loads(content)

        matrix = []

        for element in obj:
            row = []

            for value in element:
                row.append(int(value, 10))

            matrix.append(row)

        return Matrix(matrix)


def attack(keyspace: Keyspace, ciphertext: Matrix) -> Iterable[bytes]:
    d = keyspace.n
    N = keyspace.modulus

    R = Zmod(N)
    MS = MatrixSpace(R, d, d)

    P = PolynomialRing(R, ', '.join(['x'] + [f'r{i}' for i in range(d - 1)]))
    x, *r = P.gens()

    matrix = MS(ciphertext)

    pols = [
        x^i + sum(ri^i for ri in r) - (matrix ^ i).trace()
        for i in range(1, d + 1)
    ]

    I = Ideal(pols)

    for pol in I.groebner_basis():
        try:
            univariate_pol = pol.univariate_polynomial()
        except Exception:
            continue

        roots = univariate_pol.small_roots(X=2^256, epsilon=0.05)

        for root in roots:
            yield int(root).to_bytes(1024, 'big').strip(b'\x00')


def main() -> None:
    if IP is None:
        raise Exception('pass ip as 1st argument')

    if FLAG_ID is None:
        raise Exception('pass flag_id as 2nd argument')

    # FLAG_ID: 'username|ciphertext_id'
    username, ciphertext_id = FLAG_ID.split('|')

    client = Client(IP, PORT)

    keyspace = client.get_keyspace(username)
    if keyspace is None:
        raise Exception('failed to get keyspace')

    ciphertext = client.get_ciphertext(ciphertext_id)
    if ciphertext is None:
        raise Exception('failed to get ciphertext')

    for flag in attack(keyspace, ciphertext):
        print(flag)


if __name__ == '__main__':
    main()
