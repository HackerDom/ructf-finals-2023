#!/usr/bin/bash

while true
do
	python3 dcs.checker.py test localhost
	if [[ "$?" != 0 ]]; then
		exit 1
	fi
done

