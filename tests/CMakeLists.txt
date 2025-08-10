include(FetchContent)

# Download GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Make available
FetchContent_MakeAvailable(googletest)

# Link gtest to your tests
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_matching_engine test_matching_engine.cpp ../src/matching_engine.cpp)
target_include_directories(test_matching_engine PRIVATE ../inc)
target_link_libraries(test_matching_engine gtest gtest_main)

add_test(NAME TestMatchingEngine COMMAND test_matching_engine)