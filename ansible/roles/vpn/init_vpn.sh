#!/bin/bash -e

cd "$(dirname "$0")"

if [[ $# != 2 ]]; then
    echo "Usage: init_vpn.sh <domain> <number_of_teams>"
    exit 1
fi

DOMAIN="$1"
NUMBER_OF_TEAMS="$2"

if [[ ! $DOMAIN =~ ^[0-9a-z.-]+$ ]]; then
  echo "Bad domain"
  exit 1
fi

if [[ ! $NUMBER_OF_TEAMS =~ ^[0-9]+$ ]]; then
   echo "Bad number of teams"
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

echo "Patching gen/gen_conf_client_prod.py"
sed -E -i "s/SERVER = \"[0-9a-z.-]+\"/SERVER = \"game.${DOMAIN}\"/" gen/gen_conf_client_prod.py

echo "Patching number of teams"
sed -E -i "s/N = [0-9]+/N = ${NUMBER_OF_TEAMS}/" gen/gen_conf_server_prod.py
sed -E -i "s/N = [0-9]+/N = ${NUMBER_OF_TEAMS}/" gen/gen_conf_client_prod.py
sed -E -i "s/N = [0-9]+/N = ${NUMBER_OF_TEAMS}/" gen/gen_keys_prod.py
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/networkclosed/open_network.sh
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/networkclosed/check_network.sh
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/networkclosed/check_network.sh
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/networkclosed/close_network.sh
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/snat/check_snat_rules.sh 
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/snat/del_snat_rules.sh 
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/snat/add_snat_rules.sh
sed -E -i "s/for num in \{0\.\.[0-9]+\}/for num in \{0\.\.$((NUMBER_OF_TEAMS-1))\}/" files/snat/add_snat_rules.sh
sed -E -i "s/openvpn@\{0\.\.[0-9]+\}/openvpn@\{0\.\.$((NUMBER_OF_TEAMS-1))\}/" handlers/main.yml

echo "Generating VPN configs"
gen/gen_all_keys.sh

echo "Copying server VPN configs from gen/server_prod/ to files/openvpn_prod"
rsync -a gen/server_prod/ files/openvpn_prod/

echo "Everyting is ready for 'ansible-playbook vpn.yaml'"