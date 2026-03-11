# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(MiniCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Core library
add_library(compiler_core STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    # Add preprocessor if implementing stretch
)

target_include_directories(compiler_core PUBLIC src)

# Executable
add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE compiler_core)

# Tests
enable_testing()
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lexer_tests tests/lexer_test.cpp)
target_link_libraries(lexer_tests PRIVATE compiler_core GTest::gtest_main)
add_test(NAME LexerTests COMMAND lexer_tests)

# For file-based tests, we can add a custom target or use a script, but for simplicity, use GTest to run file tests too