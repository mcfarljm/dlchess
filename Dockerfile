FROM ubuntu:22.04

RUN apt-get update && apt-get install --no-install-recommends -y python3.10-dev python3-pip build-essential cmake wget unzip git && \
	apt-get clean && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir click && \
    pip3 install --no-cache-dir torch torchvision --index-url https://download.pytorch.org/whl/cpu

WORKDIR /opt
RUN wget -q https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip && \
    unzip -q libtorch-shared-with-deps-latest.zip

WORKDIR /app
COPY src src
COPY CMakeLists.txt .

WORKDIR /app/build
RUN cmake -DCMAKE_PREFIX_PATH=/opt/libtorch -DCMAKE_BUILD_TYPE=RELEASE .. && \
    make -j4