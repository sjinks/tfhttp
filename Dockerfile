FROM ubuntu:24.04@sha256:562456a05a0dbd62a671c1854868862a4687bf979a96d48ae8e766642cd911e8 AS build

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

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:6dff3d81e2edaa0ef48ea87b808c34c4b24959169d9ad317333bdda4ec3c4002
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
