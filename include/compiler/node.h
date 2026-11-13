#pragma once
#include <unordered_map>
#include "compiler/lexer.h"
#include "compiler/error.h"
#include "compiler/scope.h"

[[noreturn]] void throwLexicalError(int line, std::string message);
[[noreturn]] void throwSyntaxError(int line, std::string message);
void throwSemanticWarning(int line, std::string message);
[[noreturn]] void throwSemanticError(int line, std::string message);
std::string repeat_string(const std::string &input, size_t num);

struct Table
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> prefixSymbols;
    std::vector<std::string> visibleScope;

    std::unordered_map<std::string, Scope> getSymbolTable()
    {
        return symbolTable;
    }
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> getPrefixSymbols()
    {
        return prefixSymbols;
    }
    void setPrefixSymbols(std::unordered_map<std::string, std::unordered_map<std::string, Scope>> prefixes)
    {
        this->prefixSymbols = prefixes;
    }
    void addVisibleScope()
    {
        visibleScope.push_back(std::to_string(visibleScope.size() + 1));
    }
    std::string getLatestVisibleScope()
    {
        return getVisibleScope()[getVisibleScope().size() - 1];
    }
    void removeVisibleScope()
    {
        visibleScope.pop_back();
    }
    std::vector<std::string> getVisibleScope()
    {
        return visibleScope;
    }
    Scope getScope(std::string scope)
    {
        return symbolTable[scope];
    }
    void setScope(std::string visibleScope, Scope scope)
    {
        symbolTable[visibleScope] = scope;
    }
    void removeScope()
    {
        symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    }
    void addSymbol(std::string scope, Symbol *symbol)
    {
        symbolTable[scope].addSymbol(symbol);
    }
};

struct TemporaryVariables
{
    std::vector<std::vector<std::vector<std::string>>> tempvarMap;
    std::unordered_map<std::string, std::string> prefixImports;

    TemporaryVariables()
    {
        tempvarMap = {{}};
    }

    void addPrefix(std::string key, std::string import)
    {
        prefixImports[key] = import;
    }
};

struct Node
{
protected:
    int line;

public:
    Node();
    virtual ~Node() = default;
    virtual void lookForCallExprs(Table &table);
    virtual bool checkType(Table &table, const std::string &type);
    virtual void checkSemantics(Table &table);
    virtual std::string inferType(Table &table);
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global = false, std::string prefix = "");
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "");
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "");
    virtual void findReturnStmts(std::vector<Node *> &stmts);
    virtual void print(int depth = 0) const = 0;
    int getLine() const;
    void setLine(int line);
};

struct RootNode : Node
{
private:
    std::string fileName, fileNameWDots;
    std::vector<Node *> children;
    Node *globalCode;

protected:
    static std::string currentFile;

public:
    RootNode();
    ~RootNode() override;
    static std::string getCurrentFile();
    std::string getFileName();
    std::string getFileNameWDots();
    void setFileName(std::string name);
    static void setCurrentFile(std::string name);
    void addNode(Node *node);
    Node *getGlobalCode();
    void setGlobalCode(Node *block);
    std::vector<Node *> getChildren();
    std::vector<Node *> &getChildrenRef();
    virtual void checkSemantics(Table &table) override;
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

    bool checkType(Table &table, const std::string &type)
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

    virtual std::string inferType(Table &table)
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

    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "")
    {
        std::string localOrGlobal;
        std::string name = assignmentName;

        if (global)
            localOrGlobal = "@";
        else
        {
            bool globalVar = true;
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == assignmentName && tempvarMap[i][j][1] == "ptr")
                    {
                        globalVar = false;
                        localOrGlobal = "%";
                        name = std::to_string(j + 1);
                        break;
                    }
                }

                if (!globalVar)
                    break;
            }

            if (globalVar)
                localOrGlobal = "@";
        }

        if (name == ".return")
            localOrGlobal = "%";

        if (localOrGlobal == "@")
            name = prefix + name;

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

    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "")
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

struct ImportNode : Node
{
private:
    std::string fileName, moduleName;

public:
    ImportNode(std::string fileName, std::string moduleName);
    ~ImportNode() override;
    virtual void print(int depth = 0) const override;
};

struct IdentifierNode : Node
{
private:
    std::string name, modName;
    TokenType type;
    bool imported;

public:
    IdentifierNode(std::string name);
    IdentifierNode(std::string name, std::string returnType);
    ~IdentifierNode() override;
    std::string getName() const;
    TokenType getType() const;
    void setName(std::string name);
    void setType(TokenType type);
    std::string getModName() const;
    bool isImported();
    void setImported(std::string modName);
    virtual void print(int depth = 0) const override;
    virtual bool checkType(Table &table, const std::string &type) override;
    virtual std::string inferType(Table &table) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    std::string getOperandType() const;
    std::string getResultType() const;
    void traverseBinary(std::string &ir, std::string &newType, TemporaryVariables &tempVars, bool global, std::string prefix = "");
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(Table &table) override;
    virtual bool checkType(Table &table, const std::string &type) override;
    virtual std::string inferType(Table &table) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    std::string getOperandType() const;
    std::string getResultType() const;
    void traverseUnary(std::string &ir, std::string &newType, TemporaryVariables &tempVars, bool global, std::string prefix = "");
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(Table &table) override;
    virtual bool checkType(Table &table, const std::string &type) override;
    virtual std::string inferType(Table &table) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};

struct CallExprNode : Node
{
private:
    std::string name, returnType, modName;
    std::vector<std::string> argTypes;
    std::vector<Node *> arguments;
    bool imported;

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
    ~CallExprNode() override;
    std::string getName() const;
    std::string getReturnType() const;
    std::string getModName() const;
    void setImported(std::string modName);
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(Table &table) override;
    virtual bool checkType(Table &table, const std::string &type) override;
    virtual void checkSemantics(Table &table) override;
    virtual std::string inferType(Table &table) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};

struct AssignNode : Node
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
    virtual void lookForCallExprs(Table &table) override;
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};

struct ReturnStmtNode : Node
{
private:
    std::string returnType;
    Node *identifier;

public:
    ReturnStmtNode(Node *identifier = nullptr);
    ~ReturnStmtNode() override;
    void setType(std::string type);
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(Table &table) override;
    void mapToGlobalIR(std::string &ir, std::string prefix);
    void mapImportsToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::vector<std::string> imports);
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual void print(int depth = 0) const override;
};

struct ParamNode : Node
{
private:
    TokenType type;
    std::string name;
    bool reference;

public:
    ParamNode(std::string identifier, int line);
    ParamNode(std::string name, std::string type, int line);
    ~ParamNode() override;
    std::string getName() const;
    TokenType getType() const;
    bool isReference() const;
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};

struct FuncNode : Node
{
private:
    std::string name;
    TokenType returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;
    bool emptyFunction;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line);
    ~FuncNode() override;
    virtual void print(int depth = 0) const override;
    void addSignatureToTable(Table &table, bool includeParams = true);
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    void mapCondition(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix = "");
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    void mapCondition(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix = "");
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(Table &table) override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct BreakNode : Node
{
private:
    int currentWhileStmt;

public:
    BreakNode(int line);
    ~BreakNode() override;
    void setCurrentWhile(int currentStmt);
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};

struct SkipNode : Node
{
private:
    int currentWhileStmt;

public:
    SkipNode(int line);
    ~SkipNode() override;
    void setCurrentWhile(int currentStmt);
    virtual void print(int depth = 0) const override;
    virtual void mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global = false, std::string prefix = "") override;
};
