#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Takes a snapshot of vm"""

import sys
import time
import os
import traceback
import re

import do_api
from do_tokens import DO_TOKENS
from cloud_common import (log_progress, call_unitl_zero_exit, get_cloud_name,
                          get_image_name, get_snapshot_prefix, #get_cloud_ip,
                          SSH_OPTS #SSH_YA_OPTS
                          )

TEAM = int(sys.argv[1])
VMNUM = int(sys.argv[2])
NAME = sys.argv[3]

# IMAGE_VM_NAME = "team%d" % TEAM

def log_stderr(*params):
    print("Team %d:" % TEAM, *params, file=sys.stderr)


def main():
    if not re.fullmatch(r"[0-9a-zA-Z_]{,64}", NAME):
        print("msg: ERR, name validation error")
        return 1

    image_state = open("db/team%d/serv%d_image_deploy_state" % (TEAM, VMNUM)).read().strip()

    if image_state == "NOT_STARTED":
        print("msg: ERR, vm is not started")
        return 1

    if image_state == "RUNNING":
        cloud_name = get_cloud_name(TEAM)
        if not cloud_name:
            log_stderr("no cloud_name, exiting")
            return 1

        token = DO_TOKENS[cloud_name]

        ids = do_api.get_ids_by_vmname(token, get_image_name(TEAM, VMNUM))
        if not ids:
            log_stderr("failed to find vm")
            return 1

        if len(ids) > 1:
            log_stderr("more than one vm with this name exists")
            return 1

        if NAME == "init":
            print("msg: ERR, init snapshot can't be overwritten")
            return 1

        snapshot_name = get_snapshot_prefix(TEAM, VMNUM) + NAME

        for snapshot in do_api.list_snapshots(token):
            if snapshot.get("name", "") == snapshot_name:
                print("msg: ERR, snapshot already exists")
                return 1

        result = do_api.take_vm_snapshot(token, list(ids)[0], snapshot_name)
        if not result:
            log_stderr("take shapshot failed")
            return 1

        print("msg: OK, snapshoting is in progress, it takes about five minutes")
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
