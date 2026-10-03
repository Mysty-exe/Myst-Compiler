#include <iostream>
#include <exception>
#include <string>

class LexicalError : public std::exception
{
private:
    std::string message;

public:
    LexicalError(const std::string &msg) : message(msg) {}

    // Override what() to provide custom message
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

class SyntaxError : public std::exception
{
private:
    std::string message;

public:
    SyntaxError(const std::string &msg) : message(msg) {}

    // Override what() to provide custom message
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

class SemanticError : public std::exception
{
private:
    std::string message;

public:
    SemanticError(const std::string &msg) : message(msg) {}

    // Override what() to provide custom message
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};