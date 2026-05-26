cmake_minimum_required(VERSION 3.15)
project(MiniCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_library(compiler_core STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp

    src/utils/diagnostics.cpp

    src/parser/ast.cpp
    src/parser/parser.cpp

    src/semantic/type_system.cpp
    src/semantic/symbol_table.cpp
    src/semantic/errors.cpp
    src/semantic/analyzer.cpp

    src/ir/ir_instructions.cpp
    src/ir/basic_block.cpp
    src/ir/control_flow.cpp
    src/ir/ir_generator.cpp
    src/ir/optimizer.cpp

    src/codegen/abi.cpp
    src/codegen/stack_frame.cpp
    src/codegen/register_allocator.cpp
    src/codegen/label_manager.cpp
    src/codegen/control_flow_generator.cpp
    src/codegen/expression_generator.cpp
    src/codegen/array_generator.cpp
    src/codegen/external_calls.cpp
    src/codegen/optimization_passes.cpp
    src/codegen/x86_generator.cpp
)

target_include_directories(compiler_core PUBLIC src)

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE compiler_core)

add_executable(mycc src/main.cpp)
target_link_libraries(mycc PRIVATE compiler_core)

option(BUILD_TESTS "Build MiniCompiler tests" OFF)

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
        add_test(NAME CoreExtendedTests COMMAND core_tests_extended)
    endif()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/semantic_tests.cpp")
        add_executable(semantic_tests tests/semantic_tests.cpp)
        target_link_libraries(semantic_tests PRIVATE compiler_core)
        add_test(NAME SemanticTests COMMAND semantic_tests)
    endif()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/ir_tests.cpp")
        add_executable(ir_tests tests/ir_tests.cpp)
        target_link_libraries(ir_tests PRIVATE compiler_core)
        add_test(NAME IRTests COMMAND ir_tests)
    endif()

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/codegen_tests.cpp")
        add_executable(codegen_tests tests/codegen_tests.cpp)
        target_link_libraries(codegen_tests PRIVATE compiler_core)
        add_test(NAME CodegenTests COMMAND codegen_tests)
    endif()
endif()
