#include <compiler/llvm.h>

void LLVM::runLLVM(const std::string &outputFile, RootNode *root)
{
    mapToIR(root);
    generateExecutable(outputFile);
}

void LLVM::mapToIR(RootNode *root)
{
    std::ofstream tempFile("temp.ll");
    if (!tempFile.is_open())
    {
        std::cerr << "\033[31m";
        std::cerr << "Error: ";
        std::cerr << "\033[0m";
        std::cout << "Couldn't open intermediate artifact." << std::endl;
    }

    for (Node *node : root->getChildren())
    {
    }
}

void LLVM::generateExecutable(const std::string &outputFile)
{
    std::filesystem::remove("temp.ll");
}
