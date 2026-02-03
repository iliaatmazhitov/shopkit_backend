FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    libjsoncpp-dev \
    libssl-dev \
    zlib1g-dev \
    libbrotli-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Drogon framework
RUN git clone https://github.com/drogonframework/drogon /tmp/drogon && \
    cd /tmp/drogon && \
    git submodule update --init && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/drogon

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build application
RUN cd shopkit && mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Create required directories
RUN mkdir -p /app/shopkit/build/uploads/products /app/shopkit/build/logs

# Set working directory to build output
WORKDIR /app/shopkit/build

# Expose port
EXPOSE 8080

# Run application
CMD ["./shopkit"]
