#pragma once
#include "lexer.h"

struct Node
{
public:
    Node();
};

struct RootNode : Node
{
private:
    std::vector<Node *> children;

public:
    RootNode();
    void addNode(Node *node);
};

struct LiteralNode : Node
{
private:
    std::string value;
};

struct BinaryExprNode : Node
{
private:
    Node *left, *right;

public:
};

struct StmtNode : Node
{
private:
public:
};

struct AssignNode : StmtNode
{
private:
    std::string name;
    std::string type;
    Node *value;

public:
    AssignNode(std::string name, std::string type, Node *value);
};

struct ReturnStmtNode : StmtNode
{
private:
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier);
};

struct BlockNode : Node
{
private:
    std::vector<StmtNode *> statements;

public:
    BlockNode();
    void addStatement(StmtNode *stmt);
};

struct ParamNode : Node
{
private:
    std::string name, returnType;

public:
    ParamNode(std::string name, std::string returnType);
};

struct FuncNode : Node
{
private:
    std::string name, returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body);
};

struct CallExprNode : Node
{
private:
    std::string name;
    std::vector<Node *> arguments;
};

struct IfStmtNode : Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

struct ElseIfStmtNode : Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

struct ElseNode : Node
{
private:
    BlockNode *body;
};

struct WhileNode : Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

class AbstractSyntaxTree
{
private:
    RootNode *root;
    bool isValidToken(Token token) const;
    bool isValidLine(const std::vector<Token> &tokens) const;
    int getTokenIndex(const std::vector<Token> &tokenLine, const std::string &tokenStr, bool includeOnlyValidTokens = false) const;
    int getNumTabs(const std::vector<Token> &tokenLine) const;
    Token getNthToken(const std::vector<Token> &tokenLine, int n) const;
    int getNextDedentedLine(const std::vector<std::vector<Token>> &tokens, int startLine) const;
    std::vector<std::string> getFunctionInformation(const std::vector<Token> &tokenLine) const;
    std::string splitLine(const std::vector<Token> &tokens, int startIndex, int endIndex) const;

public:
    AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens);
    Node *buildIdentifier(const std::vector<Token> &line, int start, int end);
    BlockNode *buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine);
    void buildTree(const std::vector<std::vector<Token>> &tokens);
    void printTree() const;
};
