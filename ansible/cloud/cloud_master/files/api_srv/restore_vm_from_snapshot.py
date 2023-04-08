#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Restores vm from snapshot"""

import sys
import time
import os
import traceback
import re

import do_api
from do_tokens import DO_TOKENS
from do_settings import CLOUDS
from cloud_common import (log_progress, call_unitl_zero_exit, get_cloud_name,
                          get_image_name, get_snapshot_prefix,
                          get_service_name_by_num, SSH_OPTS, # SSH_YA_OPTS
                         )

TEAM = int(sys.argv[1])
VMNUM = int(sys.argv[2])
NAME = sys.argv[3]
#IMAGE_VM_NAME = "team%d" % TEAM


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


        vm_ids = do_api.get_ids_by_vmname(token, get_image_name(TEAM, VMNUM))
        if not vm_ids:
            log_stderr("failed to find vm")
            return 1

        if len(vm_ids) > 1:
            log_stderr("more than one vm with this name exists")
            return 1

        vm_id = list(vm_ids)[0]

        snapshot_id = None
        if NAME == "init":
            cloud = CLOUDS[cloud_name]

            service_name = get_service_name_by_num(VMNUM)
            if not service_name:
                print("msg:", "internal error: service is not open")
                return 1

            if service_name not in cloud["vulnimages"]:
                print("msg:", "internal error: no such service")
                return 1

            snapshot_id = int(cloud["vulnimages"][service_name])
        else:
            SNAPSHOT_NAME = get_snapshot_prefix(TEAM, VMNUM) + NAME
            snapshots = do_api.list_snapshots(token)

            ids = []

            for snapshot in snapshots:
                if snapshot.get("name", "") != SNAPSHOT_NAME:
                    continue

                ids.append(int(snapshot["id"]))

            if not ids:
                print("msg:", "no such snapshot")
                return 1

            if len(ids) > 1:
                print("msg:", "internal error: too much snapshots")
                return 1
            snapshot_id = ids[0]

        result = do_api.restore_vm_from_snapshot_by_id(token, vm_id, snapshot_id)

        if not result:
            log_stderr("restore shapshot failed")
            return 1


        print("msg:", "restore started, it takes couple of minutes")

        # cloud_ip = get_cloud_ip(TEAM)
        # if not cloud_ip:
        #     log_stderr("no cloud ip, exiting")
        #     return 1

        # cmd = ["sudo", "/cloud/scripts/restore_vm_from_snapshot.sh", str(TEAM), NAME]
        # ret = call_unitl_zero_exit(["ssh"] + SSH_YA_OPTS +
        #                            [cloud_ip] + cmd, redirect_out_to_err=False,
        #                            attempts=1)
        # if not ret:
        #     log_stderr("restore vm from snapshot shapshot failed")
        #     return 1
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
