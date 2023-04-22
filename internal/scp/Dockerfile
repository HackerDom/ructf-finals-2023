FROM ubuntu:latest
COPY /templates /templates
COPY /static /static
COPY /museum.scp museum.scp
ENTRYPOINT ["/museum.scp", "-Xms128m", "-Xmx1G"]
