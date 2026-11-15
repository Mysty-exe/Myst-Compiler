#include "compiler/node.h"

ParamNode::ParamNode(std::string identifier, int line)
{
    reference = false;
    this->line = line;
    std::string refStr, typeStr, nameStr;
    for (int i = identifier.size() - 1; i > -1; i--)
    {
        if (identifier[i] == ' ')
        {
            name = identifier.substr(i + 1);

            std::string type = identifier.substr(0, i);
            if (identifier.substr(0, 3) == "ref")
            {
                type = identifier.substr(4, i - 4);
                reference = true;
            }

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

            return;
        }
    }

    throwSyntaxError(line, "Missing parameter type (or name) for '" + identifier + "'");
}
ParamNode::ParamNode(std::string name, std::string type, int line)
{
    this->name = name;
    this->line = line;
    reference = false;

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

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line)
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
        std::vector<Node *> returnStmts;
        findReturnStmts(returnStmts);

        if (returnStmts.size() != 0)
        {
            for (int i = 0; i < returnStmts.size(); i++)
            {
                ReturnStmtNode *returnStmt = static_cast<ReturnStmtNode *>(returnStmts[i]);
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
    for (ParamNode *param : parameters)
    {
        delete param;
        param = nullptr;
    }

    delete body;
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
    for (ParamNode *parameter : parameters)
    {
        if (parameter->isReference())
            parameterTypes.push_back("REF" + parameter->getType());
        else
            parameterTypes.push_back(parameter->getType());
    }

    semanticsInfo.addSymbol(semanticsInfo.getVisibleScope()[0], new FuncSymbol(line, false, name, returnType, false, parameterTypes));
    if (includeParams)
    {
        semanticsInfo.addVisibleScope();
        std::string scope = semanticsInfo.getLatestVisibleScope();
        for (ParamNode *parameter : parameters)
            semanticsInfo.addSymbol(scope, new VarSymbol(parameter->getLine(), false, parameter->getName(), parameter->getType()));
    }
}
void FuncNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
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
        funcName += (parameters[i]->isReference()) ? ".REF" + parameters[i]->getType() : "." + parameters[i]->getType();
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
        for (Node *node : body->getStatements())
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

    if (returnType != "" && !dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]))
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    ir += "}\n\n";

    llvmInfo.removeScope();
}
void FuncNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
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
    for (Node *stmt : statements)
    {
        delete stmt;
        stmt = nullptr;
    }
}
void BlockNode::addStatement(Node *stmt)
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
std::vector<Node *> BlockNode::getStatements() const
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
    for (Node *statement : statements)
        statement->checkSemantics(semanticsInfo);
}
void BlockNode::mapToGlobalIR(std::string &ir, std::string prefix)
{
    for (Node *node : statements)
    {
        if (AssignNode *assignment = dynamic_cast<AssignNode *>(node))
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

    for (Node *node : statements)
        node->mapToIR(ir, llvmInfo, true, prefix);

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::mapImportsToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::vector<std::string> imports)
{
    ir += "define i32 @main() {\nentry:\n";

    for (const std::string &import : imports)
        ir += "call i32 @." + import + ".main()\n";

    for (Node *node : statements)
        node->mapToIR(ir, llvmInfo, true, "");

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < statements.size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(statements[i]))
        {
            if (i != statements.size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < statements.size(); i++)
        statements[i]->findReturnStmts(stmts);
}

IfStmtNode::IfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
IfStmtNode::~IfStmtNode()
{
    delete condition;
    condition = nullptr;
    delete body;
    body = nullptr;
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
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
void IfStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "If Statement condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();

    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
bool IfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
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
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);

    llvmInfo.addTempVar({".endScope", "ptr"});
}
void IfStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
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

ElseIfStmtNode::ElseIfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
ElseIfStmtNode::~ElseIfStmtNode()
{
    delete condition;
    condition = nullptr;
    delete body;
    body = nullptr;
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
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseIfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
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
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);
}
void ElseIfStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "Else If Statement condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeVisibleScope();
    semanticsInfo.removeScope();
}
void ElseIfStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
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

ElseStmtNode::ElseStmtNode(BlockNode *body)
{
    this->body = body;
}
ElseStmtNode::~ElseStmtNode()
{
    delete body;
    body = nullptr;
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
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseStmtNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    semanticsInfo.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(semanticsInfo);

    semanticsInfo.removeScope();
    semanticsInfo.removeVisibleScope();
}
void ElseStmtNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string loadTemp = std::to_string(llvmInfo.getNextTempVar());
    llvmInfo.addTempVar({".elsenode_" + loadTemp, "ptr"});

    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);
}
void ElseStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
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
    branches = new BlockNode();
}

BranchBlockNode::~BranchBlockNode()
{
    delete branches;
    branches = nullptr;
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
void BranchBlockNode::addBranch(Node *branch)
{
    branches->addStatement(branch);
}
void BranchBlockNode::setCurrentWhile(int currentWhile)
{
    for (Node *stmt : branches->getStatements())
    {
        if (dynamic_cast<IfStmtNode *>(stmt) != nullptr)
        {
            IfStmtNode *ifStmt = static_cast<IfStmtNode *>(stmt);
            ifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast<ElseIfStmtNode *>(stmt) != nullptr)
        {
            ElseIfStmtNode *elseifStmt = static_cast<ElseIfStmtNode *>(stmt);
            elseifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast<ElseStmtNode *>(stmt) != nullptr)
        {
            ElseStmtNode *elseStmt = static_cast<ElseStmtNode *>(stmt);
            elseStmt->setCurrentWhile(currentWhile);
        }
    }
}
void BranchBlockNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    for (Node *node : branches->getStatements())
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
        Node *node = branches->getStatements()[i];

        if (dynamic_cast<IfStmtNode *>(node) != nullptr)
        {
            IfStmtNode *ifStmt = static_cast<IfStmtNode *>(node);
            ifStmt->mapCondition(ir, llvmInfo, global, prefix);

            if (branches->getNumStatements() == 1)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() == 2 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %if_true_" + currentIf + ", label %if_next_" + currentIf + "_" + currentElseIf + "\n";

            ir += "if_true_" + currentIf + ":\n";
            ifStmt->mapToIR(ir, llvmInfo, global, prefix);

            if (!ifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }

        if (dynamic_cast<ElseIfStmtNode *>(node) != nullptr)
        {
            ir += "if_next_" + currentIf + "_" + currentElseIf + ":\n";
            ElseIfStmtNode *elseifStmt = static_cast<ElseIfStmtNode *>(node);
            elseifStmt->mapCondition(ir, llvmInfo, false, prefix);

            if (i == branches->getNumStatements() - 1)
                ir += "br i1 %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() >= 3 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
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

        if (dynamic_cast<ElseStmtNode *>(node) != nullptr)
        {
            ElseStmtNode *elseStmt = static_cast<ElseStmtNode *>(node);
            ir += "else_" + currentIf + ":\n";
            elseStmt->mapToIR(ir, llvmInfo, false, prefix);
            if (!elseStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }
    }

    ir += "if_end_" + currentIf + ":\n";
}
void BranchBlockNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < branches->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(branches->getStatements()[i]))
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
WhileNode::WhileNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
WhileNode::~WhileNode()
{
    delete condition;
    delete body;
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
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void WhileNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    if (!condition->checkType(semanticsInfo, "BOOLTYPE"))
        throwSemanticError(line, "While loop condition doesn't evaluate to a boolean.");

    semanticsInfo.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
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

    for (Node *stmt : body->getStatements())
    {
        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(numWhileStmt);

        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }
    }

    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, llvmInfo, global, prefix);

    if (!endsWithBranchStmt())
        ir += "br label %while_cond_" + currentCount + "\n";

    ir += "while_end_" + currentCount + ":\n";
}
void WhileNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
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
