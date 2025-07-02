FROM ubuntu:24.04@sha256:440dcf6a5640b2ae5c77724e68787a906afb8ddee98bf86db94eea8528c2c076 AS build

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV LANGUAGE=en_US

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends bash sudo ca-certificates wget file cmake git make pkgconf valgrind autoconf automake libtool m4 ninja-build jq less patch && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends openjdk-21-jre-headless nodejs npm && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends clang clang-format clang-tidy clang-tools g++ && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y --no-install-recommends nlohmann-json3-dev libev-dev zlib1g-dev unixodbc-dev libsqliteodbc odbc-mariadb && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN \
    echo "ubuntu ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/ubuntu" && \
    chmod 0440 "/etc/sudoers.d/ubuntu"

WORKDIR /usr/src

# renovate: datasource=github-tags depName=ada-url/ada
ENV LIBADA_VERSION=v3.2.5
# renovate: datasource=github-tags depName=nodejs/llhttp
ENV LLHTTP_VERSION=v9.3.0
# renovate: datasource=github-tags depName=nanodbc/nanodbc
ENV NANODBC_VERSION=v2.14.0
# renovate: datasource=github-releases depName=libressl/portable
ENV LIBRESSL_VERSION=v4.1.0

RUN \
    git clone https://github.com/ada-url/ada.git --depth 1 -b "${LIBADA_VERSION}" && \
    cd ada && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DADA_TOOLS=off -DADA_TESTING=off && \
    cmake --build build -j $(nproc) && \
    cmake --install build

RUN \
    wget -q -O - "https://github.com/nodejs/llhttp/archive/refs/tags/release/${LLHTTP_VERSION}.tar.gz" | tar xzf - && \
    cd llhttp-release-v* && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON && \
    cmake --build build -j $(nproc) && \
    cmake --install build

RUN \
    git clone https://github.com/nanodbc/nanodbc.git --depth 1 -b "${NANODBC_VERSION}" && \
    cd nanodbc && \
    CXX=g++ cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DNANODBC_DISABLE_TESTS=ON -DNANODBC_DISABLE_EXAMPLES=ON -DNANODBC_DISABLE_LIBCXX=ON && \
    cmake --build build -j $(nproc) && \
    cmake --install build

RUN \
    wget -q -O - "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-${LIBRESSL_VERSION#v}.tar.gz" | tar xzf - && \
    cd libressl-* && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF && \
    cmake --build build -j $(nproc) && \
    cmake --install build

ENV CCC_CC=clang
ENV CCC_CXX=clang++
ENV CC=clang
ENV CXX=clang++
ENV DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1
