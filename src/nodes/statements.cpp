#include "compiler/node.h"

CallExprNode::CallExprNode(std::string name, std::vector<std::unique_ptr<Node>> &arguments, int line)
{
    this->name = name;
    this->arguments = std::move(arguments);
    this->line = line;
    imported = false;
    modName = "";
}
CallExprNode::~CallExprNode()
{
}
std::string CallExprNode::getName() const
{
    return name;
}
std::string CallExprNode::getReturnType() const
{
    return returnType;
}
std::string CallExprNode::getModName() const
{
    return modName;
}
void CallExprNode::setBundle(std::string bundle)
{
    this->bundle = bundle;
}
void CallExprNode::setImported(std::string modName)
{
    imported = true;
    this->modName = modName;
}
void CallExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    if (imported)
    {
        std::cout << depthTabs << "Imported CallExpressionNode: " << name << std::endl;
        std::cout << depthTabs << "\tFrom Module: " + modName << std::endl;
    }
    else
        std::cout << depthTabs << "CallExpressionNode: " << name << std::endl;

    std::cout << depthTabs << "\tArguments: " << std::endl;
    for (int i = 0; i < arguments.size(); i++)
        arguments[i]->print(depth + 2);
}
void CallExprNode::lookForCallExprs(SemanticsInfo &semanticsInfo)
{
    if (name == "toChar" || name == "toBool" || name == "toInt" || name == "toDecimal")
        return;

    argTypes = {};
    FuncSymbol *funcSymbol = nullptr;
    std::vector<int> funcArgs;
    bool ambiguous = false;

    if (imported)
    {
        if (semanticsInfo.getPrefixSymbols().count(modName) == 0)
            throwSemanticError(line, "Module '" + modName + "' was not found.");

        auto &scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        checkCallExprInScope(scope, semanticsInfo, returnType, funcSymbol, funcArgs, ambiguous, true);

        if (ambiguous)
            throwSemanticError(line, "Couldn't decide on the correct function signature to use, functions are ambiguous.");

        if (funcSymbol == nullptr)
            throwSemanticError(line, "Couldn't find proper function signature for '" + name + "' call expression in Module '" + modName + "'.");

        for (int i = 0; i < funcSymbol->getNumParameters(); i++)
            this->argTypes.push_back(funcSymbol->getParameters()[i]);

        return;
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto &scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        checkCallExprInScope(scope, semanticsInfo, returnType, funcSymbol, funcArgs, ambiguous, false);
    }

    if (ambiguous)
        throwSemanticError(line, "Couldn't decide on the correct function signature to use, functions are ambiguous.");

    if (funcSymbol == nullptr)
        throwSemanticError(line, "Couldn't find proper function signature for '" + name + "' call expression.");

    for (int i = 0; i < funcSymbol->getNumParameters(); i++)
        this->argTypes.push_back(funcSymbol->getParameters()[i]);
}
std::vector<int> CallExprNode::checkFuncArgs(SemanticsInfo &semanticsInfo, FuncSymbol *&funcSymbol)
{
    std::vector<int> result;
    for (int k = 0; k < funcSymbol->getParameters().size(); k++)
    {
        std::string p = funcSymbol->getParameters()[k];
        if (p.substr(0, 7) == "BORROW.")
        {
            p = p.substr(7);

            if (arguments[k]->inferType(semanticsInfo) == p)
                result.push_back(0);
            else if (canCoerceLossless(arguments[k]->inferType(semanticsInfo), p))
                result.push_back(1);
            else if (canCoerceLossy(arguments[k]->inferType(semanticsInfo), p))
                result.push_back(2);
            else
                result.push_back(-1);
        }

        arguments[k]->checkType(semanticsInfo, funcSymbol->getParameters()[k]);
        if (!canCoerce(arguments[k]->inferType(semanticsInfo), p))
        {
            if (arguments[k]->inferType(semanticsInfo) == p)
                result.push_back(0);
            else if (canCoerceLossless(arguments[k]->inferType(semanticsInfo), p))
                result.push_back(1);
            else if (canCoerceLossy(arguments[k]->inferType(semanticsInfo), p))
                result.push_back(2);
            else
                result.push_back(-1);
        }
    }

    this->returnType = funcSymbol->getDataType();
    funcSymbol->setUsed(true);

    return result;
}
bool CallExprNode::validFunc(std::vector<int> current)
{
    for (int i = 0; i < current.size(); i++)
    {
        if (current[i] == -1)
            return false;
    }

    return true;
}
bool CallExprNode::betterFunc(std::vector<int> current, std::vector<int> next)
{
    if (current.size() != next.size())
        return false;

    bool betterThan = false;
    for (int i = 0; i < current.size(); i++)
    {
        if (next[i] > current[i])
            betterThan = true;

        if (next[i] < current[i])
            return false;
    }

    return betterThan;
}
bool CallExprNode::isAmbiguousFunc(std::vector<int> current, std::vector<int> next)
{
    if (current.size() != next.size())
        return false;

    bool betterThanA = false, betterThanB = false;
    for (int i = 0; i < current.size(); i++)
    {
        if (next[i] > current[i])
            betterThanA = true;

        if (next[i] < current[i])
            betterThanB = true;
    }

    return (!betterThanA && !betterThanB) || (betterThanA && betterThanB);
}
void CallExprNode::checkCallExprInScope(Scope &scope, SemanticsInfo &semanticsInfo, std::string type, FuncSymbol *&funcSymbol, std::vector<int> &funcArgs, bool &ambiguous, bool importing)
{
    for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
    {
        if (importing)
        {
            if (!symbol->isExported())
                continue;
        }

        if (symbol->getType() == "BUNDLE" && symbol->getName() == name)
        {
            auto *derivedSymbol = static_cast<BundleSymbol *>(symbol.get());
            for (std::unique_ptr<FuncSymbol> &func : derivedSymbol->getFunctions())
            {
                auto *derivedFuncSymbol = static_cast<FuncSymbol *>(func.get());
                if ((type == "" || func->getDataType() == type || canCoerce(func->getDataType(), type)) && arguments.size() == func->getNumParameters())
                {
                    std::vector<int> next = checkFuncArgs(semanticsInfo, derivedFuncSymbol);
                    if (validFunc(next) && (betterFunc(funcArgs, next) || funcArgs.size() == 0))
                    {
                        funcSymbol = derivedFuncSymbol;
                        funcArgs = next;
                    }
                }
            }
        }

        if (symbol->getType() == "FUNC" && symbol->getName() == name)
        {
            auto *derivedSymbol = static_cast<FuncSymbol *>(symbol.get());
            if ((type == "" || symbol->getDataType() == type || canCoerce(symbol->getDataType(), type)) && arguments.size() == derivedSymbol->getNumParameters())
            {
                std::vector<int> next = checkFuncArgs(semanticsInfo, derivedSymbol);
                if (next.size() > 0 && isAmbiguousFunc(funcArgs, next))
                {
                    ambiguous = true;
                    continue;
                }
                if (validFunc(next) && (betterFunc(funcArgs, next) || funcArgs.size() == 0))
                {
                    ambiguous = false;
                    funcSymbol = derivedSymbol;
                    funcArgs = next;
                }
            }
        }
    }
}
bool CallExprNode::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
{
    argTypes = {};

    if (name == "toChar" || name == "toBool" || name == "toInt" || name == "toDecimal")
    {
        if (arguments.size() == 1)
        {
            std::string argType = arguments[0]->inferType(semanticsInfo);
            if (name == "toChar")
                if (argType == "INTTYPE")
                    returnType = "CHARTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with type '" + getDataTypeStr(argType) + "'.");
            if (name == "toBool")
                if (argType == "INTTYPE")
                    returnType = "BOOLTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with type '" + getDataTypeStr(argType) + "'.");
            if (name == "toInt")
                if (argType == "DECIMALTYPE" || argType == "CHARTYPE" || argType == "BOOLTYPE")
                    returnType = "INTTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with type '" + getDataTypeStr(argType) + "'.");
            if (name == "toDecimal")
                if (argType == "INTTYPE")
                    returnType = "DECIMALTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with type '" + getDataTypeStr(argType) + "'.");

            argTypes.push_back(argType);
            arguments[0]->checkType(semanticsInfo, argType);
            return true;
        }
        else
            throwSemanticError(line, "'" + name + "' has the wrong arguments passed in.");
    }

    bool found = true;
    if (name == "print" || name == "input" || name == "exit")
    {
        this->returnType = "";

        for (int i = 0; i < Token::builtInFunctions.size(); i++)
        {
            found = true;
            if (Token::builtInFunctions[i][0] != name)
                continue;
            else
            {
                if (arguments.size() != Token::builtInFunctions[i].size() - 1)
                    continue;

                for (int j = 1; j < Token::builtInFunctions[i].size(); j++)
                {
                    std::string currentType = arguments[j - 1]->inferType(semanticsInfo);
                    if ((Token::builtInFunctions[i][j] == "infer" || Token::builtInFunctions[i][j] == currentType))
                    {
                        argTypes.push_back(currentType);
                        continue;
                    }
                    else
                        found = false;
                }

                for (std::unique_ptr<Node> &arg : arguments)
                    arg->checkType(semanticsInfo, arg->inferType(semanticsInfo));

                if (found)
                    return true;
            }
        }
    }

    if (name == "print" || name == "input" || name == "exit")
        throwSemanticError(line, "function '" + name + "' has the wrong arguments passed in.");

    FuncSymbol *funcSymbol = nullptr;
    std::vector<int> funcArgs;
    bool ambiguous = false;
    if (imported)
    {
        if (semanticsInfo.getPrefixSymbols().count(modName) == 0)
            throwSemanticError(line, "Module '" + modName + "' was not found.");

        auto &scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        checkCallExprInScope(scope, semanticsInfo, type, funcSymbol, funcArgs, ambiguous, true);

        if (ambiguous)
            throwSemanticError(line, "Couldn't decide on the correct function signature to use, functions are ambiguous.");

        if (funcSymbol == nullptr)
            throwSemanticError(line, "Couldn't find proper function signature for '" + name + "' call expression in Module '" + modName + "'.");

        for (int i = 0; i < funcSymbol->getNumParameters(); i++)
            this->argTypes.push_back(funcSymbol->getParameters()[i]);

        return true;
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto &scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        checkCallExprInScope(scope, semanticsInfo, type, funcSymbol, funcArgs, ambiguous, false);
    }

    if (ambiguous)
        throwSemanticError(line, "Couldn't decide on the correct function signature to use, functions are ambiguous.");

    if (funcSymbol == nullptr)
        throwSemanticError(line, "Couldn't find proper function signature for '" + name + "' call expression.");

    for (int i = 0; i < funcSymbol->getNumParameters(); i++)
        this->argTypes.push_back(funcSymbol->getParameters()[i]);

    return true;
}
void CallExprNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    checkType(semanticsInfo, "");
}

std::string CallExprNode::inferType(SemanticsInfo &semanticsInfo)
{
    if (name == "print" || name == "input" || name == "exit")
        return "";

    if (name == "toChar")
        return "CHARTYPE";
    if (name == "toBool")
        return "BOOLTYPE";
    if (name == "toInt")
        return "INTTYPE";
    if (name == "toDecimal")
        return "DECIMALTYPE";

    if (imported)
    {
        if (semanticsInfo.getPrefixSymbols().count(modName) == 0)
            throwSemanticError(line, "Module '" + modName + "' was not found.");

        auto &scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
            }
            if (symbol->getType() == "BUNDLE" && symbol->getName() == name && symbol->isExported())
            {
                returnType = symbol->getName();
                return symbol->getName();
            }
        }

        throwSemanticError(line, "function '" + name + "' was not found in Module '" + modName + "'.");
        return "";
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto &scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
            }
            if (symbol->getType() == "BUNDLE" && symbol->getName() == name)
            {
                returnType = symbol->getName();
                return symbol->getName();
            }
        }
    }

    throwSemanticError(line, "function '" + name + "' hasn't been declared.");
    return "";
}
void CallExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    mapToIR(ir, llvmInfo, global, prefix);

    if (imported)
        prefix = "." + llvmInfo.getImportName(modName) + ".";

    std::string tempVarName = std::to_string(llvmInfo.getCurrentTempVar());
    std::string llvmType = (returnType != newType) ? newType : returnType;

    if (returnType != newType)
    {
        coerceValue(ir, llvmInfo, name, tempVarName, std::to_string(llvmInfo.getCurrentTempVar()), returnType, newType);
    }

    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
        return;
    }

    if (global)
    {
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    auto tempVarMap = llvmInfo.getTempVars();
    for (int i = tempVarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempVarMap[i].size() - 1; j >= 0; j--)
        {
            bool inScope = checkTempVarInScope(tempVarMap[i][j][0], outOfScope);
            if (!inScope)
                continue;

            if (tempVarMap[i][j].size() == 2 && tempVarMap[i][j][0] == assignmentName && tempVarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(llvmType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
    }

    llvmInfo.addTempVar({name, "ptr"});
}
void CallExprNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (imported)
        prefix = "." + llvmInfo.getImportName(modName) + ".";

    std::string callName = prefix + name;
    for (int i = 0; i < argTypes.size(); i++)
        callName += "." + argTypes[i];

    if (name == "print")
    {
        if (argTypes[0] == "CHARTYPE")
            callName = "print_char";
        else if (argTypes[0] == "DECIMALTYPE")
            callName = "print_double";
        else
            callName = "print_int";

        if (argTypes.size() > 1)
            callName += "_end";
    }
    else if (name == "input")
        callName = name;
    else if (name == "exit")
        callName = "myst_exit";
    else if (name == "main")
        callName = "__main__";

    if (name == "toInt" || name == "toDecimal" || name == "toChar" || name == "toBool")
        callName = name;

    std::vector<std::string> args;
    for (int i = 0; i < arguments.size(); i++)
    {
        bool isRef = argTypes[i].substr(0, 7) == "BORROW.";
        if (!isRef)
        {
            std::string callArg = ".callarg_" + std::to_string(llvmInfo.getNextTempVar());
            std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
            ir += "%" + loadTemp + " = alloca " + mapTypeIR(argTypes[i]) + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
            llvmInfo.addTempVar({callArg, "ptr"});

            std::vector<std::string> loadedVars;
            arguments[i]->loadVariables(ir, argTypes[i], loadedVars, llvmInfo, global, prefix);
            arguments[i]->storeValue(ir, argTypes[i], callArg, llvmInfo, false, prefix);

            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = load " + mapTypeIR(argTypes[i]) + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
            llvmInfo.addTempVar({callArg, "ptr"});
            args.push_back("%" + std::to_string(llvmInfo.getCurrentTempVar()));
        }
        else
        {
            IdentifierNode *identifier = static_cast<IdentifierNode *>(arguments[i].get());
            if (global)
                args.push_back("@" + identifier->getName());
            else
            {
                bool globalVar = true;
                auto tempVarMap = llvmInfo.getTempVars();
                for (int i = tempVarMap.size() - 1; i > 0; i--)
                {
                    for (int j = tempVarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempVarMap[i][j].size() == 2 && tempVarMap[i][j][0] == identifier->getName() && tempVarMap[i][j][1] == "ptr")
                        {
                            globalVar = false;
                            args.push_back("%" + std::to_string(j + 1));
                            break;
                        }
                    }

                    if (!globalVar)
                        break;
                }

                if (globalVar)
                    args.push_back("@" + identifier->getName());
            }
        }
    }

    if (name == "toInt" || name == "toDecimal" || name == "toChar" || name == "toBool")
    {
        std::string newType;
        if (name == "toInt")
            newType = "INTTYPE";
        if (name == "toDecimal")
            newType = "DECIMALTYPE";
        if (name == "toChar")
            newType = "CHARTYPE";
        if (name == "toBool")
            newType = "BOOLTYPE";

        coerceValue(ir, llvmInfo, name, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), argTypes[0], newType);
    }
    else
    {
        if (returnType != "")
        {
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = call " + mapTypeIR(returnType) + " @" + callName + "(";
            llvmInfo.addTempVar({name, "ptr"});
        }
        else
            ir += "call " + mapTypeIR(returnType) + " @" + callName + "(";

        for (int i = 0; i < args.size(); i++)
        {
            if (argTypes[i].substr(0, 7) == "BORROW.")
            {
                if (i == args.size() - 1)
                    ir += "ptr " + args[i];
                else
                    ir += "ptr " + args[i] + ", ";
            }
            else
            {
                if (i == args.size() - 1)
                    ir += mapTypeIR(argTypes[i]) + " " + args[i];
                else
                    ir += mapTypeIR(argTypes[i]) + " " + args[i] + ", ";
            }
        }
        ir += ")\n";
    }
}

AssignNode::AssignNode(std::string name, std::string type, std::unique_ptr<Node> &value, int line)
{
    declared = true;
    if (type == "")
        declared = false;
    this->identifier = std::make_unique<IdentifierNode>(IdentifierNode(name, type, line));
    this->value = std::move(value);
    this->line = line;
}
AssignNode::~AssignNode()
{
}
std::string AssignNode::getName() const
{
    return identifier->getName();
}
std::string AssignNode::getType() const
{
    return identifier->getType();
}
std::unique_ptr<Node> &AssignNode::getValue()
{
    return value;
}
bool AssignNode::isDeclared() const
{
    return declared;
}
void AssignNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "AssignmentNode: " << std::endl;
    std::cout << depthTabs << "\tLeft Side: " << getDataTypeStr(identifier->getType()) << " " << identifier->getName() << std::endl;
    std::cout << depthTabs << "\tRight Side: " << std::endl;
    value->print(depth + 2);
}
void AssignNode::lookForCallExprs(SemanticsInfo &semanticsInfo)
{
    value->lookForCallExprs(semanticsInfo);
}
void AssignNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (identifier->getType() != "")
    {
        std::string visibleScope = semanticsInfo.getLatestVisibleScope();
        auto &scope = semanticsInfo.getScope(visibleScope);
        for (const std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "BUNDLE" && symbol->getName() == identifier->getName())
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a bundle.");

            if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName() && !(semanticsInfo.getVisibleScope().size() > 1))
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared.");

            if (symbol->getType() == "FUNC" && symbol->getName() == identifier->getName())
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a function.");
        }

        if (identifier->getType() == "INFER")
        {
            std::string t = value->inferType(semanticsInfo);
            identifier->setType(t);
            value->checkType(semanticsInfo, identifier->getType());
            std::unique_ptr<VarSymbol> varSymbol = std::make_unique<VarSymbol>(line, false, identifier->getName(), t);
            semanticsInfo.addSymbol(semanticsInfo.getLatestVisibleScope(), std::move(varSymbol));
        }
        else if (value->checkType(semanticsInfo, identifier->getType()) || canCoerce(value->inferType(semanticsInfo), identifier->getType()))
        {
            std::unique_ptr<VarSymbol> varSymbol = std::make_unique<VarSymbol>(line, false, identifier->getName(), identifier->getType());
            semanticsInfo.addSymbol(semanticsInfo.getLatestVisibleScope(), std::move(varSymbol));
        }
        else
            throwSemanticError(line, "Value in '" + identifier->getName() + "' must have type '" + getDataTypeStr(identifier->getType()) + "'.");
    }
    else
    {
        for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
        {
            auto &scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
            for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
            {
                if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName())
                {
                    if (value->checkType(semanticsInfo, symbol->getDataType()) || canCoerce(symbol->getDataType(), value->inferType(semanticsInfo)))
                    {
                        identifier->setType(symbol->getDataType());
                        return;
                    }
                    else
                        throwSemanticError(line, "'" + identifier->getName() + "' has already been declared with type '" + getDataTypeStr(symbol->getDataType()) + "'.");
                }

                if (symbol->getType() == "FUNC" && symbol->getName() == identifier->getName())
                    throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a function.");
            }
        }

        throwSemanticError(line, "'" + identifier->getName() + "' hasn't been declared.");
    }
}
void AssignNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (!global && declared)
    {
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = alloca " + mapTypeIR(identifier->getType()) + ", align " + mapNumBytesIR(identifier->getType()) + "\n";
        llvmInfo.addTempVar({identifier->getName(), "ptr"});
    }

    std::vector<std::string>
        loadedVars;
    std::string newType = identifier->getType();
    value->loadVariables(ir, newType, loadedVars, llvmInfo, global, prefix);
    value->storeValue(ir, newType, prefix + identifier->getName(), llvmInfo, global, prefix);
}

ReturnStmtNode::ReturnStmtNode(int line, std::unique_ptr<Node> identifier)
{
    this->identifier = std::move(identifier);
    this->line = line;
}
ReturnStmtNode::~ReturnStmtNode()
{
}
void ReturnStmtNode::setType(std::string type)
{
    this->returnType = type;
}
void ReturnStmtNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ReturnNode:" << std::endl;
    std::cout << depthTabs << "\tReturning:" << std::endl;
    if (identifier == nullptr)
        std::cout << depthTabs << "\t\tVoid" << std::endl;
    else
        identifier->print(depth + 2);
}
void ReturnStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto &scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC")
            {
                if (identifier == nullptr && returnType != "")
                    throwSemanticError(line, "Return statement found without an identifier.");

                if (returnType == "" && identifier != nullptr)
                    throwSemanticError(line, "Return statement found in a function that doesn't have a return type.");

                if (identifier != nullptr && !identifier->checkType(semanticsInfo, returnType) && !canCoerce(identifier->inferType(semanticsInfo), returnType))
                    throwSemanticError(line, "Return statement doesn't return the required return type.");

                auto *derivedSymbol = static_cast<FuncSymbol *>(symbol.get());
                derivedSymbol->setHasReturnFlag(true);
            }
        }
    }
}
void ReturnStmtNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string type = mapTypeIR(returnType);
    std::vector<std::string> loadedVars;

    if (identifier == nullptr)
        ir += "ret void\n";
    else
    {
        std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
        ir += "%" + loadTemp + " = alloca " + mapTypeIR(returnType) + ", align " + mapNumBytesIR(returnType) + "\n";
        llvmInfo.addTempVar({".return_" + loadTemp, "ptr"});

        identifier->loadVariables(ir, returnType, loadedVars, llvmInfo, false, prefix);
        identifier->storeValue(ir, returnType, ".return_" + loadTemp, llvmInfo, false, prefix);

        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = load " + type + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(returnType) + "\n";
        ir += "ret " + type + " %" + std::to_string(llvmInfo.getNextTempVar()) + "\n";
        llvmInfo.addTempVar({".return_" + std::to_string(llvmInfo.getNextTempVar()), "ptr"});
    }
}
void ReturnStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    stmts.push_back(this);
}
bool ReturnStmtNode::returnValueInPath()
{
    return true;
}

StopNode::StopNode(int line)
{
    this->line = line;
    currentWhileStmt = 0;
}
StopNode::~StopNode()
{
}
void StopNode::setCurrentWhile(int currentStmt)
{
    currentWhileStmt = currentStmt;
}
void StopNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Break Statement\n";
}
void StopNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    ir += "br label %while_end_" + std::to_string(currentWhileStmt) + "\n";
}

SkipNode::SkipNode(int line)
{
    this->line = line;
    currentWhileStmt = 0;
}
SkipNode::~SkipNode()
{
}
void SkipNode::setCurrentWhile(int currentStmt)
{
    currentWhileStmt = currentStmt;
}
void SkipNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Skip Statement\n";
}
void SkipNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    ir += "br label %while_cond_" + std::to_string(currentWhileStmt) + "\n";
}
