FROM julia:1.8.5-alpine3.17

COPY . /app

WORKDIR /app

ENV ENV=PROD

RUN julia install.jl

WORKDIR /app/src

ENTRYPOINT ["/usr/local/julia/bin/julia", "Program.jl"]

