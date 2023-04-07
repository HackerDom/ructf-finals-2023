#!/usr/bin/env sage

import re
import sys
import json
import dataclasses

import requests


IP = sys.argv[1] if len(sys.argv) > 1 else 'localhost'
PORT = 17173


@dataclasses.dataclass
class Keyspace:
    n: int
    m: int
    modulo: int


def parse_ciphertext(content):
    obj = json.loads(content)

    matrix = []

    for element in obj:
        row = []

        for value in element:
            row.append(int(value, 10))

        matrix.append(row)

    return Matrix(matrix)


def parse_keyspace(content):
    parts = re.findall(r'\d+', content)

    n = int(parts[0])
    m = int(parts[1])
    modulo = int(parts[2])

    return Keyspace(n, m, modulo)


def get_keyspace(username):
    url = f'http://{IP}:{PORT}/api/storage/keyspace'
    params = {
        'username': username,
    }

    resp = requests.get(url, params = params)

    return parse_keyspace(resp.text)


def get_ciphertext(ciphertext_id):
    url = f'http://{IP}:{PORT}/api/storage/ciphertext'
    params = {
        'id': ciphertext_id,
    }

    resp = requests.get(url, params = params)

    return parse_ciphertext(resp.text)


def attack(keyspace, ciphertext):
    d = keyspace.n
    N = keyspace.modulo

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


def main():
    # flag_id = 'username|ciphertext_id'
    flag_id = 'UvtDiu0Zu8d4DU5W|e9fce3eb799c5ba1e17e241f3c18dcad'

    username, ciphertext_id = flag_id.split('|')

    keyspace = get_keyspace(username)
    ciphertext = get_ciphertext(ciphertext_id)

    for flag in attack(keyspace, ciphertext):
        print(flag)


if __name__ == '__main__':
    main()
