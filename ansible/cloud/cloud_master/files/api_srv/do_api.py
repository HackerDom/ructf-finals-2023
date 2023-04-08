# Developed by Alexander Bersenev from Hackerdom team, bay@hackerdom.ru

"""Common functions that make requests to digital ocean api"""

import requests
import time
import json
import sys

VERBOSE = True


def log(*params):
    if VERBOSE:
        print(*params, file=sys.stderr)


def call_do_api(token, method, endpoint, data=None):
    headers = {
        "Content-Type": "application/json",
        "Authorization": "Bearer %s" % token,
    }

    resp = requests.request(method, endpoint, headers=headers, data=data)
    if not str(resp.status_code).startswith("2"):
        log(resp.status_code, resp.headers, resp.text)
        raise Exception("bad status code %d" % resp.status_code)

    if not resp.text:
        return {}

    return json.loads(resp.text)




def get_all_vms(token, attempts=5, timeout=10):
    vms = {}
    url = "https://api.digitalocean.com/v2/droplets?per_page=200"

    cur_attempt = 1

    while True:
        try:
            data = call_do_api(token, "get", url)

            for droplet in data["droplets"]:
                vms[droplet["id"]] = droplet

            if ("links" in data and "pages" in data["links"] and
                                    "next" in data["links"]["pages"]):
                url = data["links"]["pages"]["next"]
            else:
                break
        except Exception as e:
            log("get_all_vms trying again %s" % (e,))
            cur_attempt += 1
            if cur_attempt > attempts:
                return None  # do not return parts of the output
            time.sleep(timeout)
    return list(vms.values())


def get_ids_by_vmname(token, vm_name):
    ids = set()

    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            ids.add(droplet['id'])
    return ids


def check_vm_exists(token, vm_name):
    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            return True
    return False


def create_vm(token, vm_name, image, ssh_keys, vpc_uuid=None,
              region="ams3", size="s-1vcpu-2gb", tag="vm",
              user_data="#!/bin/bash\n\n",
              attempts=10, timeout=20):
    for i in range(attempts):
        try:
            data = json.dumps({
                "name": vm_name,
                "region": region,
                "size": size,
                "image": image,
                "ssh_keys": ssh_keys,
                "vpc_uuid": vpc_uuid,
                "backups": False,
                "ipv6": False,
                "user_data": user_data,
                "private_networking": None,
                "volumes": None,
                "tags": [tag]  # tags are too unstable in DO
            })

            log("creating new")
            url = "https://api.digitalocean.com/v2/droplets"

            data = call_do_api(token, "post", url, data)
            droplet_id = data["droplet"]["id"]

            return droplet_id
        except Exception as e:
            log("create_vm trying again %s" % (e,))
        time.sleep(timeout)
    return None


def delete_vm_by_id(token, droplet_id, attempts=10, timeout=20):
    for i in range(attempts):
        try:
            log("deleting droplet")
            url = "https://api.digitalocean.com/v2/droplets/%d" % droplet_id
            call_do_api(token, "delete", url)
            return True
        except Exception as e:
            log("delete_vm_by_id trying again %s" % (e,))
        time.sleep(timeout)
    return False


def get_ip_by_id(token, droplet_id, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/droplets/%d" % droplet_id
            data = call_do_api(token, "get", url)

            ip = data['droplet']['networks']['v4'][0]['ip_address']

            if ip.startswith("10."):
                # take next
                ip = data['droplet']['networks']['v4'][1]['ip_address']

            return ip
        except Exception as e:
            log("get_ip_by_id trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to get ip by id")
    return None


def get_ip_by_vmname(token, vm_name):
    ids = set()

    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            ids.add(droplet['id'])

    if len(ids) > 1:
        log("warning: there are more than one droplet with name " + vm_name +
            ", using random :)")

    if not ids:
        return None

    return get_ip_by_id(token, list(ids)[0])


def reboot_vm_by_id(token, droplet_id, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/droplets/%d/actions" % droplet_id
            data = json.dumps({"type": "power_cycle"})
            call_do_api(token, "post", url, data=data)
            return True
        except Exception as e:
            log("reboot_vm_by_id trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to reboot vm by id")
    return None


def reboot_vm_by_vmname(token, vm_name):
    ids = set()

    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            ids.add(droplet['id'])

    if len(ids) > 1:
        log("warning: there are more than one droplet with name " + vm_name +
            ", using random :)")

    if not ids:
        return None
    return reboot_vm_by_id(token, list(ids)[0])


def take_vm_snapshot(token, droplet_id, snapshot_name, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/droplets/%d/actions" % droplet_id
            data = json.dumps({"type": "snapshot", "name": snapshot_name})
            call_do_api(token, "post", url, data=data)
            return True
        except Exception as e:
            log("reboot_vm_by_id trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to take shapshot of vm")
    return None


def list_snapshots(token, attempts=4, timeout=5):
    snapshots = {}
    url = ("https://api.digitalocean.com/v2/snapshots?per_page=200")

    cur_attempt = 1

    while True:
        try:
            data = call_do_api(token, "get", url) # json.loads(resp.text)
            for snapshot in data["snapshots"]:
                snapshots[snapshot["id"]] = snapshot

            if ("links" in data and "pages" in data["links"] and
                                    "next" in data["links"]["pages"]):
                url = data["links"]["pages"]["next"]
            else:
                break
        except Exception as e:
            log("list_snapshots trying again %s" % (e,))
            cur_attempt += 1
            if cur_attempt > attempts:
                return None  # do not return parts of the output
            time.sleep(timeout)

    return list(snapshots.values())


def restore_vm_from_snapshot_by_id(token, droplet_id, snapshot_id, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/droplets/%d/actions" % droplet_id
            data = json.dumps({"type": "restore", "image": snapshot_id})
            call_do_api(token, "post", url, data=data)

            return True
        except Exception as e:
            log("reboot_vm_by_id trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to reboot vm by id")
    return None



def delete_snapshot(token, snapshot_id, attempts=10, timeout=20):
    for i in range(attempts):
        try:
            log("deleting snapshot %s" % snapshot_id)
            url = ("https://api.digitalocean.com/v2/snapshots" +
                   "/%d" % snapshot_id)
            # resp = requests.delete(url, headers=HEADERS)
            # if not str(resp.status_code).startswith("2"):
            #     log(resp.status_code, resp.headers, resp.text)
            #     raise Exception("bad status code %d" % resp.status_code)
            call_do_api(token, "delete", url)
            return True
        except Exception as e:
            log("delete_snapshot trying again %s" % (e,))
        time.sleep(timeout)
    return False


def add_tag(token, droplet_id, tag, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/tags/%s/resources" % tag
            data = json.dumps({"resources":
                [{"resource_id": str(droplet_id), "resource_type": "droplet"}]})
            call_do_api(token, "post", url, data=data)

            return True
        except Exception as e:
            log("add_tag trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to add_tag by id")
    return None


def add_tag_by_vmname(token, vm_name, tag):
    ids = set()

    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            ids.add(droplet['id'])

    if len(ids) > 1:
        log("warning: there are more than one droplet with name " + vm_name +
            ", using random :)")

    if not ids:
        return None

    return add_tag(token, list(ids)[0], tag)



def remove_tag(token, droplet_id, tag, attempts=5, timeout=20):
    for i in range(attempts):
        try:
            url = "https://api.digitalocean.com/v2/tags/%s/resources" % tag
            data = json.dumps({"resources":
                [{"resource_id": str(droplet_id), "resource_type": "droplet"}]})
            call_do_api(token, "delete", url, data=data)

            return True
        except Exception as e:
            log("remove_tag trying again %s" % (e,))
        time.sleep(timeout)
    log("failed to remove_tag by id")
    return None


def remove_tag_by_vmname(token, vm_name, tag):
    ids = set()

    droplets = get_all_vms(token)
    if droplets is None:
        return None

    for droplet in droplets:
        if droplet["name"] == vm_name:
            ids.add(droplet['id'])

    if len(ids) > 1:
        log("warning: there are more than one droplet with name " + vm_name +
            ", using random :)")

    if not ids:
        return None

    return remove_tag(token, list(ids)[0], tag)


# def list_tags(token, attempts=5, timeout=20):
#     for i in range(attempts):
#         try:
#             url = "https://api.digitalocean.com/v2/tags"
#             print(call_do_api(token, "get", url))

#             return True
#         except Exception as e:
#             log("list_tags trying again %s" % (e,))
#         time.sleep(timeout)
#     log("failed to list_tags by id")
#     return None


def get_all_domain_records(token, domain, attempts=5, timeout=20):
    records = {}
    url = ("https://api.digitalocean.com/v2/domains/" + domain +
           "/records?per_page=200")

    cur_attempt = 1

    while True:
        try:
            data = call_do_api(token, "get", url)
            for record in data["domain_records"]:
                records[record["id"]] = record

            if ("links" in data and "pages" in data["links"] and
                                    "next" in data["links"]["pages"]):
                url = data["links"]["pages"]["next"]
            else:
                break
        except Exception as e:
            log("get_all_domain_records trying again %s" % (e,))
            cur_attempt += 1
            if cur_attempt > attempts:
                return None  # do not return parts of the output
            time.sleep(timeout)

    return list(records.values())


def get_domain_ids_by_hostname(token, host_name, domain, print_warning_on_fail=False):
    ids = set()

    records = get_all_domain_records(token, domain)
    if records is None:
        return None

    for record in records:
        if record["type"] == "A" and record["name"] == host_name:
            ids.add(record['id'])

    if not ids:
        if print_warning_on_fail:
            log("failed to get domain ids by hostname", host_name)

    return ids


def create_domain_record(token, name, ip, domain, attempts=10, timeout=20):
    for i in range(attempts):
        try:
            data = json.dumps({
                "type": "A",
                "name": name,
                "data": ip,
                "ttl": 30
            })
            url = "https://api.digitalocean.com/v2/domains/%s/records" % domain
            call_do_api(token, "post", url, data)
            return True
        except Exception as e:
            log("create_domain_record trying again %s" % (e,))
        time.sleep(timeout)
    return None


def delete_domain_record(token, domain_id, domain, attempts=10, timeout=20):
    for i in range(attempts):
        try:
            log("deleting domain record %d" % domain_id)
            url = ("https://api.digitalocean.com/v2/domains" +
                   "/%s/records/%d" % (domain, domain_id))
            call_do_api(token, "delete", url)
            return True
        except Exception as e:
            log("delete_domain_record trying again %s" % (e,))
        time.sleep(timeout)
    return False


def get_all_vpcs(token, attempts=5, timeout=20):
    vpcs = {}
    url = ("https://api.digitalocean.com/v2/vpcs?per_page=200")

    cur_attempt = 1

    while True:
        try:
            data = call_do_api(token, "get", url)
            for vpc in data["vpcs"]:
                vpcs[vpc["id"]] = vpc

            if ("links" in data and "pages" in data["links"] and
                                    "next" in data["links"]["pages"]):
                url = data["links"]["pages"]["next"]
            else:
                break
        except Exception as e:
            log("get_all_vpcs trying again %s" % (e,))
            cur_attempt += 1
            if cur_attempt > attempts:
                return None  # do not return parts of the output
            time.sleep(timeout)

    return vpcs


def get_vpc_by_name(token, name, print_warning_on_fail=False):
    vpcs = get_all_vpcs(token)
    if vpcs is None:
        return None

    for vpc in vpcs.values():
        if vpc["name"] == name:
            return vpc['id']

    if print_warning_on_fail:
        log("failed to get vpc ids by name", name)

    return None


def create_vpc(token, name, ip_range, region="ams3", attempts=10, timeout=20):
    for i in range(attempts):
        try:
            data = json.dumps({
                "name": name,
                "region": region,
                "ip_range": ip_range
            })
            url = "https://api.digitalocean.com/v2/vpcs"
            data = call_do_api(token, "post", url, data=data)
            return data["vpc"]["id"]
        except Exception as e:
            log("create_vpc trying again %s" % (e,))
        time.sleep(timeout)
    return None


def get_rate_limit(token):
    url = "https://api.digitalocean.com/v2/account"

    headers = {
        "Content-Type": "application/json",
        "Authorization": "Bearer %s" % token,
    }

    resp = requests.get(url, headers=headers)
    if not str(resp.status_code).startswith("2"):
        log(resp.status_code, resp.headers, resp.text)
        raise Exception("bad status code %d" % resp.status_code)

    return resp.headers.get("Ratelimit-Remaining"), resp.headers.get("Ratelimit-Reset", -1)
    

def get_ssh_keys(token, attempts=5, timeout=20):
    keys = {}
    url = ("https://api.digitalocean.com/v2/account/keys?per_page=200")

    cur_attempt = 1

    while True:
        try:
            data = call_do_api(token, "get", url)
            for ssh_key in data["ssh_keys"]:
                keys[ssh_key["id"]] = ssh_key["name"]

            if ("links" in data and "pages" in data["links"] and
                                    "next" in data["links"]["pages"]):
                url = data["links"]["pages"]["next"]
            else:
                break
        except Exception as e:
            log("get_ssh_keys trying again %s" % (e,))
            cur_attempt += 1
            if cur_attempt > attempts:
                return None  # do not return parts of the output
    return keys
