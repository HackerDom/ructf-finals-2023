FROM php:8-fpm-alpine

RUN apk add gmp-dev \
    && docker-php-ext-install gmp opcache

RUN mkdir -p /tmp/storage \
    && chown www-data:www-data /tmp/storage \
    && chmod 755 /tmp/storage
