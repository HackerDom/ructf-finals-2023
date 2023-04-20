FROM golang:1.19-alpine AS build

WORKDIR /app

COPY go.* ./

RUN go mod download

COPY ./ ./

RUN CGO_ENABLED=0 go build -o ./main ./cmd/main.go

FROM alpine:3.17

COPY --from=build /app/main /main
COPY --from=build /app/internal/templates/* ./

ENTRYPOINT ["/main"]