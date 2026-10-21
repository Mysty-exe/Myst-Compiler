#include <compiler/llvm.h>

std::string getTargetTriple()
{
#if defined(_WIN32)
    return "x86_64-pc-windows-msvc";
#elif defined(__linux__)
    return "x86_64-pc-linux-gnu";
#else
    return "x86_64-unknown-unknown";
#endif
}

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
    std::vector<std::vector<std::vector<std::string>>> tempvarMap = {{}};
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

    for (Node *node : root->getChildren())
    {
        FuncNode *function = static_cast<FuncNode *>(node);
        function->mapToIR(ir, tempvarMap, false);
    }

    global->mapToIR(ir, tempvarMap, true);
    tempFile << "target triple = \"" << getTargetTriple() << "\"\n\n";
    tempFile << ir;
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
