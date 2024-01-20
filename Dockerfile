FROM ubuntu:mantic-20231128@sha256:cbc171ba52575fec0601f01abf6fdec67f8ed227658cacbc10d778ac3b218307 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Do not install libsqlite3-dev because we want to it without dynamic extension support. It requires tcl.
# Do not install libev-dev because we want to use a custom build.
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates cmake file git clang make nlohmann-json3-dev libtls-dev libssl-dev pkgconf tcl && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_BINARY=off -DBUILD_MOSTLY_STATIC_BINARY=on -DFORCE_EXTERNAL_LIBEV=on && \
    cmake --build build && \
    strip --strip-unneeded build/src/tfhttp

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:6895a08124484dcab1daa40861ebe814290e0a48aab71cdd3eccc75de0c045ce
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
