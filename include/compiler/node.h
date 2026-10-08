#pragma once
#include <unordered_map>
#include "compiler/lexer.h"
#include "compiler/error.h"
#include "compiler/scope.h"

void throwLexicalError(int line, std::string message);
void throwSyntaxError(int line, std::string message);
void throwSemanticWarning(int line, std::string message);
void throwSemanticError(int line, std::string message);
std::string repeat_string(const std::string &input, size_t num);

struct Node
{
protected:
    int line;

public:
    Node();
    virtual ~Node() = default;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope);
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const;
    virtual std::string getIR();
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
    std::vector<Node *> &getChildrenRef();
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    void sendWarnings(std::unordered_map<std::string, Scope> symbolTable);
};

template <typename T>
struct LiteralNode : Node
{
private:
    std::string type;
    T value;

public:
    LiteralNode(T value, std::string type)
    {
        this->value = value;
        this->type = type;
    }

    ~LiteralNode()
    {
    }

    void print(int depth) const
    {
        std::string depthTabs = repeat_string("\t", depth);
        std::cout << depthTabs << "LiteralNode: " << value << std::endl;
    }

    bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
    {
        if (this->type == "CHAR" && type == "CHARTYPE")
            return true;
        if (this->type == "STRING" && type == "STRINGTYPE")
            return true;
        if (this->type == "INT" && type == "INTTYPE")
            return true;
        if (this->type == "DECIMAL" && type == "DECIMALTYPE")
            return true;
        if (this->type == "BOOL" && type == "BOOLTYPE")
            return true;

        return false;
    }

    std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
    {
        if (this->type == "CHAR")
            return "CHARTYPE";
        if (this->type == "STRING")
            return "STRINGTYPE";
        if (this->type == "INT")
            return "INTTYPE";
        if (this->type == "DECIMAL")
            return "DECIMALTYPE";
        if (this->type == "BOOL")
            return "BOOLTYPE";

        return "";
    }

    virtual std::string getIR()
    {
        return "";
    }
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
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual std::string getIR();
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
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual std::string getIR();
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
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual std::string getIR();
};

struct StmtNode : Node
{
private:
protected:
public:
    StmtNode();
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
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual std::string getIR();
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
};

struct ReturnStmtNode : StmtNode
{
private:
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier = nullptr);
    ~ReturnStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
};

struct BlockNode : Node
{
private:
    std::vector<Node *> statements;

public:
    BlockNode();
    ~BlockNode() override;
    void addStatement(Node *stmt);
    void clearStatments();
    int getNumStatements() const;
    std::vector<Node *> getStatements() const;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
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
    virtual std::string getIR();
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
};

struct ElseStmtNode : Node
{
private:
    BlockNode *body;

public:
    ElseStmtNode(BlockNode *body);
    ~ElseStmtNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string getIR();
};
