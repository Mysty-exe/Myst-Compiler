#include <iostream>
#include <fstream>
#include <string>
#include "compiler/lexer.h"
#include "compiler/ast.h"

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

void parseFile(std::ifstream &inputFile)
{
    Lexer lexer;
    lexer.tokenizeFile(inputFile);
    lexer.readTokens();

    AbstractSyntaxTree ast(lexer.getTokens());
    ast.printTree();
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        std::string file = argv[1];
        std::string ext = getExtension(file);
        if (file == "-help")
        {
            printHelp();
            return 0;
        }

        if (ext != "ms")
            std::cerr << "Unknown File Format Detected" << std::endl;

        std::ifstream inputFile(file);
        if (!inputFile.is_open())
        {
            std::cerr << "Error: Could not open the file!" << std::endl;
            return 1;
        }

        parseFile(inputFile);
    }
    else
    {
        std::cout << "mystc: No Input Files" << std::endl;
        std::cout << "Comilation Terminated." << std::endl;
        return -1;
    }

    return 0;
}
