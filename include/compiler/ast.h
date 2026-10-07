#pragma once
#include "semantic_analyzer.h"
#include "lexer.h"
#include "error.h"

class AbstractSyntaxTree
{
private:
    RootNode *root;
    static const std::vector<std::vector<std::string>> operatorPrecedence;
    bool isDataType(const std::string &str) const;
    bool isValidToken(Token token) const;
    bool isValidLine(const std::vector<Token> &tokens) const;
    int getNumValidTokens(const std::vector<Token> &line) const;
    bool hasOperator(const std::vector<Token> &line, int start, int end) const;
    int getTokenIndex(const std::vector<Token> &tokenLine, const std::string &tokenStr, bool includeOnlyValidTokens = false) const;
    int getNumTabs(const std::vector<Token> &tokenLine) const;
    Token getNthToken(const std::vector<Token> &tokenLine, int n) const;
    std::vector<std::string> getFunctionInformation(const std::vector<Token> &tokenLine) const;
    std::string splitLine(const std::vector<Token> &tokens, int startIndex, int endIndex) const;

public:
    AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens);
    RootNode *getRoot() const;
    Node *buildIdentifier(const std::vector<Token> &line, int start, int end);
    BlockNode *buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction = false);
    void buildTree(const std::vector<std::vector<Token>> &tokens);
    void printTree() const;
};
