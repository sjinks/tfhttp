FROM ubuntu:24.04@sha256:2e863c44b718727c860746568e1d54afd13b2fa71b160f5cd9058fc436217b30 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Do not install libev-dev because we need libev to be built with -fPIC.
# Do not install libsqlite3-dev because we want to it without dynamic extension support. It requires tcl.
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates cmake file git clang make nlohmann-json3-dev pkgconf && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_BINARY=off -DBUILD_MOSTLY_STATIC_BINARY=on && \
    cmake --build build && \
    strip --strip-unneeded build/src/tfhttp

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:7cbd1cd7a6a1ca37de8d242ade7e65253323d1991611ebcb20b15347a95b8ebf
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
