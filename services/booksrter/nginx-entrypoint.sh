#!/usr/bin/env sh
set -eu

envsubst '${SERVER_PORT}' < /etc/nginx/conf.d/nginx.conf.template > /etc/nginx/conf.d/nginx.conf

exec "$@"