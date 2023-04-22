FROM ghcr.io/graalvm/native-image:22.3.1

RUN mkdir /usr/app
COPY reflectconfig.json /usr/app/reflectconfig.json
COPY proxy.json /usr/app/proxy.json
COPY free_interpretation-1.0-jar-with-dependencies.jar /usr/app/free_interpretation-1.0-jar-with-dependencies.jar
WORKDIR /usr/app
RUN ["native-image", "-jar", "free_interpretation-1.0-jar-with-dependencies.jar", "--target=linux-amd64"]
