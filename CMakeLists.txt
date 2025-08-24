cmake_minimum_required(VERSION 3.16)

project(trading_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include header files from inc/
include_directories(inc)

# Collect source files
set(SOURCES
    src/matching_engine.cpp
    src/random_bot.cpp
    src/cli.cpp
    src/logger.cpp
    src/main.cpp
)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

find_package(nlohmann_json REQUIRED)

# Create executable
add_executable(trading_sim ${SOURCES})

# Create executable for WebSocket server
add_executable(ws_server src/ws_server.cpp)

add_library(trading_sim_lib
    src/matching_engine.cpp
    src/random_bot.cpp
    src/cli.cpp
    src/logger.cpp
)

target_link_libraries(trading_sim ${CURSES_LIBRARIES})

target_link_libraries(ws_server trading_sim_lib ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)

# Unit tests
enable_testing()
add_subdirectory(tests)