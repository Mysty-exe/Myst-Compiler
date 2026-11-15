#pragma once
#include "compiler/node.h"
#include "compiler/lexer.h"

enum ExportState
{
    STANDBY,
    IN,
    OUT
};

class AbstractSyntaxTree
{
private:
    std::vector<ImportNode *> importNodes;
    std::unordered_map<std::string, Scope> symbolTable;
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> prefixSymbols;
    std::vector<std::string> prefixes, imports, exports;
    bool imported;

    RootNode *root;
    static const std::vector<std::vector<std::string>> operatorPrecedence;

    bool isDataType(const std::string &str) const;
    bool isValidToken(Token token) const;
    bool isValidLine(const std::vector<Token> &tokens) const;
    int getNumValidTokens(const std::vector<Token> &line) const;
    bool hasOperator(const std::vector<Token> &line, int start, int end) const;
    int getTokenIndex(const std::vector<Token> &tokenLine, const std::string &tokenStr, bool includeOnlyValidTokens = false) const;
    int getTokenIndex(const std::vector<Token> &tokenLine, const std::vector<std::string> &tokenStrs, bool includeOnlyValidTokens = false) const;
    int getNumTabs(const std::vector<Token> &tokenLine) const;
    Token getNthToken(const std::vector<Token> &tokenLine, int n) const;
    std::vector<std::string> getFunctionInformation(const std::vector<Token> &tokenLine) const;
    std::string splitLine(const std::vector<Token> &tokens, int startIndex, int endIndex) const;

public:
    AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens, std::string fileName);
    ~AbstractSyntaxTree();

    RootNode *getRoot() const;
    void format();
    void printTree() const;
    bool isImported() const;
    void setImported(bool imported);
    bool hasImports() const;
    std::vector<std::string> getPrefixes() const;
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> getPrefixSymbols() const;
    void setSymbols(std::string prefix, std::unordered_map<std::string, Scope> symbols);
    std::vector<std::string> getImports() const;
    void setImports(std::vector<std::string>);
    void freeMemory();

    std::unordered_map<std::string, Scope> getSymbolTable() const;
    void setSymbolTable(std::unordered_map<std::string, Scope> symbolTable);
    void buildExports(const std::vector<std::vector<Token>> &tokens, int &startLine);
    Node *buildLiteral(const std::vector<Token> &line, int start);
    Node *buildIdentifier(const std::vector<Token> &line, int start, int end);
    CallExprNode *buildCallExpr(const std::vector<Token> &line, int start, int end);
    Node *buildExpression(const std::vector<Token> &line, int start, int end);
    BranchBlockNode *getIfStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop);
    void addElseIfStatement(int index, BlockNode *blockNode, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop);
    void addElseStatement(int index, BlockNode *blockNode, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop);
    WhileNode *getWhileStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop);
    AssignNode *getAssignStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int tabs);
    AssignNode *getOperatorAssignStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int tabs);
    BlockNode *buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction = false, bool inLoop = false);
    void addImport(const std::vector<std::vector<Token>> &tokens, int &line);
    void addFunction(const std::vector<std::vector<Token>> &tokens, int &line);
    void buildTree(const std::vector<std::vector<Token>> &tokens);
};
