#pragma once
#include <iostream>
#include <exception>
#include <string>

class CompilerError : public std::exception
{
protected:
    std::string message;

public:
    CompilerError(const std::string &msg) : message(msg) {};

    // Override what() to provide custom message
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

class LexicalError : public CompilerError
{
public:
    LexicalError(const std::string &msg) : CompilerError(msg) {};
};

class SyntaxError : public CompilerError
{
private:
    std::string message;

public:
    SyntaxError(const std::string &msg) : CompilerError(msg) {};
};

class SemanticError : public CompilerError
{
private:
    std::string message;

public:
    SemanticError(const std::string &msg) : CompilerError(msg) {};
};