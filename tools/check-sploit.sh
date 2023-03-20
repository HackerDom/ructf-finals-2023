#!/bin/bash

SERVICE_NAME=$1;
VULN_ID=1;
FLAG_ID="flag-id";
TEST_FLAG="test-flag";

FLAG_ID=(cd checkers/$SERVICE_NAME && ./$SERVICE_NAME.checker.py PUT 127.0.0.1 $FLAG_ID $TEST_FLAG $VULN_ID);
(cd sploits/$SERVICE_NAME/ && ./$SERVICE_NAME.sploit.py 127.0.0.1 $FLAG_ID test-flag);