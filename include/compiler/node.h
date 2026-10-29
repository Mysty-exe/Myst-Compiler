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
    virtual void lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope);
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope);
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope);
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void findReturnStmts(std::vector<Node *> &stmts);
    virtual void print(int depth = 0) const = 0;
    int getLine() const;
    void setLine(int line);
};

struct RootNode : Node
{
private:
    std::vector<Node *> children;
    Node *globalCode;

protected:
    static std::string fileName;

public:
    RootNode();
    ~RootNode() override;
    static std::string getFileName();
    static void setFileName(std::string name);
    void addNode(Node *node);
    Node *getGlobalCode();
    void setGlobalCode(Node *block);
    std::vector<Node *> getChildren();
    std::vector<Node *> &getChildrenRef();
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void print(int depth = 0) const override;
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

    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
    {
        if constexpr (std::is_same_v<T, int>)
            return "INTTYPE";
        if constexpr (std::is_same_v<T, double>)
            return "DECIMALTYPE";
        if constexpr (std::is_same_v<T, char>)
            return "CHARTYPE";
        if constexpr (std::is_same_v<T, bool>)
            return "BOOLTYPE";

        return "";
    }

    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false)
    {
        std::string localOrGlobal;
        std::string name = assignmentName;

        if (global)
            localOrGlobal = "@";
        else
        {
            bool globalVar = true;
            for (int i = 0; i < tempvarMap.size(); i++)
            {
                for (int j = 0; j < tempvarMap[i].size(); j++)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == assignmentName)
                    {
                        globalVar = false;
                        localOrGlobal = "%";
                        name = std::to_string(j + 1);
                        break;
                    }
                }
            }

            if (globalVar)
                localOrGlobal = "@";
        }

        if (name == ".return")
            localOrGlobal = "%";

        if constexpr (std::is_same_v<T, int>)
        {
            if (newType == "DECIMALTYPE")
                ir += "store double " + std::to_string(static_cast<double>(value)) + ", ptr " + localOrGlobal + name + ", align 8\n";
            else if (newType == "CHARTYPE")
                ir += "store i8 " + std::to_string(static_cast<char>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
            else if (newType == "BOOLTYPE")
                ir += "store i1 " + std::to_string(static_cast<bool>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
            else
                ir += "store i32 " + std::to_string(value) + ", ptr " + localOrGlobal + name + ", align 4\n";
        }
        if constexpr (std::is_same_v<T, double>)
        {
            if (newType == "INTTYPE")
                ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
            else
                ir += "store double " + std::to_string(value) + ", ptr " + localOrGlobal + name + ", align 8\n";
        }
        if constexpr (std::is_same_v<T, char>)
            if (newType == "INTTYPE")
                ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
            else
                ir += "store i8 " + std::to_string(static_cast<char>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
        if constexpr (std::is_same_v<T, bool>)
            if (newType == "INTTYPE")
                ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
            else
                ir += "store i1 " + std::to_string(static_cast<bool>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
    }

    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false)
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
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
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
    void traverseBinary(std::string &ir, std::string &newType, std::vector<std::vector<std::vector<std::string>>> &tempvarMap);
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
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
    void traverseUnary(std::string &ir, std::string &newType, std::vector<std::vector<std::vector<std::string>>> &tempvarMap);
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false);
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
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
    std::string name, returnType;
    std::vector<std::string> argTypes;
    std::vector<Node *> arguments;

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
    ~CallExprNode() override;
    std::string getName() const;
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual bool checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type) override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual std::string inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
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
    Node *getValue() const;
    bool isDeclared() const;
    ~AssignNode() override;
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
};

struct ReturnStmtNode : StmtNode
{
private:
    std::string returnType;
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier = nullptr);
    ~ReturnStmtNode() override;
    void setType(std::string type);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual void print(int depth = 0) const override;
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
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    void mapToGlobalIR(std::string &ir);
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual void print(int depth = 0) const override;
};

struct ParamNode : Node
{
private:
    TokenType type;
    std::string name;

public:
    ParamNode(std::string identifier, int line);
    ParamNode(std::string name, std::string type, int line);
    ~ParamNode() override;
    std::string getName() const;
    TokenType getType() const;
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
};

struct FuncNode : Node
{
private:
    std::string name;
    TokenType returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line);
    ~FuncNode() override;
    virtual void print(int depth = 0) const override;
    void addSignatureToTable(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, bool includeParams = true);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
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
    void setCurrentWhile(int currentStmt);
    bool endsWithBranchStmt();
    void mapCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    void mapToIRInLoop(Node *loop, std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global);
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
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
    void setCurrentWhile(int currentStmt);
    bool endsWithBranchStmt();
    void mapCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global);
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct ElseStmtNode : Node
{
private:
    BlockNode *body;

public:
    ElseStmtNode(BlockNode *body);
    ~ElseStmtNode() override;
    virtual void print(int depth = 0) const override;
    void setCurrentWhile(int currentStmt);
    bool endsWithBranchStmt();
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct BranchBlockNode : Node
{
private:
    static int numIfStmt;
    BlockNode *branches;

public:
    BranchBlockNode();
    ~BranchBlockNode() override;
    static int getNumberIf();
    void setCurrentWhile(int currentStmt);
    void addBranch(Node *branch);
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    void mapToIRInLoop(Node *loop, std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global);
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct WhileNode : Node
{
private:
    static int numWhileStmt;
    Node *condition;
    BlockNode *body;
    bool skip;

public:
    WhileNode(Node *condition, BlockNode *body);
    ~WhileNode() override;
    bool getSkip();
    void setSkip(bool skip);
    virtual void print(int depth = 0) const override;
    bool endsWithBranchStmt();
    virtual void checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
    void checkCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global, int currentWhile, std::string loadTemp);
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct BreakNode : Node
{
private:
    int currentWhileStmt;

public:
    BreakNode();
    ~BreakNode() override;
    void setCurrentWhile(int currentStmt);
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
};

struct SkipNode : Node
{
private:
    int currentWhileStmt;

public:
    SkipNode();
    ~SkipNode() override;
    void setCurrentWhile(int currentStmt);
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global = false) override;
};
