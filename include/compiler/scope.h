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

    ~VarSymbol()
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

    ~FuncSymbol()
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
public:
    ContainerSymbol(int line, std::string name, bool used = true, std::string type = "") : Symbol(line, used, name, type)
    {
    }

    virtual std::string getType() override
    {
        return "CONTAINER";
    }
};

struct Scope
{
private:
    std::vector<Symbol *> symbols;

public:
    std::vector<Symbol *> getSymbols() const
    {
        return symbols;
    }

    void setSymbols(std::vector<Symbol *> symbols)
    {
        this->symbols = symbols;
    }

    void addSymbol(Symbol *symbol)
    {
        symbols.push_back(symbol);
    }
};
