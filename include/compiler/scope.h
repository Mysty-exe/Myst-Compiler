#pragma once
#include <iostream>
#include <vector>

struct Symbol
{
protected:
    int line;
    bool used, exported;
    std::string name, type;

public:
    Symbol(int line, bool used, std::string name, std::string type)
    {
        this->line = line;
        this->used = used;
        this->name = name;
        this->type = type;
        this->exported = false;
    }

    virtual ~Symbol()
    {
    }

    int getLine() const
    {
        return line;
    }

    bool isUsed() const
    {
        return used;
    }

    bool isExported() const
    {
        return exported;
    }

    std::string getName() const
    {
        return name;
    }

    std::string getDataType() const
    {
        return type;
    }

    void setExported(bool flag)
    {
        this->exported = flag;
    }

    void setUsed(bool flag)
    {
        this->used = flag;
    }

    virtual std::string getType() = 0;
};

struct VarSymbol : Symbol
{
public:
    VarSymbol(int line, bool used, std::string name, std::string type) : Symbol(line, used, name, type)
    {
    }

    ~VarSymbol() override
    {
    }

    virtual std::string getType() override
    {
        return "VAR";
    }
};

struct FuncSymbol : Symbol
{
private:
    bool hasReturnIfNeeded;
    std::string returnType;
    std::vector<std::string> parameterTypes;

public:
    FuncSymbol(int line, bool used, std::string name, std::string type, bool hasReturnIfNeeded, std::vector<std::string> parameterTypes) : Symbol(line, used, name, type)
    {
        this->hasReturnIfNeeded = hasReturnIfNeeded;
        this->parameterTypes = parameterTypes;
    }

    ~FuncSymbol() override
    {
    }

    bool hasReturn() const
    {
        return hasReturnIfNeeded;
    }

    void setReturnType(std::string type)
    {
        this->returnType = type;
    }

    void setHasReturnFlag(bool flag)
    {
        this->hasReturnIfNeeded = flag;
    }

    std::vector<std::string> getParameters() const
    {
        return parameterTypes;
    }

    int getNumParameters() const
    {
        return parameterTypes.size();
    }

    virtual std::string getType() override
    {
        return "FUNC";
    }
};

struct ContainerSymbol : Symbol
{
private:
    std::vector<std::unique_ptr<VarSymbol>> variables;
    std::vector<std::unique_ptr<FuncSymbol>> functions;

public:
    ContainerSymbol(int line, std::string name, std::vector<std::unique_ptr<VarSymbol>> variables, std::vector<std::unique_ptr<FuncSymbol>> funcs, bool used = true, std::string type = "") : Symbol(line, used, name, type)
    {
        this->variables = variables;
        this->functions = funcs;
    }

    ~ContainerSymbol() override
    {
    }

    virtual std::string getType() override
    {
        return "CONTAINER";
    }

    std::vector<std::unique_ptr<VarSymbol>> getVariables()
    {
        return variables;
    }

    std::vector<std::unique_ptr<FuncSymbol>> getFunctions()
    {
        return functions;
    }
};

struct Scope
{
private:
    std::vector<std::unique_ptr<Symbol>> symbols;

public:
    std::vector<std::unique_ptr<Symbol>> &getSymbols()
    {
        return symbols;
    }

    void setSymbols(std::vector<std::unique_ptr<Symbol>> symbols)
    {
        this->symbols = symbols;
    }

    void addSymbol(std::unique_ptr<Symbol> &symbol)
    {
        symbols.push_back(symbol);
    }
};
