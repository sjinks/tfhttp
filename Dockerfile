FROM ubuntu:mantic-20240122@sha256:f0bb9ee844f7adb284ac036a15469062adbe3a4458c06680216ed73df231cb31 AS build

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

FROM cgr.dev/chainguard/glibc-dynamic:latest@sha256:1790b6f92f7b5d817fd936e95d8a0dfbbc0f5a3648eac67961e66ab5ffbb7fbd
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
