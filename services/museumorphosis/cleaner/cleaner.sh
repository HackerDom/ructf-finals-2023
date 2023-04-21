#!/bin/sh

while true; do
  date -uR
  find "/database/" -type f -and -not -newermt "-3600 seconds" -delete
  sleep 60
done