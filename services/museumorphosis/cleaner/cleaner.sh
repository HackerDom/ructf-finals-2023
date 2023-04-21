#!/bin/sh

while true; do
  date -uR
  find "/database/" -type f -and -not -newermt "-7200 seconds" -delete
  sleep 60
done