FROM alpine:latest

RUN apk add findutils

COPY cleaner.sh /var/cleaner.sh

RUN chmod +x /var/cleaner.sh

ENTRYPOINT ["/var/cleaner.sh"]
