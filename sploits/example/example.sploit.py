#!/usr/bin/env python3
import json
import random
import sys
import traceback

import requests


checker = NewChecker()
PORT = 2222
HOSTNAME = sys.argv[1]
FLAG_ID = sys.argv[2]
FLAG = sys.argv[3]
HACK_URL_PATTERN = "http://{hostname}:{port}/hack?username={username}"

if __name__ == '__main__':
  flag_id_json = json.loads(FLAG_ID)

  username = flag_id_json['username']

  hack_url = HACK_URL_PATTERN.format(hostname=HOSTNAME, port=PORT, username=username)
  r = requests.get(hack_url, timeout=10)
  r.raise_for_status()
  if r.text != FLAG:
    print(f"Wrong flag: {r.text} != {FLAG}")
    exit(1)
