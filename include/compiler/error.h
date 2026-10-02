#include <iostream>
#include <exception>
#include <string>

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