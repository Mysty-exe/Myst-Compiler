#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <string>

enum class TokenType
{
    FUNC,
    VAR,
    IF,
    ELSEIF,
    ELSE,
    FOR,
    WHILE,
    BREAK,
    RETURN,
    IDENTIFIER,
    CHARTYPE,
    STRINGTYPE,
    INTTYPE,
    DECIMALTYPE,
    BOOLTYPE,
    CHAR,
    STRING,
    INT,
    DECIMAL,
    BOOL,
    COMMENT,
    ASSIGN,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    NEQUALS,
    EQUALS,
    LESSTHAN,
    GREATERTHAN,
    LESSTHANEQ,
    GREATERTHANEQ,
    NOTEQUAL,
    AND,
    OR,
    NOT,
    COLON,
    ARROW,
    LPAREN,
    RPAREN,
    COMMA,
    DOT,
    NEWLINE,
    INDENT,
    UNKNOWN
};

struct Token
{
private:
    TokenType type;

    std::string value;
    int lineNumber, columnNumber;

public:
    Token();
    Token(std::string value, int line, int col);
    static std::string getType(TokenType type);
    std::string getToken() const;
    std::string getValue() const;
    int getLineNumber() const;
    static const std::vector<std::string> keywords, dataTypes, operators, oneCharOperators, twoCharOperators;
};

class Lexer
{
private:
    std::vector<std::vector<Token>> tokens;
    void addTabsInLine(const std::string &line, int lineNum, int &col);
    int countSubstrings(const std::string &str, const std::string &substr) const;
    bool validNumber(const std::string &str) const;
    bool isNegativeNumber(const std::string &line, int col) const;
    bool startedNewString(const std::string &line, int col) const;
    bool stringHasntEnded(bool inString, const std::string &line, int col) const;
    bool commentStarted(const std::string &line, int col) const;
    bool isAOneCharOperator(const std::string &line, int col) const;
    bool isATwoCharOperator(const std::string &line, int col) const;
    void addCurrentToken(std::string &token, int lineNum, int col);

public:
    Lexer();
    void tokenizeFile(std::ifstream &file);
    void readTokens() const;
    void readFile() const;
    std::vector<std::vector<Token>> getTokens() const;
};
