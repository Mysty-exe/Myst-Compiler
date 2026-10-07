#include "compiler/semantic_analyzer.h"

Symbol::Symbol(int line, bool used, std::string name, std::string type)
{
    this->line = line;
    this->used = used;
    this->name = name;
    this->type = type;
}
Symbol::~Symbol()
{
}
int Symbol::getLine() const
{
    return line;
}
bool Symbol::isUsed() const
{
    return used;
}
std::string Symbol::getName() const
{
    return name;
}
std::string Symbol::getDataType() const
{
    return type;
}
void Symbol::setUsed(bool flag)
{
    this->used = flag;
}

VarSymbol::VarSymbol(int line, bool used, std::string name, std::string type) : Symbol(line, used, name, type)
{
}
VarSymbol::~VarSymbol()
{
}
std::string VarSymbol::getType()
{
    return "VAR";
}

FuncSymbol::FuncSymbol(int line, bool used, std::string name, std::string type, bool hasReturnIfNeeded, std::vector<std::string> parameterTypes) : Symbol(line, used, name, type)
{
    this->hasReturnIfNeeded = hasReturnIfNeeded;
    this->parameterTypes = parameterTypes;
}
FuncSymbol::~FuncSymbol()
{
}
bool FuncSymbol::hasReturn() const
{
    return hasReturnIfNeeded;
}
void FuncSymbol::setReturnType(std::string type)
{
    this->returnType = type;
}
void FuncSymbol::setHasReturnFlag(bool flag)
{
    this->hasReturnIfNeeded = flag;
}
std::vector<std::string> FuncSymbol::getParameters() const
{
    return parameterTypes;
}
int FuncSymbol::getNumParameters() const
{
    return parameterTypes.size();
}
std::string FuncSymbol::getType()
{
    return "FUNC";
}

std::vector<Symbol *> Scope::getSymbols()
{
    return symbols;
}
void Scope::addSymbol(Symbol *symbol)
{
    symbols.push_back(symbol);
}

void SemanticAnalyzer::analyzeTree(RootNode *root)
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::vector<std::string> visibleScope;

    root->checkSemantics(symbolTable, visibleScope);
    root->sendWarnings(symbolTable);
}
