# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Common functions and consts that are often used by other scripts in 
this directory"""

import subprocess
import sys
import time
import os
import shutil
import re

from do_settings import DO_SSH_ID_FILE

SSH_OPTS = [
    "-o", "StrictHostKeyChecking=no",
    "-o", "CheckHostIP=no",
    "-o", "NoHostAuthenticationForLocalhost=yes",
    "-o", "BatchMode=yes",
    "-o", "LogLevel=ERROR",
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "ConnectTimeout=10"
]

SSH_DO_OPTS = SSH_OPTS + [
    "-o", "Port=2222",
    "-o", "User=root",
    "-o", "IdentityFile=%s" % DO_SSH_ID_FILE
]

SSH_YA_OPTS = SSH_OPTS + [
    "-o", "User=cloud",
    #"-o", "IdentityFile=ructfe2020_ya_deploy"
]

# def untake_cloud_ip(team):
#     for slot in os.listdir("db/team%d" % team):
#         if not slot.startswith("slot_"):
#             continue
#         os.rename("db/team%d/%s" % (team, slot), "slots/%s" % slot)
    
#     try:
#         os.remove("db/team%d/cloud_ip" % team)
#     except FileNotFoundError:
#         return

def get_cloud_name(team):
    try:
        return open("db/team%d/cloud_name" % team).read().strip()
    except FileNotFoundError as e:
        return None


def put_cloud_name(team, cloud_name):
    open("db/team%d/cloud_name" % team, "w").write(cloud_name)


# def unput_cloud_name(team):
#     os.remove("db/team%d/cloud_name" % team)


# def take_cloud_ip(team):
#     for slot in os.listdir("db/team%d" % team):
#         if not slot.startswith("slot_"):
#             continue
#         shutil.copy2("db/team%d/%s" % (team, slot), "db/team%d/cloud_ip" % team)
#         cloud_ip = open("db/team%d/cloud_ip" % team).read().strip()
#         print("Retaking slot %s, taken %s" % (slot, cloud_ip), file=sys.stderr)
#         return cloud_ip

#     for slot in os.listdir("slots"):
#         try:
#             os.rename("slots/%s" % slot, "db/team%d/%s" % (team, slot))
#         except FileNotFoundError:
#             continue

#         shutil.copy2("db/team%d/%s" % (team,slot), "db/team%d/cloud_ip" % team)
#         cloud_ip = open("db/team%d/cloud_ip" % team).read().strip()
#         print("Taking slot %s, taken %s" % (slot, cloud_ip), file=sys.stderr)
#         return cloud_ip
#     return None




def log_progress(*params):
    print("progress:", *params, flush=True)


def call_unitl_zero_exit(params, redirect_out_to_err=True, attempts=60, timeout=10):
    if redirect_out_to_err:
        stdout = sys.stderr
    else:
        stdout = sys.stdout

    for i in range(attempts-1):
        if subprocess.call(params, stdout=stdout) == 0:
            return True
        time.sleep(timeout)
    if subprocess.call(params, stdout=stdout) == 0:
        return True

    return None

def get_available_services(only=None):
    try:
        ret = {}
        for line in open("db/services.txt"):
            line = line.strip()
            m = None
            if only == "visible":
                m = re.fullmatch(r"([0-9a-zA-Z_]+)+\s+(\d+)\s*(?:\+)", line)
            elif only == "invisible":
                m = re.fullmatch(r"([0-9a-zA-Z_]+)+\s+(\d+)\s*(?:\-)", line)
            elif only is None:
                m = re.fullmatch(r"([0-9a-zA-Z_]+)\s+(\d+)\s*(?:\+|\-)", line)
            else:
                raise Exception("Internal error: bad param for get_available_services")

            if not m:
                continue
            vm, vm_number = m.groups()
            ret[vm] = int(vm_number)
        return ret
    except (OSError, ValueError):
        return {}


def get_service_name_by_num(num):
    if list(get_available_services().values()).count(num) != 1:
        return ""

    for k, v in get_available_services().items():
        if v == num:
            return k
    return ""

def get_image_name(team, num):
    # cloud_name = get_cloud_name(team)
    # if not cloud_name:
    #     raise Exception("No image name")

    service_name = get_service_name_by_num(num)
    if not service_name:
        raise Exception("No service with this num, edit services.txt")

    # the underscore is bad character for image name
    service_name = service_name.replace("_", "-")

    if not re.fullmatch(r"[a-z0-9-]+", service_name):
        raise Exception("Service has bad name")

    return "team%d-%s" % (team, service_name)

def get_snapshot_prefix(team, num):
    service_name = get_service_name_by_num(num)
    if not service_name:
        raise Exception("No service with this num, edit services.txt")

    # the hyphen is bad character for snapshot name
    service_name = service_name.replace("-", "_")

    if not re.fullmatch(r"[a-z0-9_]+", service_name):
        raise Exception("Service has bad name")

    return "team%d-%s-" % (team, service_name)