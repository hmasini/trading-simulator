FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y build-essential cmake libboost-all-dev libncurses-dev git wget \
    && apt-get install -y python3 python3-pip

# Install nlohmann_json
RUN apt-get install -y nlohmann-json3-dev

# Copy source code
WORKDIR /app
COPY . .

# Build the backend
RUN mkdir -p build && cd build && cmake .. && make

# Expose WebSocket server port
EXPOSE 9001

# Default command (run WebSocket server)
CMD ["./build/ws_server"]