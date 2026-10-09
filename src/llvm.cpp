#include <compiler/llvm.h>

std::string execCommand(const std::string &cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}

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

    BlockNode *global = static_cast<BlockNode *>(root->getGlobalCode());
    std::string ir = global->mapToGlobalIR();
    tempFile << ir;

    for (Node *node : root->getChildren())
    {
        FuncNode *function = static_cast<FuncNode *>(node);
    }

    tempFile << global->mapToIR(true);
    tempFile.close();
}

void LLVM::generateExecutable(const std::string &outputFile)
{
    try
    {
        std::string output = execCommand("clang temp.ll -o " + outputFile);
        std::cout << output;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // std::filesystem::remove("temp.ll");
}
