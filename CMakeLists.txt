cmake_minimum_required(VERSION 3.15...3.31)
project(MystCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mystc src/main.cpp)

target_compile_features(mystc PRIVATE cxx_std_17)

target_sources(mystc PRIVATE
    include/compiler/lexer.h
)

target_include_directories(mystc PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
