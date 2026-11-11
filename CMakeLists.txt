cmake_minimum_required(VERSION 3.15...3.31)
project(MystCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

# 1. Create a static library for all core compiler components (everything except main.cpp)
add_library(myst_lib STATIC
    src/lexer.cpp
    src/node.cpp
    src/ast.cpp
    src/llvm.cpp
    include/compiler/error.h
    include/compiler/scope.h
    include/compiler/node.h
    include/compiler/lexer.h
    include/compiler/ast.h
    include/compiler/llvm.h
)

# 2. Create the executable target containing only main.cpp
add_executable(mystc src/main.cpp)

# 3. Link the core compiler library to your executable
target_link_libraries(mystc PRIVATE myst_lib)

# 4. Compile features and include directories for both targets
target_compile_features(myst_lib PRIVATE cxx_std_17)
target_compile_features(mystc PRIVATE cxx_std_17)

target_include_directories(myst_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(mystc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 5. Output directory properties for the executable binary
set_target_properties(mystc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/"
)

find_package(Python3 REQUIRED)

add_test(
    NAME GoldenExpressions
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/expr_comprehensive.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/expr_comprehensive.out
    ../tests/
)

add_test(
    NAME GoldenFactorial
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/factorial.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/factorial.out
    ../tests/
)

add_test(
    NAME GoldenReferences
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/references.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/references.out
    ../tests/
)

add_test(
    NAME GoldenDecimals
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/decimals.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/decimals.out
    ../tests/
)

add_test(
    NAME GoldenTypesAndVars
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/types_and_vars.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/types_and_vars.out
    ../tests/
)

add_test(
    NAME GoldenMultipleReferences
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/multiple_references.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/multiple_references.out
    ../tests/
)

add_test(
    NAME GoldenMultipleReturns
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/multiple_returns.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/multiple_returns.out
    ../tests/
)

add_test(
    NAME GoldenNestedLoops
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/nested_loops.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/nested_loops.out
    ../tests/
)

add_test(
    NAME GoldenScopeShadowing
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/scope_shadowing.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/scope_shadowing.out
    ../tests/
)

add_test(
    NAME GoldenScopeImportsExports
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
    $<TARGET_FILE:mystc>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/modules/utility_import.ms
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/utility.out
    ../tests/modules/
)

# add_test(
# NAME GoldenScopeContainers
# COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
# $<TARGET_FILE:mystc>
# ${CMAKE_CURRENT_SOURCE_DIR}/tests/containers.ms
# ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/containers.out
# )

# add_test(
# NAME GoldenImportExportContainers
# COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
# $<TARGET_FILE:mystc>
# ${CMAKE_CURRENT_SOURCE_DIR}/tests/modules/import_containers.ms
# ${CMAKE_CURRENT_SOURCE_DIR}/tests/output/import_export_containers.out
# )