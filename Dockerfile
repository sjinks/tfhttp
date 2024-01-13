FROM ubuntu:23.10 AS build

ENV DEBIAN_FRONTEND=noninteractive
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates cmake git clang make python3-minimal python3-whichcraft nlohmann-json3-dev libsqlite3-dev libev-dev pkgconf
COPY . /app
WORKDIR /app

RUN \
    cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel && \
    cmake --build build && \
    strip --strip-unneeded build/src/tfhttp

FROM scratch
COPY --from=build /app/build/src/tfhttp /tfhttp
ENTRYPOINT ["/tfhttp"]
