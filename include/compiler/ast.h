#pragma once
#include "lexer.h"
#include "error.h"

struct Node
{
protected:
    int line;

public:
    Node();
    virtual ~Node() = default;
    virtual void print(int depth = 0) const = 0;
    int getLine() const;
    void setLine(int line);
};

struct RootNode : Node
{
private:
    std::vector<Node *> children;

public:
    RootNode();
    void addNode(Node *node);
    std::vector<Node *> getChildren();
    virtual void print(int depth = 0) const override;
};

struct LiteralNode : Node
{
private:
    std::string value;

public:
    LiteralNode(std::string value);
    virtual void print(int depth = 0) const override;
};

struct IdentifierNode : Node
{
private:
    std::string name;
    std::string type;

public:
    IdentifierNode(std::string name);
    IdentifierNode(std::string name, std::string returnType);
    std::string getName() const;
    std::string getType() const;
    virtual void print(int depth = 0) const override;
};

struct BinaryExprNode : Node
{
private:
    std::string op;
    Node *left, *right;

public:
    BinaryExprNode(std::string op, Node *left, Node *right);
    virtual void print(int depth = 0) const override;
};

struct UnaryExprNode : Node
{
private:
    std::string op;
    Node *right;

public:
    UnaryExprNode(std::string op, Node *right);
    virtual void print(int depth = 0) const override;
};

struct StmtNode : Node
{
private:
protected:
public:
    virtual void print(int depth = 0) const override;
};

struct AssignNode : StmtNode
{
private:
    IdentifierNode *identifier;
    Node *value;

public:
    AssignNode(std::string name, std::string type, Node *value);
    virtual void print(int depth = 0) const override;
};

struct ReturnStmtNode : StmtNode
{
private:
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier);
    virtual void print(int depth = 0) const override;
};

struct BlockNode : Node
{
private:
    std::vector<Node *> statements;

public:
    BlockNode();
    void addStatement(Node *stmt);
    int getNumStatements() const;
    std::vector<Node *> getStatements() const;
    virtual void print(int depth = 0) const override;
};

struct ParamNode : Node
{
private:
    std::string name, returnType;

public:
    ParamNode(std::string identifier);
    ParamNode(std::string name, std::string returnType);
    virtual void print(int depth = 0) const override;
};

struct FuncNode : Node
{
private:
    std::string name, returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body);
    virtual void print(int depth = 0) const override;
};

struct CallExprNode : Node
{
private:
    std::string name;
    std::vector<Node *> arguments;

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
    virtual void print(int depth = 0) const override;
};

struct IfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    IfStmtNode(Node *condition, BlockNode *body);
    virtual void print(int depth = 0) const override;
};

struct ElseIfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    ElseIfStmtNode(Node *condition, BlockNode *body);
    virtual void print(int depth = 0) const override;
};

struct ElseNode : Node
{
private:
    BlockNode *body;

public:
    ElseNode(BlockNode *body);
    virtual void print(int depth = 0) const override;
};

struct WhileNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    WhileNode(Node *condition, BlockNode *body);
    virtual void print(int depth = 0) const override;
};

class AbstractSyntaxTree
{
private:
    RootNode *root;
    static const std::vector<std::vector<std::string>> operatorPrecedence;
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
