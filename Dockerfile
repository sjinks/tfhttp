FROM ubuntu:mantic-20240405@sha256:008c4aa50da87ad7bc586a47787491f76c79c62cc5bdbee29121ee9e02c03f3e AS build

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

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:e1f6ffa9629cc74d31d3d28007d846264f7a6cc2447bfa4275e9e7238a6619c3
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
