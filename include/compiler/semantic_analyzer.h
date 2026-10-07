#pragma once
#include <iostream>
#include <unordered_map>
#include "lexer.h"
#include "ast.h"

struct Symbol
{
protected:
    int line;
    bool used;
    std::string name, type;

public:
    Symbol(int line, bool used, std::string name, std::string type);
    virtual ~Symbol();
    int getLine() const;
    bool isUsed() const;
    std::string getName() const;
    std::string getDataType() const;
    void setUsed(bool flag);
    virtual std::string getType() = 0;
};

struct VarSymbol : Symbol
{
public:
    VarSymbol(int line, bool used, std::string name, std::string type);
    ~VarSymbol() override;
    virtual std::string getType() override;
};

struct FuncSymbol : Symbol
{
private:
    bool hasReturnIfNeeded;
    std::string returnType;
    std::vector<std::string> parameterTypes;

public:
    FuncSymbol(int line, bool used, std::string name, std::string type, bool hasReturnIfNeeded, std::vector<std::string> parameterTypes);
    ~FuncSymbol() override;
    bool hasReturn() const;
    void setReturnType(std::string type);
    void setHasReturnFlag(bool flag);
    std::vector<std::string> getParameters() const;
    int getNumParameters() const;
    virtual std::string getType() override;
};

struct Scope
{
private:
    std::vector<Symbol *> symbols;

public:
    std::vector<Symbol *> getSymbols();
    void addSymbol(Symbol *symbol);
};

class SemanticAnalyzer
{
public:
    static void analyzeTree(RootNode *rootNode);
};
