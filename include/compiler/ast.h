#pragma once
#include "semantic_analyzer.h"
#include "lexer.h"
#include "error.h"

struct Node
{
protected:
    int line;

public:
    Node();
    virtual ~Node();
    virtual ~Node() = default;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope);
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const;
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
    ~RootNode() override;
    void addNode(Node *node);
    std::vector<Node *> getChildren();
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
    void sendWarnings(std::unordered_map<std::string, Scope> symbolTable);
};

template <typename T>
struct LiteralNode : Node
{
private:
    std::string type;
    T value;

public:
    LiteralNode(T value, std::string type);
    ~LiteralNode() override;
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const override;
};

struct IdentifierNode : Node
{
private:
    std::string name;
    TokenType type;

public:
    IdentifierNode(std::string name);
    IdentifierNode(std::string name, std::string returnType);
    ~IdentifierNode() override;
    std::string getName() const;
    TokenType getType() const;
    void setName(std::string name);
    void setType(TokenType type);
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const override;
};

struct BinaryExprNode : Node
{
private:
    std::string op;
    Node *left, *right;

public:
    BinaryExprNode(std::string op, Node *left, Node *right);
    ~BinaryExprNode() override;
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const override;
};

struct UnaryExprNode : Node
{
private:
    std::string op;
    Node *right;

public:
    UnaryExprNode(std::string op, Node *right);
    ~UnaryExprNode() override;
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const override;
};

struct StmtNode : Node
{
private:
protected:
public:
    ~StmtNode() override;
    virtual void print(int depth = 0) const override;
};

struct CallExprNode : StmtNode
{
private:
    std::string name;
    std::vector<Node *> arguments;

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
    ~CallExprNode() override;
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) const override;
};

struct AssignNode : StmtNode
{
private:
    IdentifierNode *identifier;
    Node *value;

public:
    AssignNode(std::string name, std::string type, Node *value);
    ~AssignNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct ReturnStmtNode : StmtNode
{
private:
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier = nullptr);
    ~ReturnStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct BlockNode : Node
{
private:
    std::vector<Node *> statements;

public:
    BlockNode();
    ~BlockNode() override;
    void addStatement(Node *stmt);
    int getNumStatements() const;
    std::vector<Node *> getStatements() const;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct ParamNode : Node
{
private:
    TokenType type;
    std::string name;

public:
    ParamNode(std::string identifier);
    ParamNode(std::string name, std::string type);
    ~ParamNode() override;
    std::string getName() const;
    TokenType getType() const;
    virtual void print(int depth = 0) const override;
};

struct FuncNode : Node
{
private:
    std::string name;
    TokenType returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body);
    ~FuncNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct IfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    IfStmtNode(Node *condition, BlockNode *body);
    ~IfStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct ElseIfStmtNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    ElseIfStmtNode(Node *condition, BlockNode *body);
    ~ElseIfStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct ElseStmtNode : Node
{
private:
    BlockNode *body;

public:
    ElseStmtNode(BlockNode *body);
    ~ElseStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

struct WhileNode : Node
{
private:
    Node *condition;
    BlockNode *body;

public:
    WhileNode(Node *condition, BlockNode *body);
    ~WhileNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> symbolTable, std::vector<std::string> &visibleScope) override;
};

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
