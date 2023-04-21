#!/bin/sh

while true; do
    date -uR

    find "/tmp/storage/ciphertexts/" \
        -type f \
        -and -not -newermt "-7200 seconds" \
        -delete

    find "/tmp/storage/keys/" \
        -type f \
        -and -not -newermt "-7200 seconds" \
        -delete

    find "/tmp/storage/users/" \
        -type f \
        -and -not -newermt "-7200 seconds" \
        -delete

    sleep 60
done
