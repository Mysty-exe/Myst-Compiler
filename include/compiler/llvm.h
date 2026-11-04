#pragma once
#include <fstream>
#include <filesystem>
#include <array>
#include <memory>
#include "compiler/ast.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

class LLVM
{
public:
    static void mapToIR(std::vector<AbstractSyntaxTree *> roots);
    static void generateExecutable(const std::string &outputFile);
};
