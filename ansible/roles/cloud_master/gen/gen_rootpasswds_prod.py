#!/usr/bin/python3

import sys
import os
import secrets

N = 60
S = 3

def genpass(n=12):
 abc = "abcdefgkmnrtxyzABCDEFGKMNRTXYZ23456789"
 return "".join([secrets.choice(abc) for i in range(n)])

os.chdir(os.path.dirname(os.path.realpath(__file__)))

try:
    os.mkdir("passwds_prod")
except FileExistsError:
    print("Remove ./passwds_prod dir first")
    sys.exit(1)


for t in range(1, N):
    for s in range(1, S+1):
        open("passwds_prod/team%d_serv%d_root_passwd.txt" % (t, s), "w").write(genpass()+"\n")

