cmake_minimum_required(VERSION 3.15...3.31)
project(MystCompiler LANGUAGES CXX)
add_executable(compiler src/main.cpp)
target_compile_features(compiler PRIVATE cxx_std_17)
