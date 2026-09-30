#pragma once
#include "lexer.h"

struct Node
{
public:
    Node();
    friend std::ostream &operator<<(std::ostream &os, Node const &node);

protected:
    virtual void print(std::ostream &os) const = 0;
};

struct RootNode : Node
{
private:
    std::vector<Node *> children;

protected:
    virtual void print(std::ostream &os) const override;

public:
    RootNode();
    void addNode(Node *node);
    std::vector<Node *> getChildren();
};

struct LiteralNode : Node
{
private:
    std::string value;

protected:
    virtual void print(std::ostream &os) const override;

public:
    LiteralNode(std::string value);
};

struct IdentifierNode : Node
{
private:
    std::string name;
    std::string type;

protected:
    virtual void print(std::ostream &os) const override;

public:
    IdentifierNode(std::string name);
    IdentifierNode(std::string name, std::string returnType);
};

struct BinaryExprNode : Node
{
private:
    std::string op;
    Node *left, *right;

protected:
    virtual void print(std::ostream &os) const override;

public:
    BinaryExprNode(std::string op, Node *left, Node *right);
};

struct UnaryExprNode : Node
{
private:
    std::string op;
    Node *right;

protected:
    virtual void print(std::ostream &os) const override;

public:
    UnaryExprNode(std::string op, Node *right);
};

struct StmtNode : Node
{
private:
protected:
    virtual void print(std::ostream &os) const override;

public:
};

struct AssignNode : StmtNode
{
private:
    IdentifierNode *identifier;
    Node *value;

protected:
    virtual void print(std::ostream &os) const override;

public:
    AssignNode(std::string name, std::string type, Node *value);
};

struct ReturnStmtNode : StmtNode
{
private:
    Node *identifier;

protected:
    virtual void print(std::ostream &os) const override;

public:
    ReturnStmtNode(Node *identifier);
};

struct BlockNode : Node
{
private:
    std::vector<Node *> statements;

protected:
    virtual void print(std::ostream &os) const override;

public:
    BlockNode();
    void addStatement(Node *stmt);
};

struct ParamNode : Node
{
private:
    std::string name, returnType;

protected:
    virtual void print(std::ostream &os) const override;

public:
    ParamNode(std::string identifier);
    ParamNode(std::string name, std::string returnType);
};

struct FuncNode : Node
{
private:
    std::string name, returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;

protected:
    virtual void print(std::ostream &os) const override;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body);
};

struct CallExprNode : Node
{
private:
    std::string name;
    std::vector<Node *> arguments;

protected:
    virtual void print(std::ostream &os) const override;

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
};

struct IfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

protected:
    virtual void print(std::ostream &os) const override;

public:
    IfStmtNode(Node *condition, BlockNode *body);
};

struct ElseIfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

protected:
    virtual void print(std::ostream &os) const override;

public:
    ElseIfStmtNode(Node *condition, BlockNode *body);
};

struct ElseNode : Node
{
private:
    BlockNode *body;

protected:
    virtual void print(std::ostream &os) const override;

public:
    ElseNode(BlockNode *body);
};

struct WhileNode : Node
{
private:
    Node *condition;
    BlockNode *body;

protected:
    virtual void print(std::ostream &os) const override;

public:
    WhileNode(Node *condition, BlockNode *body);
};

class AbstractSyntaxTree
{
private:
    RootNode *root;
    static const std::vector<std::vector<std::string>> operatorPrecedence;
    bool isValidToken(Token token) const;
    bool isValidLine(const std::vector<Token> &tokens) const;
    bool hasOperator(const std::vector<Token> &line, int start, int end) const;
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
