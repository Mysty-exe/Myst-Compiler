#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
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

void throwImportError(std::string message)
{
    throw std::runtime_error("CircularImportError: " + message);
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

void analyzeFiles(std::vector<AbstractSyntaxTree *> &asts)
{
    std::vector<std::string> loadedAsts;
    bool skip = false;
    for (int i = 0; i < asts.size(); i++)
    {
        AbstractSyntaxTree *ast = asts[i];

        if (std::find(loadedAsts.begin(), loadedAsts.end(), ast->getRoot()->getFileName()) != loadedAsts.end())
            continue;

        for (const std::string import : ast->getImports())
        {
            if (std::find(loadedAsts.begin(), loadedAsts.end(), import) == loadedAsts.end())
            {
                skip = true;
                continue;
            }
        }

        RootNode::setCurrentFile(ast->getRoot()->getFileName());

        std::unordered_map<std::string, Scope> symbolTable;
        std::vector<std::string> visibleScope;

        std::unordered_map<std::string, std::unordered_map<std::string, Scope>> prefixSymbols = ast->getPrefixSymbols();
        ast->getRoot()->checkSemantics(symbolTable, visibleScope, prefixSymbols);
        ast->setSymbolTable(symbolTable);
        loadedAsts.push_back(ast->getRoot()->getFileName());

        for (AbstractSyntaxTree *a : asts)
        {
            if (ast == a)
                continue;

            for (int i = 0; i < a->getImports().size(); i++)
            {
                if (a->getImports()[i] == ast->getRoot()->getFileName())
                    a->setSymbols(a->getPrefixes()[i], symbolTable);
            }
        }

        if (parameters.seeAST)
            ast->printTree();

        if (i == asts.size() - 1)
        {
            if (skip)
                i = 0;
            skip = false;
        }
    }

    for (AbstractSyntaxTree *ast : asts)
    {
        RootNode::setCurrentFile(ast->getRoot()->getFileName());

        if (!parameters.disableWarnings)
            ast->getRoot()->sendWarnings(ast->getSymbolTable());

        for (auto &[_, scope] : ast->getSymbolTable())
        {
            for (Symbol *symbol : scope.getSymbols())
                delete symbol;
        }

        ast->format();
    }

    std::cout << std::endl;
}

AbstractSyntaxTree *parseFile(const std::string &name, std::ifstream &inputFile)
{
    Lexer lexer;
    lexer.tokenizeFile(inputFile);

    if (parameters.seeTokens)
        lexer.readTokens();

    AbstractSyntaxTree *ast = new AbstractSyntaxTree(lexer.getTokens(), name);

    return ast;
}

int runFiles(std::string file, std::vector<AbstractSyntaxTree *> &asts, bool importedFile = false)
{
    std::ifstream inputFile(file);
    if (!inputFile.is_open())
    {
        throwParameterError("Could not open the file '" + file + "'.");
        return -1;
    }

    for (AbstractSyntaxTree *a : asts)
    {
        if (a->getRoot()->getFileName() == file)
            return 0;
    }

    AbstractSyntaxTree *ast = parseFile(file, inputFile);
    if (importedFile)
    {
        ast->setImported(true);
    }

    asts.insert(asts.begin(), ast);

    if (asts.size() > 1)
    {
        for (int i = 0; i < asts[0]->getImports().size(); i++)
        {
            for (int k = 0; k < asts[1]->getImports().size(); k++)
            {
                std::string f1 = ast->getRoot()->getFileName(), f2 = asts[1]->getRoot()->getFileName();
                std::string f1name = ast->getImports()[i], f2name = asts[1]->getImports()[k];
                if (f2.substr(f2.size() - f1name.size() - 3, f1name.size()) == f1name || f1.substr(f1.size() - f2name.size() - 3, f2name.size()) == f1name)
                    throwImportError("Module A depends on Module B which depends back on Module A.");
            }
        }
    }

    if (ast->hasImports())
    {
        std::vector<std::string> imports;
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
            for (int i = 0; i < import.size() - 3; i++)
            {
                if (import[i] == '.')
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
            newImport += ".ms";

            if (file.size() > 0)
            {
                if (std::filesystem::equivalent(file, (folder + newImport)))
                    throwImportError("Module A is being imported in the same file it lives in.");
            }

            imports.push_back(folder + newImport);
            runFiles(folder + newImport, asts, true);
        }

        ast->setImports(imports);
    }

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
    analyzeFiles(asts);

    std::string output = (parameters.outputFile.size() == 0) ? parameters.inputFile.substr(0, parameters.inputFile.size() - 3) : parameters.outputFile;
    LLVM::mapToIR(asts);
    LLVM::generateExecutable(output);

    return 0;
}
