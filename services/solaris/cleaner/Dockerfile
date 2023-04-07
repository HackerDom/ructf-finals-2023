FROM alpine:latest

RUN apk add findutils

RUN adduser -H -s /bin/false -u 1000 -D cleaner

COPY cleaner.sh /var/cleaner.sh

RUN chmod +x /var/cleaner.sh

USER cleaner

ENTRYPOINT ["/var/cleaner.sh"]
