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
    virtual void loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
    virtual void print(int depth = 0) const = 0;
    int getLine() const;
    void setLine(int line);
};

struct RootNode : Node
{
private:
    std::vector<Node *> children;
    Node *globalCode;

public:
    RootNode();
    ~RootNode() override;
    void addNode(Node *node);
    Node *getGlobalCode();
    void setGlobalCode(Node *block);
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

    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap)
    {
        if constexpr (std::is_same_v<T, int>)
            ir += "store i32 " + std::to_string(value) + ", ptr @" + assignmentName + ", align 4\n";
        if constexpr (std::is_same_v<T, double>)
            ir += "store double " + std::to_string(value) + ", ptr @" + assignmentName + ", align 8\n";
        if constexpr (std::is_same_v<T, char>)
            ir += "store i1 " + std::to_string(static_cast<int>(value)) + ", ptr @" + assignmentName + ", align 1\n";
        if constexpr (std::is_same_v<T, bool>)
            ir += "store i1 " + std::to_string(static_cast<int>(value)) + ", ptr @" + assignmentName + ", align 1\n";
    }

    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false)
    {
        if constexpr (std::is_same_v<T, int>)
            ir += std::to_string(value);
        if constexpr (std::is_same_v<T, double>)
            ir += std::to_string(value);
        if constexpr (std::is_same_v<T, char>)
            ir += std::to_string(static_cast<int>(value));
        if constexpr (std::is_same_v<T, bool>)
            ir += std::to_string(static_cast<int>(value));
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
    virtual void loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
};

struct BinaryExprNode : Node
{
private:
    std::string op;
    Node *left, *right;
    std::string resultType, operandType;

public:
    BinaryExprNode(std::string op, Node *left, Node *right);
    ~BinaryExprNode() override;
    void traverseBinary(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap);
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual void loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
};

struct UnaryExprNode : Node
{
private:
    std::string op;
    Node *right;
    std::string resultType, operandType;

public:
    UnaryExprNode(std::string op, Node *right);
    ~UnaryExprNode() override;
    void traverseUnary(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap);
    virtual void print(int depth = 0) const override;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const override;
    virtual void loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::string>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
};

struct AssignNode : StmtNode
{
private:
    IdentifierNode *identifier;
    Node *value;
    bool declared;

public:
    AssignNode(std::string name, std::string type, Node *value);
    std::string getName() const;
    std::string getType() const;
    bool isDeclared() const;
    ~AssignNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    std::string mapToGlobalIR();
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
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
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::string>> &tempvarMap, bool global = false);
};
