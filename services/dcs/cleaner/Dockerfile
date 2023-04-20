FROM alpine:latest

RUN apk add findutils
RUN adduser -H -s /bin/false -u 1000 -D clean
COPY clean.sh /clean.sh
RUN chmod +x /clean.sh
USER clean
ENTRYPOINT ["/clean.sh"]

