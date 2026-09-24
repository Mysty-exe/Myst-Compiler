cmake_minimum_required(VERSION 3.15...3.31)

project(MystCompiler LANGUAGES CXX)

add_executable(hello_world main.cpp)

target_compile_features(hello_world PRIVATE cxx_std_17)
