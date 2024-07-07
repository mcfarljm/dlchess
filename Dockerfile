FROM ubuntu:22.04

RUN apt-get update && apt-get install --no-install-recommends -y \
    wget unzip git python3.10-dev python3-pip build-essential cmake \
    ca-certificates qtbase5-dev qtbase5-dev-tools libqt5svg5-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Install cutechess:
WORKDIR /opt
RUN wget -q https://github.com/cutechess/cutechess/archive/refs/tags/v1.3.1.zip && \
    unzip -q v1.3.1.zip

WORKDIR /opt/cutechess-1.3.1
RUN mkdir build && cd build && cmake .. && make -j 4 && make install

# Install pytorch
RUN pip3 install --no-cache-dir click && \
    pip3 install --no-cache-dir torch torchvision --index-url https://download.pytorch.org/whl/cpu

# Install ONNX library and Python support.  Note Python onnxruntime is only needed for
# benchmarking the onnx models from Python.
RUN pip3 install --no-cache-dir onnx onnxruntime
WORKDIR /opt
RUN wget -q https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz && \
    tar -xvf onnxruntime-linux-x64-1.16.3.tgz

# Build app
WORKDIR /app
COPY src src
COPY CMakeLists.txt .

WORKDIR /app/build
RUN cmake -DONNXRUNTIME_ROOTDIR=/opt/onnxruntime-linux-x64-1.16.3 -DCMAKE_BUILD_TYPE=RELEASE .. && \
    make -j4
