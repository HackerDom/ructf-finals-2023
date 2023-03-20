FROM golang:1.20-alpine

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN addgroup -g ${GROUP_ID} example && \
  adduser -u ${USER_ID} -g example -G example -s /usr/sbin/nologin --home /app --no-create-home --disabled-password example
USER example

COPY --chown=example . /app
WORKDIR /app

RUN go build

ENTRYPOINT ["/app/example"]
