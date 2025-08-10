# Trading Simulator

A simple trading simulator with a matching engine and central limit order book implementation in C++ 20.
Includes unit tests using Google Test.

## Features

- Central limit order book per symbol
- Matching engine for buy/sell orders
- Order cancellation and amendment
- Time and price priority matching
- Unit tests for all core functionality

## Build Instructions

1. **Install dependencies:**
   - CMake (≥3.10)

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

## Usage

TBD
