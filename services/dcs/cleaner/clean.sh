#!/bin/sh

while true; do
    date -uR
    find "/var/dcs/data/" \
        -type f \
        -and -not -newermt "-1800 seconds" \
        -delete
    sleep 30
done

