#include <iostream>
#include <fstream>
#include <string>
#include "compiler/lexer.h"

void printHelp()
{
    std::cout << "Usage: mystc [file]" << std::endl;
}

void parseFile(std::ifstream &inputFile)
{
    Lexer lexer;
    lexer.tokenizeFile(inputFile);
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        std::string file = argv[1];
        if (file == "-help")
        {
            printHelp();
            return 0;
        }

        std::ifstream inputFile("../" + file);
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
