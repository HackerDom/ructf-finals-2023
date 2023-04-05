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


def get_matrix_space(username):
    resp = requests.get(f'http://{IP}:{PORT}/api/storage/keyspace', params = {
        'username': username,
    })

    keyspace = parse_keyspace(resp.text)

    R = IntegerModRing(keyspace.modulo)
    MS = MatrixSpace(R, keyspace.n, keyspace.m)

    return MS


def get_ciphertext(MS, ciphertext_id):
    resp = requests.get(f'http://{IP}:{PORT}/api/storage/ciphertext', params = {
        'id': ciphertext_id,
    })

    ciphertext = parse_ciphertext(resp.text)

    return MS(ciphertext)


def attack(matrix):
    d = matrix.dimensions()[0]
    N = matrix.parent().base_ring().cardinality()

    R = IntegerModRing(N)
    P = PolynomialRing(R, ', '.join(['x'] + [f'r{i}' for i in range(d - 1)]))
    x, *r = P.gens()

    pols = []

    for i in range(1, d + 10):
        pol = x^i + sum(ri^i for ri in r) - (matrix ^ i).trace()
        pols.append(pol)

    basis = Ideal(pols).groebner_basis()

    for pol in basis:
        try:
            pol_x = pol.univariate_polynomial()
        except Exception:
            continue

        roots = pol_x.small_roots(X=2^256, epsilon=0.05)

        for root in roots:
            yield int(root).to_bytes(1024, 'big').strip(b'\x00')


def main():
    # flag_id = 'username|ciphertext_id'
    flag_id = 'Onkr6MyzkdtStPAJ|ddbc5967830784728daaf1de6e1a2ff4'

    username, ciphertext_id = flag_id.split('|')

    MS = get_matrix_space(username)
    ciphertext = get_ciphertext(MS, ciphertext_id)

    for flag in attack(ciphertext):
        print(flag)


if __name__ == '__main__':
    main()
