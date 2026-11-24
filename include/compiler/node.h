#pragma once
#include <unordered_map>
#include <memory>
#include "compiler/lexer.h"
#include "compiler/error.h"
#include "compiler/scope.h"

struct SemanticsInfo
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> prefixSymbols;
    std::vector<std::string> visibleScope;

    SemanticsInfo(std::unordered_map<std::string, Scope> &symbolTable, std::unordered_map<std::string, std::unordered_map<std::string, Scope>> &prefixSymbols)
    {
        this->symbolTable = std::move(symbolTable);
        this->prefixSymbols = std::move(prefixSymbols);
    }

    std::unordered_map<std::string, Scope> &getSymbolTable()
    {
        return symbolTable;
    }
    std::unordered_map<std::string, std::unordered_map<std::string, Scope>> &getPrefixSymbols()
    {
        return prefixSymbols;
    }
    void setPrefixSymbols(std::unordered_map<std::string, std::unordered_map<std::string, Scope>> &prefixes)
    {
        this->prefixSymbols = std::move(prefixes);
    }
    void addVisibleScope()
    {
        visibleScope.push_back(std::to_string(visibleScope.size() + 1));
    }
    std::string getLatestVisibleScope()
    {
        return visibleScope.back();
    }
    void removeVisibleScope()
    {
        visibleScope.pop_back();
    }
    std::vector<std::string> &getVisibleScope()
    {
        return visibleScope;
    }
    Scope &getScope(std::string &scope)
    {
        return symbolTable[scope];
    }
    void setScope(std::string visibleScope, Scope scope)
    {
        symbolTable[visibleScope] = std::move(scope);
    }
    void removeScope()
    {
        if (!visibleScope.empty())
        {
            symbolTable[visibleScope.back()].getSymbols().clear();
        }
    }
    void addSymbol(std::string scope, std::unique_ptr<Symbol> symbol)
    {
        symbolTable[scope].addSymbol(std::move(symbol));
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
bool canCoerceLossless(std::string origType, std::string newType);
bool canCoerceLossy(std::string origType, std::string newType);
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
    virtual bool returnValueInPath();
    virtual void print(int depth = 0) const = 0;
    int getLine() const;
    void setLine(int line);
};

struct RootNode : Node
{
private:
    std::string fileName, fileNameWDots;
    std::vector<std::unique_ptr<Node>> children;
    std::unique_ptr<Node> globalCode;

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
    void addNode(std::unique_ptr<Node> node);
    std::unique_ptr<Node> &getGlobalCode();
    void setGlobalCode(std::unique_ptr<Node> block);
    std::vector<std::unique_ptr<Node>> &getChildren();
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void print(int depth = 0) const override;
    void sendWarnings(std::unordered_map<std::string, Scope> &symbolTable);
};

template <typename T>
struct LiteralNode : Node
{
private:
    std::string type;
    T value;

public:
    LiteralNode(T value, std::string type, int line);
    ~LiteralNode() override;
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
    IdentifierNode(std::string name, int line);
    IdentifierNode(std::string name, std::string returnType, int line);
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
    std::unique_ptr<Node> left, right;
    std::string resultType, operandType;

    void traverseBinaryBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseBinaryExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseExprBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);
    void traverseExrpExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix);

public:
    BinaryExprNode(std::string op, std::unique_ptr<Node> left, std::unique_ptr<Node> right, int line);
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
    std::unique_ptr<Node> right;
    std::string resultType, operandType;

    void traverseBinaryOrUnary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseCallExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseIdentifier(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");
    void traverseLiteral(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix = "");

public:
    UnaryExprNode(std::string op, std::unique_ptr<Node> right, int line);
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
    std::vector<std::unique_ptr<Node>> arguments;
    bool imported;

    std::vector<int> checkFuncArgs(SemanticsInfo &semanticsInfo, FuncSymbol *&funcSymbol);
    void checkCallExprInScope(Scope &scope, SemanticsInfo &semanticsInfo, std::string type, FuncSymbol *&funcSymbol, std::vector<int> &funcArgs, bool &ambiguous, bool importing);
    bool validFunc(std::vector<int> current);
    bool betterFunc(std::vector<int> current, std::vector<int> next);
    bool isAmbiguousFunc(std::vector<int> current, std::vector<int> next);

public:
    CallExprNode(std::string name, std::vector<std::unique_ptr<Node>> &arguments, int line);
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
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<Node> value;
    bool declared;

public:
    AssignNode(std::string name, std::string type, std::unique_ptr<Node> &value, int line);
    ~AssignNode() override;
    std::string getName() const;
    std::string getType() const;
    std::unique_ptr<Node> &getValue();
    bool isDeclared() const;
    virtual void print(int depth = 0) const override;
    virtual void lookForCallExprs(SemanticsInfo &semanticsInfo) override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
};

struct ReturnStmtNode : Node
{
private:
    std::string returnType;
    std::unique_ptr<Node> identifier;

public:
    ReturnStmtNode(int line, std::unique_ptr<Node> identifier = nullptr);
    ~ReturnStmtNode() override;
    void setType(std::string type);
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual bool returnValueInPath();
    virtual void print(int depth = 0) const override;
};

struct BlockNode : Node
{
private:
    std::vector<std::unique_ptr<Node>> statements;

public:
    BlockNode(int line);
    ~BlockNode() override;
    void addStatement(std::unique_ptr<Node> stmt);
    void clearStatments();
    int getNumStatements() const;
    std::vector<std::unique_ptr<Node>> &getStatements();
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    void mapToGlobalIR(std::string &ir, std::string prefix);
    void mapImportsToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::vector<std::string> imports);
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual bool returnValueInPath();
    virtual void print(int depth = 0) const override;
};

struct ParamNode : Node
{
private:
    std::string type;
    std::string name;
    bool reference;

public:
    ParamNode(std::vector<std::string> identifier, int line);
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
    std::vector<std::unique_ptr<ParamNode>> parameters;
    std::unique_ptr<BlockNode> body;
    bool emptyFunction;

public:
    FuncNode(std::string name, std::string returnType, std::vector<std::unique_ptr<ParamNode>> &parameters, std::unique_ptr<BlockNode> &body, int line);
    ~FuncNode() override;
    std::string getName();
    std::string getReturnType();
    void setReturnType(std::string type);
    std::vector<std::unique_ptr<ParamNode>> &getParameters();
    virtual void print(int depth = 0) const override;
    void addSignatureToTable(SemanticsInfo &semanticsInfo, bool includeParams = true);
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct BundleNode : Node
{
private:
    std::string name;
    std::vector<IdentifierNode *> variables;
    std::vector<FuncNode *> functions;
    std::vector<std::unique_ptr<Node>> body;

public:
    BundleNode(std::string name, std::vector<std::unique_ptr<Node>> body, int line);
    ~BundleNode() override;
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
};

struct IfStmtNode : Node
{
private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<BlockNode> body;

public:
    IfStmtNode(std::unique_ptr<Node> &condition, std::unique_ptr<BlockNode> &body, int line);
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
    std::unique_ptr<Node> condition;
    std::unique_ptr<BlockNode> body;

public:
    ElseIfStmtNode(std::unique_ptr<Node> &condition, std::unique_ptr<BlockNode> &body, int line);
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
    std::unique_ptr<BlockNode> body;

public:
    ElseStmtNode(std::unique_ptr<BlockNode> &body, int line);
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
    std::unique_ptr<BlockNode> branches;

public:
    BranchBlockNode(int line);
    ~BranchBlockNode() override;
    static int getNumberIf();
    void setCurrentWhile(int currentStmt);
    void addBranch(std::unique_ptr<Node> branch);
    virtual void print(int depth = 0) const override;
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
    virtual bool returnValueInPath();
};

struct WhileNode : Node
{
private:
    static int numWhileStmt;
    std::unique_ptr<Node> condition;
    std::unique_ptr<BlockNode> body;
    bool skip;

public:
    WhileNode(std::unique_ptr<Node> &condition, std::unique_ptr<BlockNode> &body, int line);
    ~WhileNode() override;
    bool getSkip();
    void setSkip(bool skip);
    virtual void print(int depth = 0) const override;
    bool endsWithBranchStmt();
    virtual void checkSemantics(SemanticsInfo &semanticsInfo) override;
    virtual void mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global = false, std::string prefix = "") override;
    virtual void findReturnStmts(std::vector<Node *> &stmts) override;
};

struct StopNode : Node
{
private:
    int currentWhileStmt;

public:
    StopNode(int line);
    ~StopNode() override;
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
