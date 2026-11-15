#include "compiler/node.h"

CallExprNode::CallExprNode(std::string name, std::vector<Node *> arguments)
{
    this->name = name;
    this->arguments = arguments;
    imported = false;
    modName = "";
}
CallExprNode::~CallExprNode()
{
    for (Node *arg : arguments)
    {
        delete arg;
        arg = nullptr;
    }
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

    if (imported)
    {
        auto scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                if (returnType == "" || symbol->getDataType() == returnType || canCoerce(symbol->getDataType(), returnType))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(semanticsInfo, derivedSymbol->getParameters()[k]))
                            break;
                    }
                }
            }
        }

        return;
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                if (returnType == "" || symbol->getDataType() == returnType || canCoerce(symbol->getDataType(), returnType))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(semanticsInfo, derivedSymbol->getParameters()[k]))
                        {
                            wrongType = true;
                            break;
                        }
                    }

                    if (!wrongType)
                        symbol->setUsed(true);
                }
            }
        }
    }
}
int CallExprNode::checkCallExprInScope(Scope scope, SemanticsInfo semanticsInfo, std::string type, bool &overridenFunction, bool importing)
{
    for (Symbol *symbol : scope.getSymbols())
    {
        if (symbol->getType() == "FUNC" && symbol->getName() == name)
        {
            if (importing)
            {
                if (!symbol->isExported())
                    continue;
            }

            overridenFunction = true;
            if (type == "" || symbol->getDataType() == type || canCoerce(symbol->getDataType(), type))
            {
                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                if (arguments.size() != derivedSymbol->getNumParameters())
                    continue;

                bool wrongType = false;
                for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                {
                    std::string p = derivedSymbol->getParameters()[k];
                    if (p.substr(0, 3) == "REF")
                    {
                        p = p.substr(3);
                        if (!dynamic_cast<IdentifierNode *>(arguments[k]))
                            throwSemanticError(line, "Must use an identifier (variable) for a reference.");

                        if (!arguments[k]->checkType(semanticsInfo, p))
                        {
                            wrongType = true;
                            break;
                        }
                    }

                    this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                    arguments[k]->checkType(semanticsInfo, derivedSymbol->getParameters()[k]);
                    if (!canCoerce(arguments[k]->inferType(semanticsInfo), p))
                    {
                        if (!arguments[k]->checkType(semanticsInfo, p))
                        {
                            wrongType = true;
                            break;
                        }
                    }
                }

                if (wrongType)
                    continue;
                else
                {
                    this->returnType = symbol->getDataType();
                    return 1;
                }
            }
            else
                return -1;
        }
    }

    return 0;
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
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toBool")
                if (argType == "INTTYPE")
                    returnType = "BOOLTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toInt")
                if (argType == "DECIMALTYPE" || argType == "CHARTYPE" || argType == "BOOLTYPE")
                    returnType = "INTTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toDecimal")
                if (argType == "INTTYPE")
                    returnType = "DECIMALTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");

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
                    if ((Token::builtInFunctions[i][j] == "var" || Token::builtInFunctions[i][j] == currentType))
                    {
                        argTypes.push_back(currentType);
                        continue;
                    }
                    else
                        found = false;
                }

                for (Node *arg : arguments)
                    arg->checkType(semanticsInfo, arg->inferType(semanticsInfo));

                if (found)
                    return true;
            }
        }
    }

    if (name == "print" || name == "input" || name == "exit")
        throwSemanticError(line, "function '" + name + "' has the wrong arguments passed in.");

    bool overridenFunction = false;
    if (imported)
    {
        auto scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            int checked = checkCallExprInScope(scope, semanticsInfo, type, overridenFunction, true);
            if (checked == 1)
                return true;
            else if (checked == -1)
                return false;
        }

        if (overridenFunction)
            throwSemanticError(line, "Coulnd't find proper function signature for '" + name + "' call expression in Module '" + modName + "'.");

        throwSemanticError(line, "function '" + name + "' was not found in Module '" + modName + "'.");
        return false;
    }

    overridenFunction = false;
    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        int checked = checkCallExprInScope(scope, semanticsInfo, type, overridenFunction, false);
        if (checked == 1)
            return true;
        else if (checked == -1)
            return false;
    }

    if (overridenFunction)
        throwSemanticError(line, "Coulnd't find proper function signature for '" + name + "' call expression.");

    throwSemanticError(line, "function '" + name + "' hasn't been declared.");
    return false;
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
        auto scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
            }
        }

        throwSemanticError(line, "function '" + name + "' was not found in Module '" + modName + "'.");
        return "";
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
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
        bool isRef = argTypes[i].substr(0, 3) == "REF";
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
            IdentifierNode *identifier = static_cast<IdentifierNode *>(arguments[i]);
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
            if (argTypes[i].substr(0, 3) == "REF")
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

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
    declared = true;
    if (type == "")
        declared = false;
    this->identifier = new IdentifierNode(name, type);
    this->value = value;
}
AssignNode::~AssignNode()
{
    delete identifier;
    identifier = nullptr;
    delete value;
    value = nullptr;
}
std::string AssignNode::getName() const
{
    return identifier->getName();
}
std::string AssignNode::getType() const
{
    return identifier->getType();
}
Node *AssignNode::getValue() const
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
        auto scope = semanticsInfo.getScope(semanticsInfo.getLatestVisibleScope());
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName() && !(semanticsInfo.getVisibleScope().size() > 1))
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared.");

            if (symbol->getType() == "FUNC" && symbol->getName() == identifier->getName())
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a function.");
        }

        if (identifier->getType() == "VAR")
        {
            std::string t = value->inferType(semanticsInfo);
            identifier->setType(t);
            value->checkType(semanticsInfo, identifier->getType());
            semanticsInfo.addSymbol(semanticsInfo.getLatestVisibleScope(), new VarSymbol(line, false, identifier->getName(), t));
        }
        else if (value->checkType(semanticsInfo, identifier->getType()) || canCoerce(value->inferType(semanticsInfo), identifier->getType()))
            semanticsInfo.addSymbol(semanticsInfo.getLatestVisibleScope(), new VarSymbol(line, false, identifier->getName(), identifier->getType()));
        else
            throwSemanticError(line, "Value in '" + identifier->getName() + "' must have type '" + getDataTypeStr(identifier->getType()) + "'.");
    }
    else
    {
        for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
        {
            auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
            for (Symbol *symbol : scope.getSymbols())
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

ReturnStmtNode::ReturnStmtNode(Node *identifier)
{
    this->identifier = identifier;
}
ReturnStmtNode::~ReturnStmtNode()
{
    delete identifier;
    identifier = nullptr;
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
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC")
            {
                if (identifier == nullptr && returnType != "")
                    throwSemanticError(line, "Return statement found without an identifier.");

                if (returnType == "" && identifier != nullptr)
                    throwSemanticError(line, "Return statement found in a function that doesn't have a return type.");

                if (identifier != nullptr && !identifier->checkType(semanticsInfo, returnType) && !canCoerce(identifier->inferType(semanticsInfo), returnType))
                    throwSemanticError(line, "Return statement doesn't return the required return type.");

                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
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

BreakNode::BreakNode(int line)
{
    this->line = line;
    currentWhileStmt = 0;
}
BreakNode::~BreakNode()
{
}
void BreakNode::setCurrentWhile(int currentStmt)
{
    currentWhileStmt = currentStmt;
}
void BreakNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Break Statement\n";
}
void BreakNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
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
