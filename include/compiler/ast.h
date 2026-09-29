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

struct IdentifierNode : Node
{
private:
    std::string name;
    std::string type;
    LiteralNode value;
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

struct ReturnStmtNode : StmtNode
{
private:
public:
};

struct BlockNode : Node
{
private:
    std::vector<StmtNode *> statements;
};

struct ImportNode : Node
{
private:
    std::string location;
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
    Node *body;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, Node *body);
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

struct VarDeclNode : StmtNode
{
private:
    IdentifierNode *identifier;
};

class AbstractSyntaxTree
{
private:
    RootNode *root;

public:
    AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens);
    int getNumTabs(const std::vector<Token> &tokenLine) const;
    Token getFirstToken(const std::vector<Token> &tokenLine) const;
    Node *buildTree(const std::vector<std::vector<Token>> &tokens);
    void printTree() const;
};
