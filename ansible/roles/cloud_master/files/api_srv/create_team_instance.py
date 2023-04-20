#!/usr/bin/python3
# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Creates vm instance for a team"""

import sys
import json
import time
import os
import traceback

import do_api
from do_settings import CLOUD_FOR_NEW_VMS, CLOUDS, CLOUD_FOR_DNS, DOMAIN
from do_tokens import DO_TOKENS
from cloud_common import ( get_cloud_name, put_cloud_name, get_cloud_name,
                          log_progress, get_service_name_by_num, get_image_name,
                          call_unitl_zero_exit,SSH_OPTS, SSH_DO_OPTS,
                          SSH_YA_OPTS)

TEAM = int(sys.argv[1])
VMNUM = int(sys.argv[2])

ROUTER_VM_NAME = "team%d-router" % TEAM
DNS_NAME = "team%d" % TEAM


def log_stderr(*params):
    print("Team %d:" % TEAM, *params, file=sys.stderr)


USERDATA_TEMPLATE = """#!/bin/bash

usermod -p '{0}' root

echo 'network:
    version: 2
    ethernets:
        eth1:
            routes:
                - to: 10.60.0.0/14
                  via: {1}
                - to: 10.80.0.0/14
                  via: {1}
                - to: 10.10.10.0/24
                  via: {1}
' > /etc/netplan/60-ctf.yaml

netplan apply
"""


def main():
    net_state = open("db/team%d/net_deploy_state" % TEAM).read().strip()

    cloud_name = get_cloud_name(TEAM)
    if not cloud_name:
        cloud_name = CLOUD_FOR_NEW_VMS
        put_cloud_name(TEAM, cloud_name)

    dns_token = DO_TOKENS[CLOUD_FOR_DNS]
    token = DO_TOKENS[cloud_name]
    do_cloud_params = CLOUDS[cloud_name]

    log_progress("0%")
    droplet_id = None

    service_name = get_service_name_by_num(VMNUM)
    if not service_name:
        log_stderr("no such service, please edit services.txt")
        exit(1)

    if service_name not in do_cloud_params["vulnimages"]:
        log_stderr("no such service in cloud settings, please edit do_settings.py")
        exit(1)


    if net_state == "NOT_STARTED":
        vpc_id = do_api.get_vpc_by_name(token, "team%d" % TEAM)
        print("vpc_id", vpc_id)

        if vpc_id is None:
            team_network = "%d.%d.%d.%d/24" % (10, 60 + TEAM//256, TEAM%256, 0)
            vpc_id = do_api.create_vpc(token, "team%d" % TEAM, team_network)

        if vpc_id is None:
            log_stderr("no vpc id, exiting")
            exit(1)

        exists = do_api.check_vm_exists(token, ROUTER_VM_NAME)
        if exists is None:
            log_stderr("failed to determine if vm exists, exiting")
            return 1

        log_progress("5%")

        if not exists:
            droplet_id = do_api.create_vm(token,
                ROUTER_VM_NAME, image=do_cloud_params["router_image"],
                ssh_keys=do_cloud_params["router_ssh_keys"],
                size=do_cloud_params["sizes"].get("router", do_cloud_params["sizes"]["default"]),
                region=do_cloud_params.get("region"),
                vpc_uuid=vpc_id, tag="team-router")
            if droplet_id is None:
                log_stderr("failed to create vm, exiting")
                return 1

        net_state = "DO_LAUNCHED"
        open("db/team%d/net_deploy_state" % TEAM, "w").write(net_state)
        time.sleep(1)  # this allows to make less requests (there is a limit)

    log_progress("10%")
    ip = None
    if net_state == "DO_LAUNCHED":
        if not droplet_id:
            ip = do_api.get_ip_by_vmname(token, ROUTER_VM_NAME)
        else:
            ip = do_api.get_ip_by_id(token, droplet_id)

        if ip is None:
            log_stderr("no ip, exiting")
            return 1

        log_progress("15%")

        domain_ids = do_api.get_domain_ids_by_hostname(dns_token, DNS_NAME, DOMAIN)
        if domain_ids is None:
            log_stderr("failed to check if dns exists, exiting")
            return 1

        if domain_ids:
            for domain_id in domain_ids:
                do_api.delete_domain_record(dns_token, domain_id, DOMAIN)

        log_progress("17%")

        if do_api.create_domain_record(dns_token, DNS_NAME, ip, DOMAIN):
            net_state = "DNS_REGISTERED"
            open("db/team%d/net_deploy_state" % TEAM, "w").write(net_state)
        else:
            log_stderr("failed to create vm: dns register error")
            return 1

        for i in range(20, 50):
            # just spinning for the sake of smooth progress
            log_progress("%d%%" % i)
            time.sleep(1)

    log_progress("50%")

    if net_state == "DNS_REGISTERED":
        if ip is None:
            ip = do_api.get_ip_by_vmname(token, ROUTER_VM_NAME)

            if ip is None:
                log_stderr("no ip, exiting")
                return 1

        log_progress("55%")

        file_from = "db/team%d/server_outside.conf" % TEAM
        file_to = "%s:/etc/openvpn/server_outside_team%d.conf" % (ip, TEAM)
        ret = call_unitl_zero_exit(["scp"] + SSH_DO_OPTS +
                                   [file_from, file_to])
        if not ret:
            log_stderr("scp to DO failed")
            return 1

        log_progress("56%")

        #file_from = "db/team%d/server_wg.conf" % TEAM
        #file_to = "%s:/etc/wireguard/team%d.conf" % (ip, TEAM)
        #ret = call_unitl_zero_exit(["scp"] + SSH_DO_OPTS +
        #                           [file_from, file_to])
        #if not ret:
        #    log_stderr("scp wg to DO failed")
        #    return 1

        log_progress("57%")

        file_from = "db/team%d/game_network.conf" % TEAM
        file_to = "%s:/etc/openvpn/game_network_team%d.conf" % (ip, TEAM)
        ret = call_unitl_zero_exit(["scp"] + SSH_DO_OPTS +
                                   [file_from, file_to])
        if not ret:
            log_stderr("scp to DO failed")
            return 1

        log_progress("58%")

        cmd = ["systemctl start openvpn@server_outside_team%d" % TEAM]
        ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        if not ret:
            log_stderr("start internal tun")
            return 1


        log_progress("59%")

        #cmd = ["systemctl start wg-quick@team%d" % TEAM]
        #ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        #if not ret:
        #    log_stderr("start internal wg tun")
        #    return 1

        # UNCOMMENT BEFORE THE GAME
        #dest = "10.%d.%d.3" % (60 + TEAM//256, TEAM%256)
        #cmd = ["iptables -t nat -A PREROUTING -d %s -p tcp " % ip +
        #       "--dport 22 -j DNAT --to-destination %s:22" % dest]
        #ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        #if not ret:
        #   log_stderr("unable to nat port 22")
        #   return 1

        log_progress("61%")

        cmd = ["iptables -t nat -A POSTROUTING -o eth1 -p tcp " +
               "-m tcp --dport 22 -j MASQUERADE"]
        ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        if not ret:
           log_stderr("unable to masquerade port 22")
           return 1

        log_progress("62%")

        net_state = "READY"
        open("db/team%d/net_deploy_state" % TEAM, "w").write(net_state)


        cmd = ["systemctl start openvpn@game_network_team%d" % TEAM]
        ret = call_unitl_zero_exit(["ssh"] + SSH_DO_OPTS + [ip] + cmd)
        if not ret:
            log_stderr("start main game net tun")
            return 1

        team_state = "CLOUD"
        open("db/team%d/team_state" % TEAM, "w").write(team_state)


    log_progress("65%")

    image_state = open("db/team%d/serv%d_image_deploy_state" % (TEAM, VMNUM)).read().strip()

    log_progress("67%")

    if net_state == "READY":
        if image_state == "NOT_STARTED":
            pass_hash = open("db/team%d/serv%d_root_passwd_hash.txt" % (TEAM, VMNUM)).read().strip()
            team_network = "%d.%d.%d.%d/24" % (10, 60 + TEAM//256, TEAM%256, 0)
            team_router = "%d.%d.%d.%d" % (10, 60 + TEAM//256, TEAM%256, 2)

            vpc_id = do_api.get_vpc_by_name(token, "team%d" % TEAM)

            if vpc_id is None:
                vpc_id = do_api.create_vpc(token, "team%d" % TEAM, team_network)

            if vpc_id is None:
                log_stderr("no vpc id, exiting")
                exit(1)

            exists = do_api.check_vm_exists(token, get_image_name(TEAM, VMNUM))
            if exists is None:
                log_stderr("failed to determine if vm exists, exiting")
                return 1

            log_progress("69%")

            if not exists:
                userdata = USERDATA_TEMPLATE.format(pass_hash, team_router)

                vulnimage_droplet_id = do_api.create_vm(token,
                    get_image_name(TEAM, VMNUM), image=do_cloud_params["vulnimages"][service_name],
                    ssh_keys=do_cloud_params["vulnimage_ssh_keys"],
                    size=do_cloud_params["sizes"].get(service_name, do_cloud_params["sizes"]["default"]),
                    region=do_cloud_params.get("region"),
                    user_data=userdata, vpc_uuid=vpc_id, tag="team-image-closed")
                if vulnimage_droplet_id is None:
                    log_stderr("failed to create vm, exiting")
                    return 1

                for i in range(70, 100):
                    # just spinning for the sake of smooth progress
                    log_progress("%d%%" % i)
                    time.sleep(3)

            image_state = "RUNNING"
            open("db/team%d/serv%d_image_deploy_state" % (TEAM, VMNUM), "w").write(image_state)
    
    log_progress("100%")
    return 0


if __name__ == "__main__":
    sys.stdout = os.fdopen(1, 'w', 1)
    print("started: %d" % time.time())
    exitcode = 1
    try:
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        exitcode = main()

        net_state = open("db/team%d/net_deploy_state" % TEAM).read().strip()
        image_state = open("db/team%d/serv%d_image_deploy_state" % (TEAM, VMNUM)).read().strip()

        log_stderr("NET_STATE:", net_state)
        log_stderr("IMAGE_STATE:", image_state)

        if net_state != "READY":
            print("msg: ERR, failed to set up the network")
        elif image_state != "RUNNING":
            print("msg: ERR, failed to start up the vm")
    except:
        traceback.print_exc()
    print("exit_code: %d" % exitcode)
    print("finished: %d" % time.time())
