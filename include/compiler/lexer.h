#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <string>

enum class TokenType
{
    EXPORT,
    IMPORT,
    MODULE,
    AS,
    FUNC,
    REF,
    VAR,
    IF,
    ELSEIF,
    ELSE,
    FOR,
    WHILE,
    BREAK,
    SKIP,
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
    PLUSASSIGN,
    MINUSASSIGN,
    STARASSIGN,
    SLASHASSIGN,
    EXPASSIGN,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MODULUS,
    EXPONENT,
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
    DOUBLECOLON,
    ARROW,
    LPAREN,
    RPAREN,
    LCURLYBRACKET,
    RCURLYBRACKET,
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
    static TokenType getTokenDataType(std::string type);
    static std::string getType(TokenType type);
    std::string getToken() const;
    std::string getValue() const;
    int getLineNumber() const;
    static const std::vector<std::string> reservedFunctions, keywords, dataTypes, operators, oneCharOperators, twoCharOperators;
    static const std::vector<std::vector<std::string>> builtInFunctions;
    static const std::vector<char> escapeSequences;
};

class Lexer
{
private:
    int lineNum;
    std::string currentToken;
    bool inString, inImport;

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
    void tokenizeLine(const std::string &line);
    void tokenizeFile(const std::string &file);
    void tokenizeFile(std::ifstream &file);
    void readTokens() const;
    void readFile() const;
    std::vector<std::vector<Token>> getTokens() const;
};
