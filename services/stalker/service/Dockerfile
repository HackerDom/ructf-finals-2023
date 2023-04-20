FROM oven/bun:0.5 AS builder

RUN mkdir -p /tmp/build

WORKDIR /tmp/build

COPY package.json tsconfig.json .eslintrc.yml ./

RUN bun install --no-cache

COPY src ./src

FROM oven/bun:0.5

RUN mkdir -p /tmp/secrets \
    && chmod 755 /tmp/secrets \
    && chown nobody:nogroup /tmp/secrets

WORKDIR /var/service

COPY --from=builder /tmp/build .

USER nobody

ENTRYPOINT [ "bun", "start" ]
