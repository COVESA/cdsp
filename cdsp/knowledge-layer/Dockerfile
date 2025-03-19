FROM ubuntu:24.10 AS builder

RUN apt-get -y update && apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  g++ \
  ca-certificates \
  git \
  wget

RUN wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz \
  && tar xzf boost_1_87_0.tar.gz \
  && cd boost_1_87_0 \
  && ./bootstrap.sh \
  && ./b2 install

WORKDIR /src

COPY connector connector
COPY numeric-analyzer numeric-analyzer
COPY symbolic-reasoner symbolic-reasoner
COPY third_party third_party
COPY common-test-resources common-test-resources
COPY CMakeLists.txt .

WORKDIR /src/build

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make reasoner_client -j8

ENTRYPOINT ["/src/build/bin/reasoner_client"]

