#include "compiler/node.h"

ParamNode::ParamNode(std::vector<std::string> identifier, int line)
{
    reference = false;
    this->line = line;
    if (identifier.size() == 2)
    {
        type = identifier[0];
        name = identifier[1];
    }
    else if (identifier.size() == 3 && identifier[0] == "ref")
    {
        reference = true;
        type = identifier[1];
        name = identifier[2];
    }
    else
        throwSyntaxError(line, "Couldn't parse parameters.");

    if (type == "char")
        this->type = "CHARTYPE";
    else if (type == "int")
        this->type = "INTTYPE";
    else if (type == "decimal")
        this->type = "DECIMALTYPE";
    else if (type == "bool")
        this->type = "BOOLTYPE";
    else
        this->type = type;
}
ParamNode::~ParamNode()
{
}
std::string ParamNode::getName() const
{
    return name;
}
std::string ParamNode::getType() const
{
    return type;
}
bool ParamNode::isReference() const
{
    return reference;
}
void ParamNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ParameterNode: " << getDataTypeStr(type) << " " << name << (reference ? ", Reference" : "") << std::endl;
}

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<std::unique_ptr<ParamNode>> parameters, std::unique_ptr<BlockNode> body, int line)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
    this->line = line;

    emptyFunction = false;
    if (body->getNumStatements() == 0)
        emptyFunction = true;

    if (returnType == "char")
        this->returnType = "CHARTYPE";
    else if (returnType == "int")
        this->returnType = "INTTYPE";
    else if (returnType == "decimal")
        this->returnType = "DECIMALTYPE";
    else if (returnType == "bool")
        this->returnType = "BOOLTYPE";
    else
        this->returnType = returnType;

    if (!emptyFunction)
    {
        std::vector<std::unique_ptr<Node>> returnStmts;
        findReturnStmts(returnStmts);

        if (returnStmts.size() != 0)
        {
            for (int i = 0; i < returnStmts.size(); i++)
            {
                std::unique_ptr<ReturnStmtNode> returnStmt = static_cast<std::unique_ptr<ReturnStmtNode>>(returnStmts[i]);
                returnStmt->setType(this->returnType);
            }
        }
        else
        {
            if (this->returnType != "")
                throwSemanticError(line, "Return Statement not found in function.");
        }
    }
}
FuncNode::~FuncNode()
{
}
std::string FuncNode::getName()
{
    return name;
}
std::string FuncNode::getReturnType()
{
    return returnType;
}
void FuncNode::setReturnType(std::string type)
{
    returnType = type;
}
std::vector<std::unique_ptr<ParamNode>> FuncNode::getParameters()
{
    return parameters;
}
void FuncNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "FunctionNode: " << name << std::endl;
    std::cout << depthTabs << "\tReturn Type: " << getDataTypeStr(returnType) << std::endl;
    std::cout << depthTabs << "\tParameters: " << std::endl;
    for (int i = 0; i < parameters.size(); i++)
        parameters[i]->print(depth + 2);
    if (!emptyFunction)
        body->print(depth + 1);
}
void FuncNode::addSignatureToTable(SemanticsInfo &semanticsInfo, bool includeParams)
{
    std::vector<std::string> parameterTypes;
    for (std::unique_ptr<ParamNode> &parameter : parameters)
    {
        if (parameter->isReference())
            parameterTypes.push_back("REF." + parameter->getType());
        else
            parameterTypes.push_back(parameter->getType());
    }

    semanticsInfo.addSymbol(semanticsInfo.getVisibleScope()[0], std::make_unique<FuncSymbol>(FuncSymbol(line, false, name, returnType, false, parameterTypes)));
    if (includeParams)
    {
        semanticsInfo.addVisibleScope();
        std::string scope = semanticsInfo.getLatestVisibleScope();
        for (std::unique_ptr<ParamNode> &parameter : parameters)
            semanticsInfo.addSymbol(scope, std::make_unique<VarSymbol>(parameter->getLine(), false, parameter->getName(), parameter->getType()));
    }
}
void FuncNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                std::unique_ptr<FuncSymbol> derivedSymbol = static_cast<std::unique_ptr<FuncSymbol>>(symbol);
                if (derivedSymbol->getNumParameters() == parameters.size())
                {
                    bool sameFuncFlag = true;
                    for (int i = 0; i < parameters.size(); i++)
                        if (parameters[i]->getType() != derivedSymbol->getParameters()[i])
                        {
                            sameFuncFlag = false;
                            break;
                        }

                    if (sameFuncFlag)
                        throwSemanticError(line, "'" + name + "' has already been declared as a function.");
                }
            }

            if (symbol->getType() == "VAR" && symbol->getName() == name)
                throwSemanticError(line, "'" + name + "' has already been declared as a variable.");
        }
    }

    addSignatureToTable(semanticsInfo);

    if (!emptyFunction)
        body->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
void FuncNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string callName = name;
    if (name == "main")
        callName = "__main__";

    std::string funcName = prefix + callName;
    for (int i = 0; i < parameters.size(); i++)
    {
        funcName += (parameters[i]->isReference()) ? ".REF." + parameters[i]->getType() : "." + parameters[i]->getType();
    }

    ir += "define " + mapTypeIR(returnType) + " @" + funcName + "(";
    llvmInfo.addScope();

    for (int i = 0; i < parameters.size(); i++)
    {
        if (parameters[i]->isReference())
            ir += (i == parameters.size() - 1) ? "ptr %" + std::to_string(i + 1) : "ptr %" + std::to_string(i + 1) + ", ";
        else
            ir += (i == parameters.size() - 1) ? mapTypeIR(parameters[i]->getType()) + " %" + parameters[i]->getName() : mapTypeIR(parameters[i]->getType()) + " %" + parameters[i]->getName() + ", ";
    }

    ir += ") {\nentry:\n";
    for (int i = 0; i < parameters.size(); i++)
    {
        if (!parameters[i]->isReference())
        {
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = alloca " + mapTypeIR(parameters[i]->getType()) + ", align " + mapNumBytesIR(parameters[i]->getType()) + "\n";
            ir += "store " + mapTypeIR(parameters[i]->getType()) + " %" + parameters[i]->getName() + ", ptr %" + std::to_string(llvmInfo.getNextTempVar()) + ", align " + mapNumBytesIR(parameters[i]->getType()) + "\n";
        }
        llvmInfo.addTempVar({parameters[i]->getName(), "ptr"});
    }

    if (!emptyFunction)
    {
        for (std::unique_ptr<Node> node : body->getStatements())
            node->mapToIR(ir, llvmInfo, false, prefix);
    }

    if (returnType == "")
        ir += "ret void\n";
    else
    {
        if (emptyFunction)
            throwSemanticError(line, "An Empty Function can't be declared with a return type.");
    }

    if (ir[ir.size() - 2] == ':')
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    if (returnType != "" && !dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[body->getNumStatements() - 1]))
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    ir += "}\n\n";

    llvmInfo.removeScope();
}
void FuncNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

BlockNode::BlockNode()
{
}
BlockNode::~BlockNode()
{
}
void BlockNode::addStatement(std::unique_ptr<Node> stmt)
{
    statements.push_back(stmt);
}
void BlockNode::clearStatments()
{
    statements.clear();
}
int BlockNode::getNumStatements() const
{
    return statements.size();
}
std::vector<std::unique_ptr<Node>> BlockNode::getStatements() const
{
    return statements;
}
void BlockNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "BlockNode:" << std::endl;
    for (int i = 0; i < statements.size(); i++)
        statements[i]->print(depth + 1);
}
void BlockNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (std::unique_ptr<Node> &statement : statements)
        statement->checkSemantics(semanticsInfo);
}
void BlockNode::mapToGlobalIR(std::string &ir, std::string prefix)
{
    for (std::unique_ptr<Node> &node : statements)
    {
        if (std::unique_ptr<AssignNode> assignment = dynamic_cast<std::unique_ptr<AssignNode>>(node))
        {
            if (!assignment->isDeclared())
                continue;

            if (assignment->getType() == "CHARTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i8 0, align 1\n";
            else if (assignment->getType() == "INTTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i32 0, align 4\n";
            else if (assignment->getType() == "DECIMALTYPE")
                ir += "@" + prefix + assignment->getName() + " = global double 0.0, align 8\n";
            else if (assignment->getType() == "BOOLTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i1 0, align 1\n";
        }
    }

    ir += "\n";
}
void BlockNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    ir += "define i32 @" + prefix + "main() {\nentry:\n";

    for (std::unique_ptr<Node> &node : statements)
        node->mapToIR(ir, llvmInfo, true, prefix);

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::mapImportsToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::vector<std::string> imports)
{
    ir += "define i32 @main() {\nentry:\n";

    for (const std::string &import : imports)
        ir += "call i32 @." + import + ".main()\n";

    for (std::unique_ptr<Node> &node : statements)
        node->mapToIR(ir, llvmInfo, true, "");

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < statements.size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(statements[i]))
        {
            if (i != statements.size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < statements.size(); i++)
        statements[i]->findReturnStmts(stmts);
}

ContainerNode::ContainerNode(std::string name, std::vector<std::unique_ptr<Node>> body, int line)
{
    this->name = name;
    this->line = line;

    for (std::unique_ptr<Node> &node : body)
    {
        if (std::unique_ptr<IdentifierNode> variable = dynamic_cast<std::unique_ptr<IdentifierNode>>(node))
            variables.push_back(variable);

        if (std::unique_ptr<FuncNode> function = dynamic_cast<std::unique_ptr<FuncNode>>(node))
            functions.push_back(function);
    }
}

ContainerNode::~ContainerNode()
{
}

void ContainerNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ContainerNode: " << name << std::endl;
    std::cout << depthTabs << "\tVariables: " << std::endl;
    for (const std::unique_ptr<IdentifierNode> &node : variables)
        node->print(depth + 2);

    std::cout << depthTabs << "\tMethods: " << std::endl;
    for (const std::unique_ptr<FuncNode> &node : functions)
        node->print(depth + 2);
}
void ContainerNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (const std::unique_ptr<Symbol> &symbol : scope.getSymbols())
        {
            if (symbol->getType() == "CONTAINER" && symbol->getName() == name)
                throwSemanticError(line, "'" + name + "' has already been declared as a container.");
        }
    }

    std::vector<std::unique_ptr<VarSymbol>> varSymbols;
    for (std::unique_ptr<IdentifierNode> &identifier : variables)
    {
        for (std::unique_ptr<Symbol> &symbol : varSymbols)
        {
            if (symbol->getName() == identifier->getName() && symbol->getType() == "VAR")
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a variable in container '" + name + "'.");
        }

        varSymbols.push_back(std::make_unique<VarSymbol>(identifier->getLine(), true, identifier->getName(), identifier->getType()));
    }

    bool foundConstructor;
    std::vector<std::unique_ptr<FuncSymbol>> funcSymbols;
    for (std::unique_ptr<FuncNode> &func : functions)
    {
        if (func->getName() == name)
            foundConstructor = true;

        for (std::unique_ptr<Symbol> &symbol : varSymbols)
        {
            if (symbol->getName() == func->getName() && symbol->getType() == "VAR")
                throwSemanticError(func->getLine(), "'" + func->getName() + "' has already been declared as a variable in container '" + name + "'.");
        }

        for (std::unique_ptr<Symbol> &symbol : funcSymbols)
        {
            if (symbol->getName() == func->getName() && symbol->getType() == "FUNC")
            {
                if (symbol->getType() == "FUNC" && symbol->getName() == name)
                {
                    std::unique_ptr<FuncSymbol> derivedSymbol = static_cast<std::unique_ptr<FuncSymbol>>(symbol);
                    if (derivedSymbol->getNumParameters() == func->getParameters().size())
                    {
                        bool sameFuncFlag = true;
                        for (int i = 0; i < func->getParameters().size(); i++)
                        {
                            if (func->getParameters()[i]->getType() != derivedSymbol->getParameters()[i])
                            {
                                sameFuncFlag = false;
                                break;
                            }
                        }

                        if (sameFuncFlag)
                            throwSemanticError(func->getLine(), "'" + func->getName() + "' has already been declared as a function in container '" + name + "' with the same parameters.");
                    }
                }
            }
        }

        if (func->getName() == name)
        {
            if (func->getReturnType() != name && func->getReturnType() != "")
                throwSemanticError(func->getLine(), "'" + func->getName() + "' must return a class object.");

            func->setReturnType(name);
        }

        std::vector<std::string> parameters;
        for (int i = 0; i < func->getParameters().size(); i++)
            parameters.push_back(func->getParameters()[i]->getType());

        funcSymbols.push_back(std::make_unique<FuncSymbol>(func->getLine(), true, func->getName(), func->getReturnType(), true, parameters));
    }

    if (!foundConstructor || functions.size() == 0)
        throwSemanticError(line, "No constructor was found in container '" + name + "'.");

    std::unique_ptr<ContainerSymbol> container = std::make_unique<ContainerSymbol>(line, name, varSymbols, funcSymbols);
    semanticsInfo.addSymbol(semanticsInfo.getVisibleScope()[0], container);
}

IfStmtNode::IfStmtNode(std::unique_ptr<Node> condition, std::unique_ptr<BlockNode> body)
{
    this->condition = condition;
    this->body = body;
}
IfStmtNode::~IfStmtNode()
{
}
void IfStmtNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "IfStatementNode: \n";
    std::cout << depthTabs << "\tCondition:" << std::endl;
    condition->print(depth + 2);
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}
void IfStmtNode::setCurrentWhile(int currentStmt)
{
    for (std::unique_ptr<Node> &stmt : body->getStatements())
    {
        if (std::unique_ptr<BreakNode> breakStmt = dynamic_cast<std::unique_ptr<BreakNode>>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (std::unique_ptr<SkipNode> skipStmt = dynamic_cast<std::unique_ptr<SkipNode>>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (std::unique_ptr<BranchBlockNode> branchBlock = dynamic_cast<std::unique_ptr<BranchBlockNode>>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
void IfStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "If Statement condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();

    for (std::unique_ptr<Node> &stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
bool IfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<BreakNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<SkipNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void IfStmtNode::mapCondition(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    llvmInfo.addTempVar({".ifcondition_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, llvmInfo, global, prefix);
    condition->storeValue(ir, newType, ".ifcondition_" + loadTemp, llvmInfo, false, prefix);
    ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    llvmInfo.addTempVar({".ifnode_" + loadTemp, "ptr"});
}
void IfStmtNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    for (std::unique_ptr<Node> &stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);

    llvmInfo.addTempVar({".endScope", "ptr"});
}
void IfStmtNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

ElseIfStmtNode::ElseIfStmtNode(std::unique_ptr<Node> condition, std::unique_ptr<BlockNode> body)
{
    this->condition = condition;
    this->body = body;
}
ElseIfStmtNode::~ElseIfStmtNode()
{
}
void ElseIfStmtNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ElseIfStatementNode:" << std::endl;
    std::cout << depthTabs << "\tCondition:" << std::endl;
    condition->print(depth + 2);
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}
void ElseIfStmtNode::setCurrentWhile(int currentStmt)
{
    for (std::unique_ptr<Node> &stmt : body->getStatements())
    {
        if (std::unique_ptr<BreakNode> breakStmt = dynamic_cast<std::unique_ptr<BreakNode>>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (std::unique_ptr<SkipNode> skipStmt = dynamic_cast<std::unique_ptr<SkipNode>>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (std::unique_ptr<BranchBlockNode> branchBlock = dynamic_cast<std::unique_ptr<BranchBlockNode>>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseIfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<BreakNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<SkipNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseIfStmtNode::mapCondition(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    llvmInfo.addTempVar({".elseifcondition_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, llvmInfo, global, prefix);
    condition->storeValue(ir, newType, ".elseifcondition_" + loadTemp, llvmInfo, false, prefix);
    ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    llvmInfo.addTempVar({".elseifnode_" + loadTemp, "ptr"});
}
void ElseIfStmtNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    for (std::unique_ptr<Node> stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);
}
void ElseIfStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "Else If Statement condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();
    for (std::unique_ptr<Node> stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeVisibleScope();
    semanticsInfo.removeScope();
}
void ElseIfStmtNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

ElseStmtNode::ElseStmtNode(std::unique_ptr<BlockNode> body)
{
    this->body = body;
}
ElseStmtNode::~ElseStmtNode()
{
}
void ElseStmtNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ElseStatementNode:" << std::endl;
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}
void ElseStmtNode::setCurrentWhile(int currentStmt)
{
    for (std::unique_ptr<Node> &stmt : body->getStatements())
    {
        if (std::unique_ptr<BreakNode> breakStmt = dynamic_cast<std::unique_ptr<BreakNode>>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (std::unique_ptr<SkipNode> skipStmt = dynamic_cast<std::unique_ptr<SkipNode>>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (std::unique_ptr<BranchBlockNode> branchBlock = dynamic_cast<std::unique_ptr<BranchBlockNode>>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<BreakNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<SkipNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    semanticsInfo.addVisibleScope();
    for (std::unique_ptr<Node> &stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
void ElseStmtNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
    llvmInfo.addTempVar({".elsenode_" + loadTemp, "ptr"});

    for (std::unique_ptr<Node> &stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);
}
void ElseStmtNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

int BranchBlockNode::numIfStmt = 0;
BranchBlockNode::BranchBlockNode()
{
    branches = std::make_unique<BlockNode>();
}

BranchBlockNode::~BranchBlockNode()
{
}

void BranchBlockNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "BranchBlockNode:" << std::endl;
    branches->print(depth + 1);
}
int BranchBlockNode::getNumberIf()
{
    return numIfStmt;
}
void BranchBlockNode::addBranch(std::unique_ptr<Node> &branch)
{
    branches->addStatement(branch);
}
void BranchBlockNode::setCurrentWhile(int currentWhile)
{
    for (std::unique_ptr<Node> &stmt : branches->getStatements())
    {
        if (dynamic_cast<std::unique_ptr<IfStmtNode>>(stmt) != nullptr)
        {
            std::unique_ptr<IfStmtNode> ifStmt = static_cast<std::unique_ptr<IfStmtNode>>(stmt);
            ifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast < std::unique_ptr<ElseIfStmtNode>(stmt) != nullptr)
        {
            Elsestd::unique_ptr<IfStmtNode> elseifStmt = static_cast < std::unique_ptr<ElseIfStmtNode>(stmt);
            elseifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast<std::unique_ptr<IfStmtNode>>(stmt) != nullptr)
        {
            std::unique_ptr<IfStmtNode> elseStmt = static_cast<std::unique_ptr<IfStmtNode>>(stmt);
            elseStmt->setCurrentWhile(currentWhile);
        }
    }
}
void BranchBlockNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (std::unique_ptr<Node> &node : branches->getStatements())
        node->checkSemantics(semanticsInfo);
}
void BranchBlockNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    BranchBlockNode::numIfStmt++;
    int numElseIfStmt = 1;
    std::string currentIf = std::to_string(numIfStmt);
    std::string currentElseIf = std::to_string(numElseIfStmt);
    std::string nextElseIf = std::to_string(numElseIfStmt + 1);
    for (int i = 0; i < branches->getNumStatements(); i++)
    {
        std::unique_ptr<Node> node = branches->getStatements()[i];

        if (dynamic_cast<std::unique_ptr<IfStmtNode>>(node) != nullptr)
        {
            std::unique_ptr<IfStmtNode> ifStmt = static_cast<std::unique_ptr<IfStmtNode>>(node);
            ifStmt->mapCondition(ir, llvmInfo, global, prefix);

            if (branches->getNumStatements() == 1)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() == 2 && dynamic_cast<std::unique_ptr<IfStmtNode>>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %if_next_" + currentIf + "_" + currentElseIf + "\n";

            ir += "if_true_" + currentIf + ":\n";
            ifStmt->mapToIR(ir, llvmInfo, global, prefix);

            if (!ifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }

        if (dynamic_cast<std::unique_ptr<ElseIfStmtNode>>(node) != nullptr)
        {
            ir += "if_next_" + currentIf + "_" + currentElseIf + ":\n";
            std::unique_ptr<ElseIfStmtNode> elseifStmt = static_cast<std::unique_ptr<ElseIfStmtNode>>(node);
            elseifStmt->mapCondition(ir, llvmInfo, false, prefix);

            if (i == branches->getNumStatements() - 1)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() >= 3 && dynamic_cast<std::unique_ptr<IfStmtNode>>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_next_" + currentIf + "_" + nextElseIf + "\n";

            ir += "elseif_true_" + currentIf + "_" + currentElseIf + ":\n";
            elseifStmt->mapToIR(ir, llvmInfo, false, prefix);
            if (!elseifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";

            numElseIfStmt++;
            currentElseIf = std::to_string(numElseIfStmt);
            nextElseIf = std::to_string(numElseIfStmt + 1);
        }

        if (dynamic_cast<std::unique_ptr<IfStmtNode>>(node) != nullptr)
        {
            std::unique_ptr<IfStmtNode> elseStmt = static_cast<std::unique_ptr<IfStmtNode>>(node);
            ir += "else_" + currentIf + ":\n";
            elseStmt->mapToIR(ir, llvmInfo, false, prefix);
            if (!elseStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }
    }

    ir += "if_end_" + currentIf + ":\n";
}
void BranchBlockNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < branches->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(branches->getStatements()[i]))
        {
            if (i != branches->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < branches->getStatements().size(); i++)
    {
        branches->getStatements()[i]->findReturnStmts(stmts);
    }
}

int WhileNode::numWhileStmt = 0;
WhileNode::WhileNode(std::unique_ptr<Node> condition, std::unique_ptr<BlockNode> body)
{
    this->condition = condition;
    this->body = body;
}
WhileNode::~WhileNode()
{
}
bool WhileNode::getSkip()
{
    return skip;
}

void WhileNode::setSkip(bool skip)
{
    this->skip = skip;
}
void WhileNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "WhileStatementNode:" << std::endl;
    std::cout << depthTabs << "\tCondition:" << std::endl;
    condition->print(depth + 2);
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}

bool WhileNode::endsWithBranchStmt()
{
    return (dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<BreakNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<std::unique_ptr<SkipNode>>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void WhileNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "While loop condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();
    for (std::unique_ptr<Node> stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
void WhileNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    numWhileStmt++;
    std::string currentCount = std::to_string(WhileNode::numWhileStmt);
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());

    ir += "br label %while_cond_" + currentCount + "\n";
    ir += "while_cond_" + currentCount + ":\n";
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    llvmInfo.addTempVar({".whilenode_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, llvmInfo, global, prefix);
    condition->storeValue(ir, newType, ".whilenode_" + loadTemp, llvmInfo, false, prefix);

    std::string condTemp = std::to_string(llvmInfo.getNextTempVar());
    ir += "%" + condTemp + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    llvmInfo.addTempVar({".whilenode_" + loadTemp, "ptr"});
    ir += "br i1 %" + condTemp + ", label %while_true_" + currentCount + ", label %while_end_" + currentCount + "\n";
    ir += "while_true_" + currentCount + ":\n";

    for (std::unique_ptr<Node> &stmt : body->getStatements())
    {
        if (std::unique_ptr<BranchBlockNode> branchBlock = dynamic_cast<std::unique_ptr<BranchBlockNode>>(stmt))
            branchBlock->setCurrentWhile(numWhileStmt);

        if (std::unique_ptr<BreakNode> breakStmt = dynamic_cast<std::unique_ptr<BreakNode>>(stmt))
        {
            breakStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (std::unique_ptr<SkipNode> skipStmt = dynamic_cast<std::unique_ptr<SkipNode>>(stmt))
        {
            skipStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }
    }

    for (std::unique_ptr<Node> &stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);

    if (!endsWithBranchStmt())
        ir += "br label %while_cond_" + currentCount + "\n";

    ir += "while_end_" + currentCount + ":\n";
}
void WhileNode::findReturnStmts(std::vector<std::unique_ptr<Node>> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (std::unique_ptr<ReturnStmtNode> returnStmt = dynamic_cast<std::unique_ptr<ReturnStmtNode>>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}
