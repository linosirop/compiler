cmake_minimum_required(VERSION 3.15)
project(MiniCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_library(compiler_core STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
    src/semantic/type_system.cpp
    src/semantic/symbol_table.cpp
    src/semantic/errors.cpp
    src/semantic/analyzer.cpp
)

target_include_directories(compiler_core PUBLIC src)

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE compiler_core)

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

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/semantic_tests.cpp")
        add_executable(semantic_tests tests/semantic_tests.cpp)
        target_link_libraries(semantic_tests PRIVATE compiler_core)
        add_test(NAME SemanticTests COMMAND semantic_tests)
    endif()
endif()
