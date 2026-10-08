#include <fstream>
#include <filesystem>
#include <compiler/node.h>

class LLVM
{
public:
    static void runLLVM(const std::string &outputFile, RootNode *root);
    static void mapToIR(RootNode *root);
    static void generateExecutable(const std::string &outputFile);
};
