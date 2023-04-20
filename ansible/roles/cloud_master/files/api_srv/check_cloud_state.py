#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Cloud state checker. 

Checks if the vm state in db consistent with an actual state

Recomended to be used by cloud administrators only
"""

import sys
import os
import traceback
import time
import glob
import re
import subprocess

import do_api
from do_tokens import DO_TOKENS
from do_settings import CLOUDS, DOMAIN, CLOUD_FOR_DNS

from cloud_common import get_available_services, get_service_name_by_num, get_image_name


DO_ROUTER_NAME_RE = r"team([0-9]+)-router"
DO_ROUTER_DNS_NAME_FMT = "team%d"

DO_IMAGE_NAME_RE = r"team([0-9]+)-(.+)"

# YA_INTRAOPENVPN_RE = r"client_intracloud_team([0-9]+)\.conf"
# YA_VBOXNAME_RE = r"test_team([0-9]+)"


def log_stderr(*params):
    print(*params, file=sys.stderr)


def log_team(team, *params):
    log_stderr("team %d:" % team, *params)


def get_vm_states():
    net_states = {}
    image_states = {}
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

            team_state = open("db/%s/team_state" % (filename)).read().strip()
            team_states[team] = team_state
        except FileNotFoundError: 
            log_team(team, "failed to load states")
    return net_states, image_states, team_states


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


def get_do_router_ips(cloud_names):
    team2routerip = {}

    for cloud_name in cloud_names:
        token = DO_TOKENS.get(cloud_name)
        if not token:
            log_team(team, "warning no token for cloud", cloud_name)
            continue

        do_vms = do_api.get_all_vms(token)
        for vm in do_vms:
            m = re.fullmatch(DO_ROUTER_NAME_RE, vm["name"])
            if not m:
                continue
            team = int(m.group(1))

            try:
                ip = vm["networks"]["v4"][0]["ip_address"]
                if ip.startswith("10."):
                    # take next
                    ip = vm['networks']['v4'][1]['ip_address']

                team2routerip[team] = ip
            except (KeyError, IndexError):
                log_team(team, "warning no ip in do router")
    return team2routerip


def get_do_image_local_ips_and_tags_by_cloud(cloud_names):
    cloud2team2service2ip = {}
    cloud2team2service2tag = {}

    services2num = get_available_services()

    for cloud_name in cloud_names:
        if cloud_name not in cloud2team2service2ip:
            cloud2team2service2ip[cloud_name] = {}
        if cloud_name not in cloud2team2service2tag:
            cloud2team2service2tag[cloud_name] = {}

        token = DO_TOKENS.get(cloud_name)
        if not token:
            log_team(team, "warning no token for cloud", cloud_name)
            continue

        do_vms = do_api.get_all_vms(token)
        for vm in do_vms:
            m = re.fullmatch(DO_IMAGE_NAME_RE, vm["name"])
            if not m:
                continue
            team = int(m.group(1))
            service = m.group(2).replace("-", "_")
            if service == "router":
                continue

            if team not in cloud2team2service2ip[cloud_name]:
                cloud2team2service2ip[cloud_name][team] = {}
            if team not in cloud2team2service2tag[cloud_name]:
                cloud2team2service2tag[cloud_name][team] = {}


            if service not in services2num:
                log_team(team, "warning vm with unknown service exists on cloud %s: %s" %
                         (cloud_name, vm["name"]))
                continue

            service_num = services2num[service]

            if service not in cloud2team2service2ip[cloud_name][team]:
                cloud2team2service2ip[cloud_name][team][service_num] = None
            if service not in cloud2team2service2tag[cloud_name][team]:
                cloud2team2service2tag[cloud_name][team][service_num] = None

            try:
                ip = vm["networks"]["v4"][1]["ip_address"]
                if not ip.startswith("10.6"):
                    # take next
                    ip = vm['networks']['v4'][2]['ip_address']

                last_octet = int(ip.split(".")[-1])
                if last_octet != service_num:
                    log_team(team, "warning bad ip address for", vm["name"],
                             "service num", service_num, "ip", ip)

                cloud2team2service2ip[cloud_name][team][service_num] = ip
            except (KeyError, IndexError):
                log_team(team, "warning no local ip for image", vm["name"])

            try:
                tags = vm["tags"]
                if len(tags) != 1:
                    log_team(team, "warning unexpected tags:", tags, "for image", vm["name"])
                else:
                    tag = tags[0]
                    cloud2team2service2tag[cloud_name][team][service_num] = tag
            except (KeyError, IndexError):
                log_team(team, "warning no tags for image", vm["name"])

    return cloud2team2service2ip, cloud2team2service2tag


def check_do_domains(records, team, expected_ip):
    success = True
    team_record_name = DO_ROUTER_DNS_NAME_FMT % team
    
    found = 0
    for record in records:
        if not record["name"] == team_record_name:
            continue
        found += 1
        if record["type"] != "A":
            log_team(team, "unexpected dns record type")
            success = False
        if record["data"] != expected_ip:
            log_team(team, "unexpected dns record ip: found %s, expected %s" % (record["data"], expected_ip))
            success = False

    if found == 0:
        log_team(team, "no dns record found, expected %s" % expected_ip)
        success = False
    elif found == 2:
        log_team(team, "multiple dns records found")
        success = False
    return success



def main():
    net_states, image_states, team_states = get_vm_states()
    team2cloud_name = get_cloud_names()

    assert net_states.keys() == image_states.keys() == team_states.keys()
    teams = list(net_states.keys())

    team2routerip = get_do_router_ips(list(set(team2cloud_name.values())))

    cloud2team2service2ip, cloud2team2service2tag = (
        get_do_image_local_ips_and_tags_by_cloud(list(set(team2cloud_name.values()))))

    # print(cloud2team2service2tag)

    dns_token = DO_TOKENS[CLOUD_FOR_DNS]

    do_dns_records = do_api.get_all_domain_records(dns_token, DOMAIN)

    visible_services = set(get_available_services(only="visible").values())
    invisible_services = set(get_available_services(only="invisible").values())

    for team in teams:
        if net_states[team] in ["DO_LAUNCHED", "DNS_REGISTERED", "DO_DEPLOYED"]:
            log_team(team, "intermediate state: %s" % net_states[team])

        if net_states[team] == "NOT_STARTED":
            if team in team2routerip:
                log_team(team, "net state is NOT_STARTED, but router vm is created")
        
        if net_states[team] in ["DO_LAUNCHED", "DNS_REGISTERED", "DO_DEPLOYED", "READY"]:
            if team not in team2routerip:
                log_team(team, "net state is %s, but router vm has no ip" % net_states[team])
            else:
                if net_states[team] in ["DNS_REGISTERED", "DO_DEPLOYED", "READY"]:
                    ip = team2routerip[team]
                    check_do_domains(do_dns_records, team, ip)

        if net_states[team] == "READY":
            if team not in team2routerip:
                log_team(team, "net state is READY, but vm has no router ip")


        for service_num in image_states[team]:
            service_state = image_states[team][service_num]

            if service_state == "NOT_STARTED":
                for cloud, team2service2ip in cloud2team2service2ip.items():
                    if service_num in team2service2ip.get(team, {}):
                        log_team(team, "service %s state is NOT_STARTED, but vm is up on cloud %s" %
                            (service_num, cloud))

            if service_state == "RUNNING":
                if team not in team2cloud_name:
                    log_team(team, "service", service_num, "is running, but no cloud defined, skipping checks")
                    break

                team_cloud_name = team2cloud_name[team]
                for cloud, team2service2ip in cloud2team2service2ip.items():
                    if cloud == team_cloud_name and service_num not in team2service2ip.get(team, {}):
                        log_team(team, "service", service_num,
                                 "is in RUNNING state, but no vm exists on cloud", cloud)
                    if cloud != team_cloud_name and service_num in team2service2ip.get(team, {}):
                        log_team(team, "service", service_num,
                                 "is in RUNNING state, but it also exists on cloud", cloud)

                for cloud, team2service2tag in cloud2team2service2tag.items():
                    tag = team2service2tag.get(team, {}).get(service_num, "")
                    if service_num in invisible_services and tag != "team-image-closed":
                        log_team(team, "service", service_num, "with tag", tag,
                                 "is invisible to teams, but unfirewalled on cloud", cloud)
                    if service_num in visible_services and tag != "team-image":
                        log_team(team, "service", service_num, "with tag", tag,
                                 "is visible to teams, but firewalled on cloud", cloud)
                    if service_num not in visible_services and service_num not in invisible_services:
                        log_team(team, "service", service_num, "with tag", tag,
                                 "is not visible nor invisible on cloud", cloud)




        if team_states[team] == "CLOUD" and net_states[team] == "NOT_STARTED":
            log_team(team, "team state is CLOUD, but net state is NOT_STARTED")

        if team_states[team] == "CLOUD" and image_states[team] == "NOT_STARTED":
            log_team(team, "team state is CLOUD, but image state is NOT_STARTED")

    for cloud in set(team2cloud_name.values()):
        token = DO_TOKENS[cloud]
        print("Ratelimit %s %s" % (cloud, do_api.get_rate_limit(token)))
            
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
