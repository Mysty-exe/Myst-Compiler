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
    std::vector<Token> getTokens() const;
};
