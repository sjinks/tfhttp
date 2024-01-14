FROM ubuntu:mantic-20231128@sha256:cbc171ba52575fec0601f01abf6fdec67f8ed227658cacbc10d778ac3b218307 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Do not install libsqlite3-dev because we want to it without dynamic extension support; we need tcl to build amalgamated version
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates cmake git clang make python3-minimal python3-whichcraft nlohmann-json3-dev libev-dev pkgconf tcl && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel && \
    cmake --build build && \
    strip --strip-unneeded build/src/tfhttp

FROM scratch
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
