#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Shows all team instances. Doesn't show teams, who doesn't create an
Instance.

Recomended to be used by cloud administrators only
"""

import sys
import os
import traceback
import time
import re

from cloud_common import get_available_services, get_service_name_by_num, get_image_name

def log_stderr(*params):
    print(*params, file=sys.stderr)


def log_team(team, *params):
    log_stderr("team %d:" % team, *params)


def get_vm_states():
    net_states = {}
    image_states = {}
    open_states = {}
    team_states = {}

    for filename in os.listdir("db"):
        m = re.fullmatch(r"team([0-9]+)", filename)
        if not m:
            continue
        team = int(m.group(1))
        try:
            net_state = open("db/%s/net_deploy_state" % (filename)).read().strip()
            net_states[team] = net_state
            
            try:
                for vm, vmnum in get_available_services().items():
                    image_state = open("db/%s/serv%d_image_deploy_state" %
                                       (filename, vmnum)).read().strip()
                    if team not in image_states:
                        image_states[team] = {}
                    image_states[team][vmnum] = image_state
            except FileNotFoundError:
                log_team(team, "failed to load states")

            try:
                for vm, vmnum in get_available_services().items():
                    open_state = open("db/%s/serv%d_open_state" %
                                       (filename, vmnum)).read().strip()
                    if team not in open_states:
                        open_states[team] = {}
                    open_states[team][vmnum] = open_state
            except FileNotFoundError:
                log_team(team, "failed to load open states")

            team_state = open("db/%s/team_state" % (filename)).read().strip()
            team_states[team] = team_state
        except FileNotFoundError: 
            log_team(team, "failed to load states")
    return net_states, image_states, open_states, team_states


def get_cloud_names():
    cloud_names = {}

    for filename in os.listdir("db"):
        m = re.fullmatch(r"team([0-9]+)", filename)
        if not m:
            continue
        team = int(m.group(1))

        try:
            cloud_name = open("db/%s/cloud_name" % filename).read().strip()
            cloud_names[team] = cloud_name

        except FileNotFoundError:
            # it is ok, for undeployed VMs
            pass
    return cloud_names




def main():
    net_states, image_states, open_states, team_states = get_vm_states()
    cloud_names = get_cloud_names()

    assert net_states.keys() == image_states.keys() == open_states.keys() == team_states.keys()
    teams = list(net_states.keys())

    service_ids = sorted(list({s for k in image_states.values() for s in k}))

    print("%4s %7s %16s %16s %16s %16s %s" % (
        "TEAM", "SERVICE", "SERVICE_NAME", "NET_STATE", "IMAGE_STATE", "TEAM_STATE", "CLOUD"))
    for service_id in service_ids:
        for team in sorted(teams):
            service_state = image_states.get(team, {}).get(service_id, "")
            if net_states[team] == "NOT_STARTED" and service_state == "NOT_STARTED":
                continue
            service_name = get_service_name_by_num(service_id)
            team_state = team_states[team]
            if open_states.get(team, {}).get(service_id, "") == "CLOSED":
                team_state = "ISOLATED"
            print("%4d %7d %16s %16s %16s %16s %s" % (
                team, service_id, service_name, net_states[team],
                service_state, team_state, cloud_names.get(team, "NO")))
            
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
