#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <string>

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
        FOR,
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
    std::string getValue() const;
    std::string getToken() const;
    static const std::vector<std::string> keywords, oneCharOperators, twoCharOperators;
};

class Lexer
{
private:
    std::vector<Token> tokens;
    bool validNumber(const std::string &str);
    int countSubstrings(const std::string &str, const std::string &substr);

public:
    Lexer();
    void tokenizeFile(std::ifstream &file);
    void readTokens() const;
    void readTokenValues() const;
    std::vector<Token> getTokens() const;
};
