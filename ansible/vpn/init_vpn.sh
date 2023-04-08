#!/bin/bash -e

cd "$(dirname "$0")"

if [[ $# != 2 ]]; then
    echo "Usage: init.sh <vpn_ip> <domain>"
    exit 1
fi

IP="$1"
DOMAIN="$2"

if [[ ! $IP =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Bad vpn_ip"
  exit 1
fi

if [[ ! $DOMAIN =~ ^[0-9a-z.-]+$ ]]; then
  echo "Bad domain"
  exit 1
fi

if ! which openvpn > /dev/null; then
  echo "It seems you don't have OpenVPN installed, please install it, it is needed for config generation"
  echo "Try 'apt install openvpn' or similar command"
  exit 1
fi

if ! which rsync > /dev/null; then
  echo "It seems you don't have rsync installed, please install it, it is needed for copying files"
  echo "Try 'apt install rsync' or similar command"
  exit 1
fi

if ! which ansible-playbook > /dev/null; then
  echo "It seems you don't have ansible installed, please install it, it is needed for deploying"
  echo "Try 'apt install ansible' or similar command"
  exit 1
fi

echo "Patching inventory.cfg"
sed -E -i "s/vpn\.a ansible_host=\S+/vpn.a ansible_host=$IP/" inventory.cfg

echo "Patching gen/gen_conf_client_prod.py"
sed -E -i "s/SERVER = \"[0-9a-z.-]+\"/SERVER = \"game.${DOMAIN}\"/" gen/gen_conf_client_prod.py

echo "Generating VPN configs"
gen/gen_all_keys.sh

echo "Copying server VPN configs from gen/server_prod/ to vpn/files/openvpn_prod"
rsync -a gen/server_prod/ vpn/files/openvpn_prod/

echo "Everyting is ready for 'ansible-playbook vpn.yaml'"