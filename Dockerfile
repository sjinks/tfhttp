FROM ubuntu:mantic-20240216@sha256:5cd569b792a8b7b483d90942381cd7e0b03f0a15520d6e23fb7a1464a25a71b1 AS build

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

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:96342877337575d814046aea77eb2e8ffd92f19c3115dbe259e6e0ec89d81c7b
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
