#pragma once
#include <unordered_map>
#include "compiler/lexer.h"
#include "compiler/error.h"
#include "compiler/scope.h"

struct SemanticsInfo
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

struct LLVMInfo
{
    std::vector<std::vector<std::vector<std::string>>> tempVarMap;
    std::unordered_map<std::string, std::string> prefixImports;

    LLVMInfo()
    {
        tempVarMap = {{}};
    }

    std::vector<std::vector<std::vector<std::string>>> getTempVars()
    {
        return tempVarMap;
    }

    void addScope()
    {
        tempVarMap.push_back({});
    }

    void removeScope()
    {
        tempVarMap.pop_back();
    }

    int getPreviousTempVar()
    {
        return tempVarMap[tempVarMap.size() - 1].size() - 1;
    }

    int getCurrentTempVar()
    {
        return tempVarMap[tempVarMap.size() - 1].size();
    }

    int getNextTempVar()
    {
        return tempVarMap[tempVarMap.size() - 1].size() + 1;
    }

    void addTempVar(std::vector<std::string> x)
    {
        tempVarMap[tempVarMap.size() - 1].push_back(x);
    }

    void addPrefix(std::string key, std::string import)
    {
        prefixImports[key] = import;
    }

    std::string getImportName(std::string key)
    {
        return prefixImports[key];
    }
};

[[noreturn]] void throwLexicalError(int line, std::string message);
[[noreturn]] void throwSyntaxError(int line, std::string message);
void throwSemanticWarning(int line, std::string message);
[[noreturn]] void throwSemanticError(int line, std::string message);
std::string repeat_string(const std::string &input, size_t num);
std::string getDataTypeStr(const std::string &str);
std::string mapTypeIR(const std::string &type);
std::string mapNumBytesIR(const std::string &type);
std::string mapOperatorIR(const std::string &op, const std::string &type);
bool canCoerce(std::string origType, std::string newType);
bool checkTempVarInScope(std::string currentTempVar, int &outOfScope);
void coerceValue(std::string &ir, LLVMInfo &llvmInfo, std::string name, std::string newTempName, std::string oldTempName, std::string origType, std::string newType, bool isExpression = false);

struct Node
{
protected:
    int line;

public:
    Node();
    virtual ~Node() = default;
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo);
    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type);
    virtual void checkSemantics(SemanticsInfo &semanticsInfo);
    virtual std::string inferType(SemanticsInfo &semanticsInfo);
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "");
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "");
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "");
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
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
    LiteralNode(T value, std::string type);
    ~LiteralNode();
    void print(int depth) const;

    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type) override;
    virtual std::string inferType(SemanticsInfo &semanticsInfo) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    std::string name, modName, type;
    bool imported;

public:
    IdentifierNode(std::string name);
    IdentifierNode(std::string name, std::string returnType);
    ~IdentifierNode() override;
    std::string getName() const;
    std::string getType() const;
    void setName(std::string name);
    void setType(std::string type);
    std::string getModName() const;
    bool isImported();
    void setImported(std::string modName);
    virtual void print(int depth = 0) const override;
    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type) override;
    virtual std::string inferType(SemanticsInfo &semanticsInfo) override;
    virtual void loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
};

struct BinaryExprNode : Node
{
private:
    std::string op;
    Node *left, *right;
    std::string resultType, operandType;

    void traverseBinaryBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseBinaryExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseExprBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseExrpExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);

public:
    BinaryExprNode(std::string op, Node *left, Node *right);
    ~BinaryExprNode() override;
    std::string getOperandType() const;
    std::string getResultType() const;
    void traverseBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo) override;
    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type) override;
    virtual std::string inferType(SemanticsInfo &semanticsInfo) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
};

struct UnaryExprNode : Node
{
private:
    std::string op;
    Node *right;
    std::string resultType, operandType;

    void traverseBinaryOrUnary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseCallExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseIdentifier(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseLiteral(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");

public:
    UnaryExprNode(std::string op, Node *right);
    ~UnaryExprNode() override;
    std::string getOperandType() const;
    std::string getResultType() const;
    void traverseUnary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo) override;
    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type) override;
    virtual std::string inferType(SemanticsInfo &semanticsInfo) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
};

struct CallExprNode : Node
{
private:
    std::string name, returnType, modName;
    std::vector<std::string> argTypes;
    std::vector<Node *> arguments;
    bool imported;

    int checkCallExprInScope(Scope scope, SemanticsInfo semanticsInfo, std::string type, bool &overridenFunction, bool importing);

public:
    CallExprNode(std::string name, std::vector<Node *> arguments);
    ~CallExprNode() override;
    std::string getName() const;
    std::string getReturnType() const;
    std::string getModName() const;
    void setImported(std::string modName);
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo) override;
    virtual bool checkType(SemanticsInfo &semanticsInfo, const std::string &type) override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual std::string inferType(SemanticsInfo &semanticsInfo) override;
    virtual void storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo) override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    void mapToGlobalIR(std::string &ir, std::string prefix);
    void mapImportsToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::vector<std::string> imports);
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual void print(int depth = 0) const override;
};

struct ParamNode : Node
{
private:
    std::string type;
    std::string name;
    bool reference;

public:
    ParamNode(std::string identifier, int line);
    ParamNode(std::string name, std::string type, int line);
    ~ParamNode() override;
    std::string getName() const;
    std::string getType() const;
    bool isReference() const;
    virtual void print(int depth = 0) const override;
};

struct FuncNode : Node
{
private:
    std::string name;
    std::string returnType;
    std::vector<ParamNode *> parameters;
    BlockNode *body;
    bool emptyFunction;

public:
    FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line);
    ~FuncNode() override;
    std::string getName();
    std::string getReturnType();
    void setReturnType(std::string type);
    virtual void print(int depth = 0) const override;
    void addSignatureToTable(SemanticsInfo &semanticsInfo, bool includeParams = true);
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct ContainerNode : Node
{
private:
    std::string name;
    std::vector<Node *> body;

public:
    ContainerNode(std::string name, std::vector<Node *> body, int line);
    ~ContainerNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
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
    void mapCondition(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    void mapCondition(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
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
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
};
