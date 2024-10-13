FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*
RUN cd /tmp/ \
    && git clone https://github.com/redis/hiredis.git \
    && cd hiredis/ \
    && make -j${nproc} \
    && make install
RUN rm -rf /tmp/hiredis
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
WORKDIR /app
COPY src /app/src
COPY include /app/include
COPY CMakeLists.txt /app
RUN mkdir -p /app/build; cd /app/build/ ; cmake .. ; make -j$(nproc)
RUN cp /app/build/p2p_server /tmp/
RUN rm -rf /app/* && mv /tmp/p2p_server /app
EXPOSE 8080
ENTRYPOINT ["./p2p_server"]
CMD ["192.168.0.190", "6379"]