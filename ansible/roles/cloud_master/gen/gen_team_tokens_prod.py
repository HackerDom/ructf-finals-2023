#!/usr/bin/python3

import sys
import os
import secrets
import hashlib

N = 60

def gentoken(team, n=32):
 abc = "abcdef0123456789"
 return "CLOUD_" + str(team) + "_" + "".join([secrets.choice(abc) for i in range(n)])

os.chdir(os.path.dirname(os.path.realpath(__file__)))

try:
    os.mkdir("tokens_prod")
except FileExistsError:
    print("Remove ./tokens_prod dir first")
    sys.exit(1)

try:
    os.mkdir("tokens_hashed_prod")
except FileExistsError:
    print("Remove ./tokens_hashed_prod dir first")
    sys.exit(1)

for i in range(1, N):
    token = gentoken(i)
    token_hashed = hashlib.sha256(token.encode()).hexdigest()
    open("tokens_prod/%d.txt" % i, "w").write(token + "\n")
    open("tokens_hashed_prod/%d.txt" % i, "w").write(token_hashed + "\n")
