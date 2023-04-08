#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Connects vm network to the game network"""

import sys
import time
import os
import traceback

import do_api
from do_tokens import DO_TOKENS
from cloud_common import (log_progress, call_unitl_zero_exit, get_cloud_name,
                          get_image_name,
                          get_service_name_by_num, SSH_OPTS, # SSH_YA_OPTS
                         )

TEAM = int(sys.argv[1])
VMNUM = int(sys.argv[2])

# ROUTER_VM_NAME = "team%d-router" % TEAM

def log_stderr(*params):
    print("Team %d:" % TEAM, *params, file=sys.stderr)


def main():
    net_state = open("db/team%d/net_deploy_state" % TEAM).read().strip()

    if net_state != "READY":
        log_stderr("the network state should be READY")
        return 1

    image_state = open("db/team%d/serv%s_image_deploy_state" % (TEAM, VMNUM)).read().strip()
    if image_state != "RUNNING":
        log_stderr("the image state should be RUNNING")
        return 1

    cloud_name = get_cloud_name(TEAM)
    if not cloud_name:
        log_stderr("no cloud_name, exiting")
        return 1
    token = DO_TOKENS[cloud_name]

    if not do_api.remove_tag_by_vmname(token, get_image_name(TEAM, VMNUM), "team-image-closed"):
        log_stderr("failed to remove tag team-image-closed from droplet")
        return 1

    if not do_api.add_tag_by_vmname(token, get_image_name(TEAM, VMNUM), "team-image"):
        log_stderr("failed to add tag team-image to droplet")
        return 1


    print("msg: OK")
    return 0

if __name__ == "__main__":
    sys.stdout = os.fdopen(1, 'w', 1)
    print("started: %d" % time.time())
    exitcode = 1
    try:
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        exitcode = main()
    except:
        traceback.print_exc()
    print("exit_code: %d" % exitcode)
    print("finished: %d" % time.time())
