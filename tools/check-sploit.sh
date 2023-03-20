#!/bin/bash

SERVICE_NAME=$1;
VULN_ID=1;
FLAG_ID="flag-id";
TEST_FLAG="test-flag";

FLAG_DATA=$(cd checkers/$SERVICE_NAME && ./$SERVICE_NAME.checker.py PUT 127.0.0.1 "$FLAG_ID" "$TEST_FLAG" $VULN_ID);
echo "Putted flag '$FLAG_DATA'"
FLAG_ID=$(FLAG_DATA | jq .public_flag_id)
echo "Public flag id: '$FLAG_ID'"
(cd sploits/$SERVICE_NAME/ && ./$SERVICE_NAME.sploit.py 127.0.0.1 "$FLAG_ID" "$TEST_FLAG");