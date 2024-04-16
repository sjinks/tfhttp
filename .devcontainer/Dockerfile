FROM ubuntu:mantic-20240405@sha256:008c4aa50da87ad7bc586a47787491f76c79c62cc5bdbee29121ee9e02c03f3e AS build

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV LANGUAGE=en_US

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends bash sudo ca-certificates wget file cmake git make pkgconf valgrind autoconf automake libtool m4 ninja-build && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends openjdk-21-jre-headless nodejs npm && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends clang clang-format clang-tidy clang-tools && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends nlohmann-json3-dev libsqlite3-dev libev-dev zlib1g-dev && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    echo "ubuntu ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/ubuntu" && \
    chmod 0440 "/etc/sudoers.d/ubuntu"

RUN sudo npm i -g @microsoft/sarif-multitool

WORKDIR /usr/src

RUN \
    git clone https://github.com/ada-url/ada.git --depth 1 -b v2.7.7 && \
    cd ada && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DADA_TOOLS=off -DADA_TESTING=off && \
    cmake --build build && \
    cmake --install build

RUN \
    wget -q -O - https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.2.0.tar.gz | tar xzf - && \
    cd llhttp-release-v* && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON && \
    cmake --build build && \
    cmake --install build

RUN \
    git clone https://github.com/iwongu/sqlite3pp.git --depth 1 -b v1.0.9 && \
    cd sqlite3pp/headeronly_src && \
    cp ./* /usr/local/include

RUN \
    wget -q -O - https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.8.2.tar.gz | tar xzf - && \
    cd libressl-* && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF && \
    cmake --build build && \
    cmake --install build

ENV CCC_CC=clang
ENV CCC_CXX=clang++
ENV CC=clang
ENV CXX=clang++
ENV DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1
