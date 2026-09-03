FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential ca-certificates git make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
ENTRYPOINT ["make"]