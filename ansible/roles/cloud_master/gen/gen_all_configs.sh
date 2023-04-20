#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"

rsync -a ../../vpn/gen/client_prod/ ./openvpn_team_main_net_client_prod/

python3 gen_team_tokens_prod.py
python3 gen_rootpasswds_prod.py
./create_team_net_certs_prod.sh
python3 gen_conf_client_entergame_prod.py
python3 gen_conf_server_outside_prod.py
python3 init_teams_prod.py

exit 0
