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
    main.cpp
)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Create executable
add_executable(trading_sim ${SOURCES})

target_link_libraries(trading_sim ${CURSES_LIBRARIES})

# Unit tests
enable_testing()
add_subdirectory(tests)