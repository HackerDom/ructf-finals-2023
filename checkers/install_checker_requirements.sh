#!/bin/bash

set -ex

for f in */requirements.txt; do
    python3.11 -m pip install -r "$f";
done
