#include <iostream>
#include <fstream>
#include <string>
#include "compiler/lexer.h"
#include "compiler/node.h"
#include "compiler/ast.h"
#include "compiler/llvm.h"

void throwParameterError(std::string message)
{
    try
    {
        throw std::invalid_argument("Invalid Parameters Provided");
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "\033[31m";
        std::cerr << e.what() << ": ";
        std::cerr << "\033[0m";
        std::cout << message << std::endl;
        exit(-1);
    }
}

struct Parameters
{
    bool help, disableWarnings, seeTokens, seeAST;
    std::string inputFile, outputFile;

    Parameters()
    {
        help = false, disableWarnings = false, seeTokens = false, seeAST = false;
        inputFile = "", outputFile = "";
    }
};

Parameters parameters = Parameters();

void printHelp()
{
    std::cout << "Usage: mystc [file]" << std::endl;
}

std::string getExtension(const std::string &file)
{
    std::string ext = "";
    for (int i = file.size() - 1; i > -1; i--)
    {
        if (file[i] == '.')
            return ext;
        else
            ext.insert(0, 1, file[i]);
    }

    return "";
}

void analyzeTree(AbstractSyntaxTree &ast)
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::vector<std::string> visibleScope;

    ast.getRoot()->checkSemantics(symbolTable, visibleScope);
    if (!parameters.disableWarnings)
        ast.getRoot()->sendWarnings(symbolTable);
}

AbstractSyntaxTree parseFile(std::ifstream &inputFile)
{
    Lexer lexer;
    lexer.tokenizeFile(inputFile);

    if (parameters.seeTokens)
        lexer.readTokens();

    AbstractSyntaxTree ast = AbstractSyntaxTree(lexer.getTokens());
    if (parameters.seeAST)
        ast.printTree();

    return ast;
}

int main(int argc, char *argv[])
{
    for (int arg = 1; arg < argc; arg++)
    {
        std::string parameter = argv[arg];
        if (arg == 1)
        {
            if (parameter == "-help")
            {
                parameters.help = true;
                break;
            }
        }

        std::string ext = getExtension(parameter);
        if (ext == "ms")
            parameters.inputFile = parameter;

        if (parameter == "-w")
            parameters.disableWarnings = true;

        if (parameter == "-t")
            parameters.seeTokens = true;

        if (parameter == "-a")
            parameters.seeAST = true;

        if (parameter == "-o")
        {
            if (arg + 1 >= argc)
            {
                throwParameterError("Output File Not Found");
            }
            parameters.outputFile = argv[arg + 1];
            break;
        }
    }

    if (parameters.help)
    {
        printHelp();
        return 0;
    }

    if (parameters.inputFile.size() < 4)
    {
        throwParameterError("No Input Files Found, Compilation Terminated.");
        return -1;
    }

    std::ifstream inputFile(parameters.inputFile);
    if (!inputFile.is_open())
    {
        throwParameterError("Could not open the file.");
        return -1;
    }

    AbstractSyntaxTree ast = parseFile(inputFile);
    analyzeTree(ast);

    std::string output = (parameters.outputFile.size() == 0) ? parameters.inputFile.substr(0, parameters.inputFile.size() - 3) : parameters.outputFile;
    LLVM::runLLVM(output, ast.getRoot());

    return 0;
}
