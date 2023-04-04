FROM php:8-fpm-alpine

RUN apk add gmp-dev \
    && docker-php-ext-install gmp opcache
