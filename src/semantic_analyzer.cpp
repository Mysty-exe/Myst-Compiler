#include "compiler/semantic_analyzer.h"

Symbol::Symbol(int line, bool used, std::string name)
{
    this->line = line;
    this->used = used;
    this->name = name;
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
void Symbol::setUsed(bool flag)
{
    this->used = flag;
}

VarSymbol::VarSymbol(int line, bool used, std::string name) : Symbol(line, used, name)
{
}
VarSymbol::~VarSymbol()
{
}
std::string VarSymbol::getType()
{
    return "VAR";
}

FuncSymbol::FuncSymbol(int line, bool used, std::string name, bool hasReturnIfNeeded, std::string returnType, std::vector<std::string> parameterTypes) : Symbol(line, used, name)
{
}
FuncSymbol::~FuncSymbol()
{
}
std::string FuncSymbol::getReturnType() const
{
    return returnType;
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
std::string FuncSymbol::getType()
{
    return "FUNC";
}

std::vector<Symbol *> Scope::getSymbols()
{
    return symbols;
}

void SemanticAnalyzer::analyzeTree(RootNode *root)
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::vector<std::string> visibleScope;

    root->checkSemantics(symbolTable, visibleScope);
    root->sendWarnings(symbolTable);
}
