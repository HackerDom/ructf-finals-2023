#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Reboots vm"""

import sys
import time
import os
import traceback

import do_api
from do_tokens import DO_TOKENS
from cloud_common import (log_progress, call_unitl_zero_exit, get_cloud_name,
                          get_image_name,
                          SSH_OPTS, # SSH_YA_OPTS
                         )

TEAM = int(sys.argv[1])
VMNUM = int(sys.argv[2])


def log_stderr(*params):
    print("Team %d:" % TEAM, *params, file=sys.stderr)


def main():
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


        result = do_api.reboot_vm_by_vmname(token, get_image_name(TEAM, VMNUM))

        if not result:
            log_stderr("failed to reboot")
            return 1

        # cloud_ip = get_cloud_ip(TEAM)
        # if not cloud_ip:

        # cmd = ["sudo", "/cloud/scripts/reboot_vm.sh", str(TEAM)]
        # ret = call_unitl_zero_exit(["ssh"] + SSH_YA_OPTS +
        #                            [cloud_ip] + cmd, redirect_out_to_err=False,
        #                            attempts=1)
        # if not ret:
        #     log_stderr("reboot vm failed")
        #     return 1
        return 0

    log_stderr("unknown state")
    return 1

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
