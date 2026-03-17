FROM ubuntu:24.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  g++ \
  ca-certificates \
  git \
  wget \
  && rm -rf /var/lib/apt/lists/*

# Install CMake
ARG CMAKE_VERSION=3.30.3
ARG TARGETARCH

RUN case "${TARGETARCH}" in \
  amd64) CMAKE_ARCH=linux-x86_64 ;; \
  arm64) CMAKE_ARCH=linux-aarch64 ;; \
  *) echo "Unsupported architecture: ${TARGETARCH}" && exit 1 ;; \
  esac \
  && wget -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.sh \
  && chmod +x cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.sh \
  && ./cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.sh --skip-license --prefix=/usr/local \
  && rm cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.sh

# Install Boost
RUN wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz \
  && tar xzf boost_1_87_0.tar.gz \
  && cd boost_1_87_0 \
  && ./bootstrap.sh \
  && ./b2 install

WORKDIR /src

COPY connector connector
COPY symbolic-reasoner symbolic-reasoner
COPY third_party third_party
COPY common-test-resources common-test-resources
COPY common common
COPY CMakeLists.txt .

WORKDIR /src/build

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make reasoner_client -j8

ENTRYPOINT ["/src/build/bin/reasoner_client"]