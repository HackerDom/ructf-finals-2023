#!/bin/python3
import sys
import random
import string
import os
import subprocess
import json
import re


def random_flag():
  letters = string.ascii_lowercase
  return '=' + ''.join(random.choice(letters) for _ in range(32))

SERVICE_NAME = sys.argv[1]
flag_id = "flag-id"
test_flag = random_flag()

vulns = []
sploit_re = p = re.compile(fr"{SERVICE_NAME}.(\d).sploit.py")

for f in os.listdir(f"sploits/{SERVICE_NAME}"):
  m = sploit_re.match(f)
  if m:
    vulns.append(m.group(1))

for vuln in vulns:
  print(f"Checking sploit on '{vuln}' vuln")
  p = subprocess.Popen([f'./{SERVICE_NAME}.checker.py', 'PUT', '127.0.0.1', flag_id, test_flag, vuln], cwd=f"checkers/{SERVICE_NAME}", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  flag_data, _ = p.communicate()
  print(f"Putted flag '{flag_data}'")

  flag_id = json.loads(flag_data)["public_flag_id"]
  print(f"Public flag id: '{flag_id}'")
  flag_private = json.loads(str(flag_data))["private_content"]
  print(f"Private flag id: '{flag_private}'")

  p = subprocess.Popen([f'./{SERVICE_NAME}.{vuln}.sploit.py', '127.0.0.1', flag_id], cwd=f"sploits/{SERVICE_NAME}", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  text_with_flag, err = p.communicate()
  if err:
    raise Exception(f"Falied to execute sploit: err='{err}'")
  print(f"Text from sploit - '{text_with_flag}'")

  if test_flag not in str(text_with_flag):
    raise Exception("Flag not found in sploit stdout")
