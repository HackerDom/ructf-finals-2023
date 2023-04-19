FROM golang:1.20-alpine as builder
LABEL stage=builder

RUN apk add build-base # for sqlite3 build

WORKDIR /app

COPY go.mod go.sum ./
RUN go mod download

COPY ./ ./
RUN CGO_ENABLED=1 go build -v -o app cmd/app/main.go

FROM alpine:3.14

WORKDIR /app

COPY --from=builder /app/app .
COPY .env .
RUN mkdir ./jwt_secret && touch ./jwt_secret/secret
RUN mkdir /database
