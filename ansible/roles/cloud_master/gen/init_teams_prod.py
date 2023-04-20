#!/usr/bin/python3

import sys
import os
import shutil
import crypt

N = 60
S = 16

DB_DIR = "db_prod"

if __name__ != "__main__":
    print("I am not a module")
    sys.exit(0)

os.chdir(os.path.dirname(os.path.realpath(__file__)))

try:
    os.mkdir(DB_DIR)
except FileExistsError:
    print(f"Remove {DB_DIR} dir first")
    sys.exit(1)

for t in range(1, N):
    os.mkdir(DB_DIR + "/team%d" % t)

    open(DB_DIR + "/team%d/deploy_method" % t, "w").write("UNKNOWN")
    open(DB_DIR + "/team%d/net_deploy_state" % t, "w").write("NOT_STARTED")
    open(DB_DIR + "/team%d/team_state" % t, "w").write("NOT_CLOUD")
    
    shutil.copyfile("client_entergame_prod/%d.conf" % t,
                    DB_DIR + "/team%d/client_entergame.ovpn" % t)
    shutil.copyfile("server_outside_prod/%d.conf" % t,
                    DB_DIR + "/team%d/server_outside.conf" % t)

    shutil.copyfile("openvpn_team_main_net_client_prod/%d.conf" % t,
                    DB_DIR + "/team%d/game_network.conf" % t)

    #shutil.copyfile("passwds_prod/team%d_root_passwd.txt" % i,
    #                DB_DIR + "/team%d/root_passwd.txt" % i)

    shutil.copyfile("tokens_hashed_prod/%d.txt" % t, DB_DIR + "/team%d/token_hash.txt" % t)

    for s in range(1, S+1):
        open(DB_DIR + "/team%d/serv%d_image_deploy_state" % (t, s), "w").write("NOT_STARTED")
        open(DB_DIR + "/team%d/serv%d_open_state" % (t, s), "w").write("OPEN")

        root_passwd_filename = "passwds_prod/team%d_serv%d_root_passwd.txt" % (t, s)
        root_passwd = open(root_passwd_filename).read().strip()
        root_passwd_copy_filename = DB_DIR + "/team%d/serv%d_root_passwd.txt" % (t, s)
        open(root_passwd_copy_filename, "w").write(root_passwd + "\n")
        root_passwd_hash = crypt.crypt(root_passwd, crypt.METHOD_SHA512)
        root_passwd_hash_filename = DB_DIR + "/team%d/serv%d_root_passwd_hash.txt" % (t, s)
        open(root_passwd_hash_filename, "w").write(root_passwd_hash + "\n")
