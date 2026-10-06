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
    std::string name;

public:
    Symbol(int line, bool used, std::string name);
    virtual ~Symbol();
    int getLine() const;
    bool isUsed() const;
    std::string getName() const;
    void setUsed(bool flag);
    virtual std::string getType() = 0;
};

struct VarSymbol : public Symbol
{
public:
    VarSymbol(int line, bool used, std::string name);
    ~VarSymbol() override;
    virtual std::string getType() override;
};

struct FuncSymbol : public Symbol
{
private:
    bool hasReturnIfNeeded;
    std::string returnType;
    std::vector<std::string> parameterTypes;

public:
    FuncSymbol(int line, bool used, std::string name, bool hasReturnIfNeeded, std::string returnType, std::vector<std::string> parameterTypes);
    ~FuncSymbol() override;
    std::string getReturnType() const;
    bool hasReturn() const;
    void setReturnType(std::string type);
    void setHasReturnFlag(bool flag);
    virtual std::string getType() override;
};

struct Scope
{
private:
    std::vector<Symbol *> symbols;

public:
    std::vector<Symbol *> getSymbols();
};

class SemanticAnalyzer
{
public:
    static void analyzeTree(RootNode *rootNode);
};
