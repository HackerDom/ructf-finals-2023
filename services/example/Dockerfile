FROM golang:1.20-alpine

RUN adduser --system --home /app --shell /usr/sbin/nologin --no-create-home --disabled-password example
USER example

COPY --chown=example . /app
WORKDIR /app

RUN go build

ENTRYPOINT ["/app/example"]
