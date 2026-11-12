FROM ubuntu:26.04@sha256:065aa8626841f16057b78b027b464cd59c4fac36c204c72dcbf48d66f5fa8a96 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Do not install libev-dev because we need libev to be built with -fPIC.
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates cmake file git clang make nlohmann-json3-dev pkgconf patch unixodbc-dev && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_MOSTLY_STATIC_BINARY=on && \
    cmake --build build && \
    strip --strip-unneeded build/src/tfhttp


FROM ubuntu:26.04@sha256:065aa8626841f16057b78b027b464cd59c4fac36c204c72dcbf48d66f5fa8a96

ENV DEBIAN_FRONTEND=noninteractive

RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends libsqliteodbc odbc-mariadb libodbc2 && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

COPY --from=build /app/build/src/tfhttp /usr/bin/tfhttp

ENTRYPOINT ["/usr/bin/tfhttp"]
