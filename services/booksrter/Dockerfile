FROM python:3.10-slim


WORKDIR /service
EXPOSE 8000

RUN apt update -y && \
	apt install -y --no-install-recommends wget xz-utils libfuse2 && \
    mkdir -p /vault/books && \
    mkdir -p /service/media/images && \
    mkdir -p /service/db && \
    touch /service/db/db.sqlite3 && \
    mkdir -p /service/media/videos && cd /opt && \
	wget https://storage.yandexcloud.net/ffmpegstore/ffmpegv2.tar.gz && \
	tar xvf ffmpegv2.tar.gz && \
	mv ffmpegr ffmpeg && \
	cd /service

COPY requirements.txt ./requirements.txt

RUN pip install -r ./requirements.txt

COPY service /service

