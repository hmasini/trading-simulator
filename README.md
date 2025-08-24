# Trading Simulator

A simple trading simulator featuring a C++20 matching engine, central limit order book, bots, logging, and a live React frontend.

## Features

- Central limit order book per symbol
- Matching engine for buy/sell orders
- Order cancellation and amendment
- Time and price priority matching
- Bots that pseudo-randomly create, add, or cancel orders
- WebSocket server for real-time data streaming
- React frontend with live order book and candlestick charts
- Logging and CSV export for trades and orders
- Unit tests for the matching engine

## Project Structure

```
trading-simulator/
├── src/                # C++ source files
├── inc/                # C++ header files
├── tests/              # Unit tests (Google Test)
├── trading-frontend/   # React frontend
├── docs/               # Documentation and diagrams
├── CMakeLists.txt
├── README.md
```

## Build Instructions (C++ Backend)

1. **Install dependencies:**
   - CMake (≥3.10)
   - Boost
   - nlohmann_json
   - ncurses (for CLI)
   - Google Test (for unit tests)

2. **Configure and build:**

```sh
mkdir -p build
cd build
cmake ..
make
```

This will build the trading simulator and the test executable.

## Running Tests

From the `build` directory:

```sh
ctest
```
or run the test binary directly:

```sh
./tests/test_matching_engine
```

## Running the CLI

From the `build` directory:

```sh
./trading-sim
```

## Running the WebSocket Server

From the `build` directory:

```sh
./ws_server
```

## Running the React Frontend

1. **Install Node.js and npm** if not already installed.
2. **Install frontend dependencies:**

```sh
cd trading-frontend
npm install
```

3. **Start the frontend development server:**

```sh
npm start
```

The frontend will connect to the WebSocket server at `ws://localhost:9001` by default.

## Usage

- Start the backend WebSocket server.
- Start the React frontend.
- View live order book, trades, and candlestick charts in your browser.
- Use the CLI or bots to simulate trading activity.

## Documentation

- See `docs/flowchart.md` for a system context diagram (Mermaid).
