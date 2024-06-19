FROM ubuntu:24.04@sha256:e3f92abc0967a6c19d0dfa2d55838833e947b9d74edbcb0113e48535ad4be12a AS build

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

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:9850190b2e79687e2ffe9948f1648ca780e8a2461dabfb3e275a95f7912f4081
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
