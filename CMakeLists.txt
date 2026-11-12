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

file(GLOB TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/tests/integrations ${CMAKE_CURRENT_SOURCE_DIR}/tests/integrations/*.ms)

foreach(TEST_FILE ${TEST_FILES})
    string(REPLACE ".ms" "" TEST_NAME ${TEST_FILE})

    set(TEST_MS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/integrations/${TEST_FILE}")
    set(TEST_OUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/output/${TEST_NAME}.out")

    add_test(
        NAME Golden_${TEST_NAME}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runTest.py
        $<TARGET_FILE:mystc>
        ${TEST_MS_PATH}
        ${TEST_OUT_PATH}
    )
endforeach()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Create the unit test executable pointing to your single test file
add_executable(unit_tests
    tests/units/test_lexer.cpp
)

# Link against your compiler core library and Catch2
target_link_libraries(unit_tests PRIVATE myst_lib Catch2::Catch2WithMain)

# Include directories so it can find your headers
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include) # Adjust to your include folder

# Register with CTest so it runs alongside your golden tests
enable_testing()
add_test(NAME Unit_Lexer COMMAND unit_tests)
