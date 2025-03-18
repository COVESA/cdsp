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

COPY . .

WORKDIR /src/build

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make reasoner_client -j8

RUN ln -s /src/build/bin/reasoner_client /usr/local/bin/reasoner_client

CMD ["reasoner_client"]
