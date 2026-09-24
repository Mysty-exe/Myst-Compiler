#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <algorithm>

struct Token
{
private:
    enum TokenKeywords
    {
        FUNC,
        STRUCT,
        CONST,
        VAR,
        IF,
        ELSEIF,
        ELSE,
        WHILE,
        BREAK,
        RETURN,
        IMPORT,
        IDENTIFIER,
        INT,
        FLOAT,
        STRING,
        COMMENT,
        ASSIGN,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        EQUALS,
        LESSTHAN,
        GREATERTHAN,
        LESSTHANEQ,
        GREATERTHANEQ,
        NOTEQUAL,
        AND,
        OR,
        COLON,
        ARROW,
        LPAREN,
        RPAREN,
        COMMA,
        DOT,
        NEWLINE,
        INDENT,
        ENDFILE,
        UNKNOWN
    } type;

    std::string value;
    int lineNumber, columnNumber;

public:
    Token(std::string value, int line, int col);
};

class Lexer
{
private:
    std::vector<Token> tokens;

public:
    Lexer();
    void tokenizeFile(std::ifstream &file);
    std::vector<Token> getTokens() const;
};
