cmake_minimum_required(VERSION 3.15)
project(MiniCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build standalone project tests" ON)
option(TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

add_library(compiler_core STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
)

target_include_directories(compiler_core PUBLIC src)

if (MSVC)
    target_compile_options(compiler_core PRIVATE /W4)
else()
    target_compile_options(compiler_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (TREAT_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(compiler_core PRIVATE /WX)
    else()
        target_compile_options(compiler_core PRIVATE -Werror)
    endif()
endif()

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE compiler_core)

if (MSVC)
    target_compile_options(compiler PRIVATE /W4)
else()
    target_compile_options(compiler PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (TREAT_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(compiler PRIVATE /WX)
    else()
        target_compile_options(compiler PRIVATE -Werror)
    endif()
endif()

if (BUILD_TESTS)
    enable_testing()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests.cpp")
        add_executable(core_tests tests/core_tests.cpp)
        target_link_libraries(core_tests PRIVATE compiler_core)
        add_test(NAME CoreTests COMMAND core_tests)
    endif()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests_extended.cpp")
        add_executable(core_tests_extended tests/core_tests_extended.cpp)
        target_link_libraries(core_tests_extended PRIVATE compiler_core)
        add_test(NAME CoreTestsExtended COMMAND core_tests_extended)
    endif()
endif()
