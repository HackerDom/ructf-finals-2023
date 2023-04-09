FROM ubuntu:22.04

RUN apt-get update -y && apt-get install binutils -y
RUN useradd --no-create-home --shell /bin/false --uid 1000 --user-group dcs
RUN mkdir -p /var/dcs/data && chown dcs:dcs /var/dcs/data
COPY dcsserver /
RUN chmod +x /dcsserver
USER dcs
ENTRYPOINT ["/dcsserver", "-w", "4"]

