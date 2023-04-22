bld:
	docker rm -f museum && \
	docker rm -f museum_img && \
	docker build . -t museum_img && \
	docker create --name museum museum_img && \
	docker run museum_img && \
    docker cp museum:/usr/app/museum.scp ../
