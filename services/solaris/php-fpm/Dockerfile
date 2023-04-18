FROM php:8-fpm-alpine

RUN apk add gmp-dev \
    && docker-php-ext-install gmp opcache

RUN mkdir -p /tmp/storage \
    && chown -R www-data /tmp/storage \
    && chmod 644 /tmp/storage
