#!/bin/bash

cd "$(dirname "$0")"

python3 gen_keys_prod.py
python3 gen_conf_server_prod.py
python3 gen_conf_client_prod.py

exit 0
