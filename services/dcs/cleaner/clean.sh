#!/bin/sh

while true; do
    date -uR
    find "/var/dcs/data/" \
        -type f \
        -and -not -newermt "-7200 seconds" \
        -delete
    sleep 30
done

