import json
import os

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
with open(os.path.join(SCRIPT_DIR, "do_vulnimages.json")) as vulnimages_file:
    vulnimages = json.load(vulnimages_file)

CLOUD_FOR_NEW_VMS = "ructf"
CLOUD_FOR_DNS = "ructf"
DOMAIN = "cloud.ructf.org"

CLOUDS = {
    "ructf": {
        "region": "ams3",
        "router_image": 130314190,
        "router_ssh_keys": [37986675, 37987387],
        "vulnimages": vulnimages,
        "vulnimage_ssh_keys": [37986675],
        "sizes": {
            "default": "s-4vcpu-8gb",
            "router": "s-2vcpu-2gb",
            "empty": "s-8vcpu-16gb",
        }
    },
}

DO_SSH_ID_FILE = "do_deploy_key"

