#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Disconnects vm network from the game network"""

import sys
import time
import os
import traceback

import do_api
from do_tokens import DO_TOKENS
from cloud_common import (log_progress, call_unitl_zero_exit, get_cloud_name,
                          SSH_OPTS, SSH_DO_OPTS, # SSH_YA_OPTS, ROUTER_HOST
                          )

TEAM = int(sys.argv[1])
ROUTER_VM_NAME = "team%d-router" % TEAM

def log_stderr(*params):
    print("Team %d:" % TEAM, *params, file=sys.stderr)


def main():
    net_state = open("db/team%d/net_deploy_state" % TEAM).read().strip()

    droplet_id = None
    if net_state != "READY":
        log_stderr("the network state should be READY")
        return 1

    team_state = open("db/team%d/team_state" % TEAM).read().strip()

    ip = None

    if team_state == "CLOUD":
        cloud_name = get_cloud_name(TEAM)
        if not cloud_name:
            log_stderr("no cloud_name, exiting")
            return 1
        token = DO_TOKENS[cloud_name]

        ip = do_api.get_ip_by_vmname(token, ROUTER_VM_NAME)
        if ip is None:
            log_stderr("no ip, exiting")
            return 1

        cmd = ["systemctl stop openvpn@game_network_team%d" % TEAM]
        ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        if not ret:
            log_stderr("stop main game net tun")
            return 1

        team_state = "MIDDLE_STATE"
        open("db/team%d/team_state" % TEAM, "w").write(team_state)

    if team_state == "MIDDLE_STATE":
        # if ip is None:
        #     ip = do_api.get_ip_by_vmname(VM_NAME)
        #     if ip is None:
        #         log_stderr("no ip, exiting")
        #         return 1

        # cmd = ["sudo", "/root/cloud/switch_team_to_not_cloud.sh",
        #        str(TEAM), ip]
        # ret = call_unitl_zero_exit(cmd)
        # if not ret:
        #     log_stderr("switch_team_to_not_cloud")
        #     return 1

        team_state = "NOT_CLOUD"
        open("db/team%d/team_state" % TEAM, "w").write(team_state)

    if team_state == "NOT_CLOUD":
        print("msg: OK, your network have been disconnected from a game network")
        return 0
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
