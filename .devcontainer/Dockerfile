FROM ubuntu:23.10 AS build

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV LANGUAGE=en_US

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends bash sudo ca-certificates wget cmake git make python3-minimal python3-whichcraft pkgconf valgrind && \
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
    apt-get install -y --no-install-recommends nlohmann-json3-dev libsqlite3-dev libev-dev && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    echo "ubuntu ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/ubuntu" && \
    chmod 0440 "/etc/sudoers.d/ubuntu"

RUN sudo npm i -g @microsoft/sarif-multitool

WORKDIR /usr/src

RUN \
    git clone https://github.com/ada-url/ada.git --depth 1 -b v2.7.4 && \
    cd ada && \
    cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel -DADA_TOOLS=off -DADA_TESTING=off && \
    cmake --build build && \
    cmake --install build

RUN \
    wget -q -O - https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.1.3.tar.gz | tar xzf - && \
    cd llhttp-release-v* && \
    cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON && \
    cmake --build build && \
    cmake --install build

RUN \
    git clone https://github.com/iwongu/sqlite3pp.git --depth 1 -b v1.0.9 && \
    cd sqlite3pp/headeronly_src && \
    cp ./* /usr/local/include

ENV CCC_CC=clang
ENV CCC_CXX=clang++
ENV CC=clang
ENV CXX=clang++
ENV DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1
