FROM php:8-fpm-alpine

RUN apk add gmp-dev \
    && docker-php-ext-install gmp opcache

RUN mkdir -p /tmp/storage \
    && mkdir -p /tmp/storage/ciphertexts \
    && mkdir -p /tmp/storage/keys \
    && mkdir -p /tmp/storage/users \
    && chown -R www-data:www-data /tmp/storage \
    && chmod -R 755 /tmp/storage
