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
    std::cout << "Usage: mystc [file] {flags}" << std::endl;
    std::cout << "\nFlags:" << std::endl;
    std::cout << "-w = Disables Warnings" << std::endl;
    std::cout << "-t = Prints the Tokens" << std::endl;
    std::cout << "-a = Prints the Abstract Syntax Tree" << std::endl;
    std::cout << "-o {file} = Set Executable Name" << std::endl;
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

void analyzeTree(AbstractSyntaxTree *ast)
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::vector<std::string> visibleScope;

    ast->getRoot()->checkSemantics(symbolTable, visibleScope);

    if (parameters.seeAST)
        ast->printTree();

    if (!parameters.disableWarnings)
        ast->getRoot()->sendWarnings(symbolTable);

    for (auto &[_, scope] : symbolTable)
    {
        for (Symbol *symbol : scope.getSymbols())
            delete symbol;
    }
}

AbstractSyntaxTree *parseFile(std::ifstream &inputFile)
{
    Lexer lexer;
    lexer.tokenizeFile(inputFile);

    if (parameters.seeTokens)
        lexer.readTokens();

    AbstractSyntaxTree *ast = new AbstractSyntaxTree(lexer.getTokens(), parameters.inputFile);

    return ast;
}

int runFiles(std::string file, std::vector<AbstractSyntaxTree *> &asts, std::string prefix = "", bool importedFile = false)
{
    std::ifstream inputFile(file);
    if (!inputFile.is_open())
    {
        throwParameterError("Could not open the file '" + file + "'.");
        return -1;
    }

    AbstractSyntaxTree *ast = parseFile(inputFile);
    if (importedFile)
    {
        ast->setImported(true);
        ast->setPrefix(prefix);
    }

    asts.insert(asts.begin(), ast);

    if (ast->hasImports())
    {
        for (int x = 0; x < ast->getImports().size(); x++)
        {
            std::string import = ast->getImports()[x];
            if (import.substr(import.size() - 3, 3) != ".ms")
                import += ".ms";

            std::string folder;
            for (int i = file.size() - 1; i >= 0; i--)
            {
                if (file[i] == '/' || file[i] == '\\')
                {
                    folder = file.substr(0, i + 1);
                    break;
                }
            }

            std::string newImport;
            for (int i = 0; i < import.size(); i++)
            {
                if (i == '.')
                {
#if defined(_WIN32)
                    newImport.push_back('\\');
#else
                    newImport.push_back('/');
#endif
                }
                else
                    newImport.push_back(import[i]);
            }

            runFiles(folder + newImport, asts, ast->getPrefixes()[x], true);
        }
    }

    analyzeTree(ast);
    ast->format();

    return 0;
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

    std::vector<AbstractSyntaxTree *> asts;
    runFiles(parameters.inputFile, asts);

    std::string output = (parameters.outputFile.size() == 0) ? parameters.inputFile.substr(0, parameters.inputFile.size() - 3) : parameters.outputFile;
    LLVM::mapToIR(asts);
    LLVM::generateExecutable(output);

    return 0;
}
