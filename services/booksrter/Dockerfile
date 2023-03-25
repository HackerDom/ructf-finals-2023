FROM python:3.10-slim


WORKDIR /service
EXPOSE 8000

RUN apt update -y && \
	apt install -y --no-install-recommends wget xz-utils && \
	cd /opt && \
	wget https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz && \
	tar xvf ffmpeg-release-amd64-static.tar.xz && \
	mv ffmpeg-*-static ffmpeg && mkdir -p /vault/books && \
    mkdir -p /service/media/images && \
    mkdir -p /service/media/videos && \
	cd /service

COPY requirements.txt ./requirements.txt

RUN pip install -r ./requirements.txt

COPY service /service

RUN adduser --disabled-password service-user

RUN chown -R service-user:service-user /service && chown -R service-user:service-user /vault/books

USER service-user