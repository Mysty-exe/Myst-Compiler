#pragma once
#include <fstream>
#include <filesystem>
#include <array>
#include <memory>
#include "compiler/node.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

class LLVM
{
public:
    static void runLLVM(const std::string &outputFile, RootNode *root);
    static void mapToIR(RootNode *root);
    static void generateExecutable(const std::string &outputFile);
};
