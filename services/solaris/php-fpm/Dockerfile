FROM php:8-fpm-alpine

RUN apk add gmp-dev \
    && docker-php-ext-install gmp opcache

RUN mkdir -p /tmp/storage \
    && mkdir -p /tmp/storage/users/ \
    && mkdir -p /tmp/storage/keys/ \
    && mkdir -p /tmp/storage/ciphertexts/ \
    && chown -R www-data /tmp/storage \
    && chmod 644 /tmp/storage
