#!/bin/bash


SERVICE_NAME=$1;
VULN_ID=1;
FLAG_ID="flag-id";
TEST_FLAG=$(echo $RANDOM | md5sum | head -c 20;);

FLAG_DATA=$(cd checkers/$SERVICE_NAME && ./$SERVICE_NAME.checker.py PUT 127.0.0.1 "$FLAG_ID" "$TEST_FLAG" $VULN_ID);
echo "Putted flag '$FLAG_DATA'"
FLAG_ID=$(echo $FLAG_DATA | jq -r .public_flag_id)
echo "Public flag id: '$FLAG_ID'"
FLAG_ACTUAL=$(echo $FLAG_DATA | jq -r .private_content | jq -r .flag)

set -e

FLAG_EXPECTED=$(cd sploits/$SERVICE_NAME/ && ./$SERVICE_NAME.sploit.py 127.0.0.1 "$FLAG_ID");

echo "Flag expected '$FLAG_EXPECTED' actual '$FLAG_ACTUAL'"

if [ "$FLAG_EXPECTED" != "$FLAG_ACTUAL" ]; then
  echo "Invalid flag"
  exit 1
fi