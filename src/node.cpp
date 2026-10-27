#include "compiler/node.h"

void throwLexicalError(int line, std::string message)
{
    try
    {
        throw LexicalError(message);
    }
    catch (const LexicalError &e)
    {
        std::cerr << "\033[31m";
        std::cerr << RootNode::getFileName() << " - Lexical Error on line " << line << ": ";
        std::cerr << "\033[0m";
        std::cout << e.what() << std::endl;
        exit(-1);
    }
}

void throwSyntaxError(int line, std::string message)
{
    try
    {
        throw SyntaxError(message);
    }
    catch (const SyntaxError &e)
    {
        std::cerr << "\033[31m";
        std::cerr << RootNode::getFileName() << " - Syntax Error on line " << line << ": ";
        std::cerr << "\033[0m";
        std::cout << e.what() << std::endl;
        exit(-1);
    }
}

void throwSemanticWarning(int line, std::string message)
{
    std::cout << "\033[38;5;208m";
    std::cout << RootNode::getFileName() << " - Warning on line " << line << ": ";
    std::cerr << "\033[0m";
    std::cout << message << std::endl;
}

void throwSemanticError(int line, std::string message)
{
    try
    {
        throw SemanticError(message);
    }
    catch (const SemanticError &e)
    {
        std::cerr << "\033[31m";
        std::cerr << RootNode::getFileName() << " - Semantic Error on line " << line << ": ";
        std::cerr << "\033[0m";
        std::cout << e.what() << std::endl;
        exit(-1);
    }
}

void checkForDivisionByZero(std::string tempVar, std::string &ir)
{
    ir += "%is_zero = icmp eq i32 " + tempVar + ", 0";
    ir += R"(
br i1 %is_zero, label %div_error, label %div_normal

div_error:
    call void @print_runtime_error(ptr @div_by_zero_msg)
    call void @myst_exit(i64 1)
    unreachable

div_normal:
)";
}

std::string repeat_string(const std::string &input, size_t num)
{
    std::string result;
    result.reserve(input.size() * num);

    while (num--)
    {
        result += input;
    }
    return result;
}

std::string getDataTypeStr(const std::string &str)
{
    if (str == "CHARTYPE")
        return "char";
    if (str == "STRINGTYPE")
        return "string";
    if (str == "INTTYPE")
        return "int";
    if (str == "DECIMALTYPE")
        return "decimal";
    if (str == "BOOLTYPE")
        return "bool";
    if (str == "VAR")
        return "var";

    return "void";
}

std::string mapTypeIR(const std::string &type)
{
    if (type == "CHARTYPE")
        return "i8";
    if (type == "INTTYPE")
        return "i32";
    if (type == "DECIMALTYPE")
        return "double";
    if (type == "BOOLTYPE")
        return "i1";

    return "void";
}

std::string mapNumBytesIR(const std::string &type)
{
    if (type == "CHARTYPE")
        return "1";
    if (type == "INTTYPE")
        return "4";
    if (type == "DECIMALTYPE")
        return "8";
    if (type == "BOOLTYPE")
        return "1";

    return "0";
}

std::string mapOperatorIR(const std::string &op, const std::string &type)
{
    if (op == "+")
    {
        if (type == "INTTYPE")
            return "add nsw";
        if (type == "DECIMALTYPE")
            return "fadd";
    }
    if (op == "-")
    {
        if (type == "INTTYPE")
            return "sub";
        if (type == "DECIMALTYPE")
            return "fsub";
    }
    if (op == "*")
    {
        if (type == "INTTYPE")
            return "mul";
        if (type == "DECIMALTYPE")
            return "fmul";
    }
    if (op == "/")
    {
        if (type == "INTTYPE")
            return "sdiv";
        if (type == "DECIMALTYPE")
            return "fdiv";
    }
    if (op == "%")
    {
        if (type == "INTTYPE")
            return "srem";
        if (type == "DECIMALTYPE")
            return "frem";
    }
    if (op == "==")
    {
        if (type == "DECIMALTYPE")
            return "fcmp eq";
        else
            return "icmp eq";
    }
    if (op == "!=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp ne";
        else
            return "icmp ne";
    }
    if (op == "<")
    {
        if (type == "DECIMALTYPE")
            return "fcmp slt";
        else
            return "icmp slt";
    }
    if (op == "<=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp sle";
        else
            return "icmp sle";
    }
    if (op == ">")
    {
        if (type == "DECIMALTYPE")
            return "fcmp sgt";
        else
            return "icmp sgt";
    }
    if (op == ">=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp sge";
        else
            return "icmp sge";
    }
    if (op == "and" || op == "&&")
        return "and";
    if (op == "or" || op == "||")
        return "or";

    return op;
}

Node::Node()
{
}
int Node::getLine() const
{
    return line;
}

void Node::setLine(int line)
{
    this->line = line;
}

void Node::lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
}

bool Node::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    return true;
}
void Node::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
}
std::string Node::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    return "";
}
void Node::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
void Node::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
void Node::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}

void Node::findReturnStmts(std::vector<Node *> &stmts)
{
}

std::string RootNode::fileName = "";

RootNode::RootNode()
{
}
RootNode::~RootNode()
{
    for (Node *node : children)
    {
        delete node;
        node = nullptr;
    }

    delete globalCode;
    globalCode = nullptr;
}
std::string RootNode::getFileName()
{
    return fileName;
}
void RootNode::setFileName(std::string name)
{
    fileName = name;
}
void RootNode::addNode(Node *node)
{
    children.push_back(node);
}
Node *RootNode::getGlobalCode()
{
    return globalCode;
}
void RootNode::setGlobalCode(Node *block)
{
    globalCode = block;
}
std::vector<Node *> RootNode::getChildren()
{
    return children;
}
std::vector<Node *> &RootNode::getChildrenRef()
{
    return children;
}
void RootNode::print(int depth) const
{
    for (Node *node : children)
    {
        node->print(depth + 1);
        std::cout << std::endl;
    }
}
void RootNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    std::vector<AssignNode *> assignments;
    visibleScope.push_back("1");
    for (Node *child : children)
    {
        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->addSignatureToTable(symbolTable, visibleScope, false);

        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (AssignNode *assignment = dynamic_cast<AssignNode *>(stmt))
                {
                    assignment->checkSemantics(symbolTable, visibleScope);
                    assignments.push_back(assignment);
                }
            }
        }
    }

    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        auto symbols = scope.getSymbols();
        auto it = symbols.begin();
        while (it != symbols.end())
        {
            if (FuncSymbol *symbol = dynamic_cast<FuncSymbol *>(*it))
                it = symbols.erase(it);
            else
                ++it;
        }
        scope.setSymbols(symbols);
        symbolTable[visibleScope[i]] = scope;
    }

    for (Node *child : children)
    {
        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (dynamic_cast<AssignNode *>(stmt) == nullptr)
                    stmt->checkSemantics(symbolTable, visibleScope);
            }
        }

        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->checkSemantics(symbolTable, visibleScope);
    }

    for (AssignNode *assign : assignments)
        assign->lookForCallExprs(symbolTable, visibleScope);
}

void RootNode::sendWarnings(std::unordered_map<std::string, Scope> symbolTable)
{
    for (auto &[_, scope] : symbolTable)
    {
        for (Symbol *symbol : scope.getSymbols())
        {
            if (!symbol->isUsed())
                if (symbol->getType() == "VAR")
                    throwSemanticWarning(symbol->getLine(), "Unused variable '" + symbol->getName() + "' found.");
                else
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    std::string errorMsg = "Unused function " + symbol->getName() + "(";
                    for (int i = 0; i < derivedSymbol->getNumParameters(); i++)
                        if (i == derivedSymbol->getNumParameters() - 1)
                            errorMsg += getDataTypeStr(derivedSymbol->getParameters()[i]);
                        else
                            errorMsg += getDataTypeStr(derivedSymbol->getParameters()[i]) + ", ";
                    throwSemanticWarning(symbol->getLine(), errorMsg + ")");
                }
            if (symbol->getType() == "FUNC")
            {
                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                if (!derivedSymbol->hasReturn() && getDataTypeStr(derivedSymbol->getDataType()) != "void")
                    throwSemanticWarning(derivedSymbol->getLine(), "Function '" + symbol->getName() + "' returns '" + getDataTypeStr(derivedSymbol->getDataType()) + "' but return statement not found.");
            }
        }
    }

    std::cout << std::endl;
}

IdentifierNode::IdentifierNode(std::string name)
{
    this->name = name;
    this->type = TokenType::UNKNOWN;
}
IdentifierNode::~IdentifierNode()
{
}
IdentifierNode::IdentifierNode(std::string name, std::string type)
{
    this->name = name;

    if (type == "char")
        this->type = TokenType::CHARTYPE;
    else if (type == "string")
        this->type = TokenType::STRINGTYPE;
    else if (type == "int")
        this->type = TokenType::INTTYPE;
    else if (type == "decimal")
        this->type = TokenType::DECIMALTYPE;
    else if (type == "bool")
        this->type = TokenType::BOOLTYPE;
    else if (type == "var")
        this->type = TokenType::VAR;
    else
        this->type = TokenType::UNKNOWN;
}
std::string IdentifierNode::getName() const
{
    return name;
}
TokenType IdentifierNode::getType() const
{
    return type;
}
void IdentifierNode::setName(std::string name)
{
    this->name = name;
}
void IdentifierNode::setType(TokenType type)
{
    this->type = type;
}
void IdentifierNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "IdentifierNode: " << getDataTypeStr(Token::getType(type)) << " " << name << std::endl;
}
bool IdentifierNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    for (int i = visibleScope.size() - 1; i >= 0; i--)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
                if (symbol->getDataType() == type)
                    this->type = Token::getTokenDataType(symbol->getDataType());

                return (symbol->getDataType() == type);
            }
        }
    }

    throwSemanticError(line, "'" + name + "' hasn't been declared.");
    return false;
}
std::string IdentifierNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    for (int i = visibleScope.size() - 1; i >= 0; i--)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
                this->type = Token::getTokenDataType(symbol->getDataType());
                return symbol->getDataType();
            }
        }
    }

    throwSemanticError(line, "'" + name + "' hasn't been declared.");
    return "";
}
void IdentifierNode::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    if (std::find(loadedVars.begin(), loadedVars.end(), name) != loadedVars.end())
        return;

    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    if (global)
    {
        ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr @" + name + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({name});
        loadedVars.push_back(name);
        return;
    }

    bool globalVar = true;
    for (int i = 0; i < tempvarMap.size(); i++)
    {
        for (int j = 0; j < tempvarMap[i].size(); j++)
        {
            if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == name)
            {
                globalVar = false;
                ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
                break;
            }
        }
    }

    if (globalVar)
        ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr @" + name + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";

    tempvarMap[tempvarMap.size() - 1].push_back({name});
    loadedVars.push_back(name);
}
void IdentifierNode::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size());

    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        return;
    }

    if (global)
    {
        ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        return;
    }

    bool globalVar = true;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == assignmentName)
            {
                globalVar = false;
                ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
                break;
            }
        }
    }

    if (globalVar)
        ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
}
void IdentifierNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    ir += name;
}

BinaryExprNode::BinaryExprNode(std::string op, Node *left, Node *right)
{
    this->op = op;
    this->left = left;
    this->right = right;
}
BinaryExprNode::~BinaryExprNode()
{
    delete left;
    left = nullptr;
    delete right;
    right = nullptr;
}
void BinaryExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "BinaryExpressionNode: " << op << std::endl;
    std::cout << depthTabs << "Operand Type: " << getDataTypeStr(operandType) << std::endl;
    std::cout << depthTabs << "Result Type: " << getDataTypeStr(resultType) << std::endl;
    std::cout << depthTabs << "\tLeft:\n";
    left->print(depth + 2);
    std::cout << depthTabs << "\tRight:\n";
    right->print(depth + 2);
}
void BinaryExprNode::lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    left->lookForCallExprs(symbolTable, visibleScope);
    right->lookForCallExprs(symbolTable, visibleScope);
}
bool BinaryExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    this->operandType = left->inferType(symbolTable, visibleScope);

    if ((op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=") && type != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }
    if ((op == "+" || op == "-" || op == "*" || op == "/") && (type != "INTTYPE" && type != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }

    this->resultType = type;

    if (op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        std::string leftSide = left->inferType(symbolTable, visibleScope), rightSide = right->inferType(symbolTable, visibleScope);
        if (!((leftSide == "INTTYPE" || leftSide == "DECIMALTYPE") && (rightSide == "INTTYPE" || rightSide == "DECIMALTYPE")))
        {
            throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
            return false;
        }

        return true;
    }

    if (op == "==" || op == "!=")
    {
        if (right->inferType(symbolTable, visibleScope) != left->inferType(symbolTable, visibleScope))
            throwSemanticError(line, "Can't compare these two types with '" + op + "' operator.");

        return right->checkType(symbolTable, visibleScope, left->inferType(symbolTable, visibleScope)) && left->checkType(symbolTable, visibleScope, right->inferType(symbolTable, visibleScope));
    }

    return right->checkType(symbolTable, visibleScope, type) && left->checkType(symbolTable, visibleScope, type);
}
std::string BinaryExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    std::string leftType = left->inferType(symbolTable, visibleScope), rightType = right->inferType(symbolTable, visibleScope);
    if (leftType != rightType)
    {
        throwSemanticError(line, "Mismatched types found with '" + op + "' operator.");
        return "";
    }

    if ((op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=") && leftType != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if ((op == "+" || op == "-" || op == "*" || op == "/") && (leftType != "INTTYPE" && leftType != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if (op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        return "BOOLTYPE";
    }

    return leftType;
}
void BinaryExprNode::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    left->loadVariables(ir, loadedVars, tempvarMap, global);
    right->loadVariables(ir, loadedVars, tempvarMap, global);
}
void BinaryExprNode::traverseBinary(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap)
{
    if (dynamic_cast<BinaryExprNode *>(left) != nullptr || dynamic_cast<BinaryExprNode *>(right) != nullptr)
    {
        if (dynamic_cast<BinaryExprNode *>(left) != nullptr && dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            BinaryExprNode *leftIdentifier, *rightIdentifier;
            leftIdentifier = static_cast<BinaryExprNode *>(left);
            rightIdentifier = static_cast<BinaryExprNode *>(right);
            leftIdentifier->traverseBinary(ir, tempvarMap);
            rightIdentifier->traverseBinary(ir, tempvarMap);

            if (op == "/")
                checkForDivisionByZero("%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()), ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1) + ", i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1) + ", double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1) + ", %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1), std::to_string(tempvarMap.size())});
        }
        else if (dynamic_cast<BinaryExprNode *>(left) != nullptr)
        {
            BinaryExprNode *leftIdentifier;
            leftIdentifier = static_cast<BinaryExprNode *>(left);
            leftIdentifier->traverseBinary(ir, tempvarMap);

            std::string r;
            if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
            {
                rightIdentifier->traverseUnary(ir, tempvarMap);
                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
            {
                rightIdentifier->mapToIR(ir, tempvarMap, false);
                for (int i = tempvarMap.size() - 1; i >= 0; i--)
                {
                    for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                        {
                            r = "%" + std::to_string(j + 1);
                            break;
                        }
                    }
                }
            }
            else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
            {
                for (int i = tempvarMap.size() - 1; i >= 0; i--)
                {
                    for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                        {
                            r = "%" + std::to_string(j + 1);
                            break;
                        }
                    }
                }
            }
            else
            {
                right->mapToIR(r, tempvarMap);
            }

            if (op == "/")
                checkForDivisionByZero(r, ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", i32 " + r + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", double " + r + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + "  %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", " + r + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, std::to_string(tempvarMap[tempvarMap.size() - 1].size()), r});
        }
        else if (dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            BinaryExprNode *rightIdentifier;
            rightIdentifier = static_cast<BinaryExprNode *>(right);
            rightIdentifier->traverseBinary(ir, tempvarMap);

            std::string l;
            if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
            {
                leftIdentifier->traverseUnary(ir, tempvarMap);
                l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
            {
                leftIdentifier->mapToIR(ir, tempvarMap, false);
                for (int i = tempvarMap.size() - 1; i >= 0; i--)
                {
                    for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == leftIdentifier->getName() && l.size() == 0)
                        {
                            l = "%" + std::to_string(j + 1);
                            break;
                        }
                    }
                }
            }
            else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
            {
                for (int i = tempvarMap.size() - 1; i >= 0; i--)
                {
                    for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == leftIdentifier->getName() && l.size() == 0)
                        {
                            l = "%" + std::to_string(j + 1);
                            break;
                        }
                    }
                }
            }
            else
                left->mapToIR(l, tempvarMap);

            if (op == "/")
                checkForDivisionByZero("%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()), ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 " + l + ", i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double " + l + ", double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, l, std::to_string(tempvarMap[tempvarMap.size() - 1].size())});
        }
    }
    else
    {
        std::string l, r;

        if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
        {
            leftIdentifier->traverseUnary(ir, tempvarMap);
            l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
        {
            leftIdentifier->mapToIR(ir, tempvarMap, false);
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == leftIdentifier->getName() && l.size() == 0)
                    {
                        l = "%" + std::to_string(j + 1);
                        break;
                    }
                }
            }
        }
        else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
        {
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == leftIdentifier->getName() && l.size() == 0)
                    {
                        l = "%" + std::to_string(j + 1);
                        break;
                    }
                }
            }
        }
        else
        {
            left->mapToIR(l, tempvarMap);
        }

        if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
        {
            rightIdentifier->traverseUnary(ir, tempvarMap);
            r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
        {
            rightIdentifier->mapToIR(ir, tempvarMap, false);
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                    {
                        r = "%" + std::to_string(j + 1);
                        break;
                    }
                }
            }
        }
        else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
        {
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                    {
                        r = "%" + std::to_string(j + 1);
                        break;
                    }
                }
            }
        }
        else
        {
            right->mapToIR(r, tempvarMap);
        }

        if (op == "/")
            checkForDivisionByZero(r, ir);
        if (op == "^")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 " + l + ", i32 " + r + ")\n";
            else if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double " + l + ", double " + r + ")\n";
        }
        else
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", " + r + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, l, r});
    }
}
void BinaryExprNode::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    traverseBinary(ir, tempvarMap);
    bool globalVar = true;
    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    if (global)
    {
        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    for (int i = 0; i < tempvarMap.size(); i++)
    {
        for (int j = 0; j < tempvarMap[i].size(); j++)
        {
            if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == assignmentName)
            {
                globalVar = false;
                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
                break;
            }
        }
    }

    if (globalVar)
        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
}
void BinaryExprNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}

UnaryExprNode::UnaryExprNode(std::string op, Node *right)
{
    this->op = op;
    this->right = right;
}
UnaryExprNode::~UnaryExprNode()
{
    delete right;
    right = nullptr;
}
void UnaryExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "UnaryExpressionNode: " << op << std::endl;
    std::cout << depthTabs << "Operand Type: " << getDataTypeStr(operandType) << std::endl;
    std::cout << depthTabs << "Result Type: " << getDataTypeStr(resultType) << std::endl;
    std::cout << depthTabs << "\tRight:" << std::endl;
    right->print(depth + 2);
}
void UnaryExprNode::lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    right->lookForCallExprs(symbolTable, visibleScope);
}
bool UnaryExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    this->operandType = right->inferType(symbolTable, visibleScope);

    if ((op == "!" || op == "not") && type != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }
    if ((op == "+" || op == "-") && (type != "INTTYPE" && type != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }

    this->resultType = type;

    return right->checkType(symbolTable, visibleScope, type);
}
std::string UnaryExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    std::string rightType = right->inferType(symbolTable, visibleScope);
    if ((op == "!" || op == "not") && rightType != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if ((op == "+" || op == "-") && (rightType != "INTTYPE" && rightType != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }

    return rightType;
}
void UnaryExprNode::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    right->loadVariables(ir, loadedVars, tempvarMap, global);
}
void UnaryExprNode::traverseUnary(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap)
{
    if (dynamic_cast<BinaryExprNode *>(right) != nullptr || dynamic_cast<UnaryExprNode *>(right) != nullptr)
    {
        if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
            rightIdentifier->traverseUnary(ir, tempvarMap);
        if (BinaryExprNode *rightIdentifier = dynamic_cast<BinaryExprNode *>(right))
            rightIdentifier->traverseBinary(ir, tempvarMap);

        if (op == "-")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sub " + mapTypeIR(operandType) + " 0, %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";
            if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fneg " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";
        }
        else if (op != "+")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = xor " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", true\n";
        }
        tempvarMap[tempvarMap.size() - 1].push_back({op, "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size())});
    }
    else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
    {
        std::string r;
        for (int i = tempvarMap.size() - 1; i >= 0; i--)
        {
            for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
            {
                if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                {
                    r = "%" + std::to_string(j + 1);
                    break;
                }
            }
        }

        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = neg " + mapTypeIR(operandType) + " " + r + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
    else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
    {
        std::string r;
        for (int i = tempvarMap.size() - 1; i >= 0; i--)
        {
            for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
            {
                if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == rightIdentifier->getName() && r.size() == 0)
                {
                    r = "%" + std::to_string(j + 1);
                    break;
                }
            }
        }

        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = neg " + mapTypeIR(operandType) + " " + r + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
    else
    {
        std::string r;
        right->mapToIR(r, tempvarMap);

        ir += "%" + std::to_string(tempvarMap.size() + 1) + " = neg " + mapTypeIR(operandType) + " " + r + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
}
void UnaryExprNode::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    traverseUnary(ir, tempvarMap);
    bool globalVar = true;
    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    for (int i = 0; i < tempvarMap.size(); i++)
    {
        for (int j = 0; j < tempvarMap[i].size(); j++)
        {
            if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == assignmentName)
            {
                globalVar = false;
                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
                break;
            }
        }
    }

    if (globalVar)
        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
}
void UnaryExprNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}

StmtNode::StmtNode()
{
}
StmtNode::~StmtNode()
{
}

ParamNode::ParamNode(std::string identifier, int line)
{
    this->line = line;
    for (int i = identifier.size() - 1; i > -1; i--)
    {
        if (identifier[i] == ' ')
        {
            name = identifier.substr(i + 1);

            std::string type = identifier.substr(0, i);
            if (type == "char")
                this->type = TokenType::CHARTYPE;
            else if (type == "string")
                this->type = TokenType::STRINGTYPE;
            else if (type == "int")
                this->type = TokenType::INTTYPE;
            else if (type == "decimal")
                this->type = TokenType::DECIMALTYPE;
            else if (type == "bool")
                this->type = TokenType::BOOLTYPE;
            else
                throwSyntaxError(line, "Unexpected type for '" + name + "' found.");

            return;
        }
    }

    throwSyntaxError(line, "Missing parameter type (or name) for '" + identifier + "'");
}
ParamNode::~ParamNode()
{
}
ParamNode::ParamNode(std::string name, std::string type, int line)
{
    this->name = name;
    this->line = line;

    if (type == "char")
        this->type = TokenType::CHARTYPE;
    if (type == "string")
        this->type = TokenType::STRINGTYPE;
    if (type == "int")
        this->type = TokenType::INTTYPE;
    if (type == "decimal")
        this->type = TokenType::DECIMALTYPE;
    if (type == "bool")
        this->type = TokenType::BOOLTYPE;
}
void ParamNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
std::string ParamNode::getName() const
{
    return name;
}
TokenType ParamNode::getType() const
{
    return type;
}
void ParamNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ParameterNode: " << getDataTypeStr(Token::getType(type)) << " " << name << std::endl;
}

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
    this->line = line;

    if (returnType == "char")
        this->returnType = TokenType::CHARTYPE;
    if (returnType == "string")
        this->returnType = TokenType::STRINGTYPE;
    if (returnType == "int")
        this->returnType = TokenType::INTTYPE;
    if (returnType == "decimal")
        this->returnType = TokenType::DECIMALTYPE;
    if (returnType == "bool")
        this->returnType = TokenType::BOOLTYPE;
    if (returnType == "void")
        this->returnType = TokenType::UNKNOWN;

    std::vector<Node *> returnStmts;
    findReturnStmts(returnStmts);

    if (returnStmts.size() != 0)
    {
        for (int i = 0; i < returnStmts.size(); i++)
        {
            ReturnStmtNode *returnStmt = static_cast<ReturnStmtNode *>(returnStmts[i]);
            returnStmt->setType(Token::getType(this->returnType));
        }
    }
    else
    {
        if (Token::getType(this->returnType) != "UNKNOWN")
            throwSemanticError(line, "Return Statement not found in function.");
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
    std::cout << depthTabs << "\tReturn Type: " << getDataTypeStr(Token::getType(returnType)) << std::endl;
    std::cout << depthTabs << "\tParameters: " << std::endl;
    for (int i = 0; i < parameters.size(); i++)
        parameters[i]->print(depth + 2);
    body->print(depth + 1);
}
void FuncNode::addSignatureToTable(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, bool includeParams)
{
    std::vector<std::string> parameterTypes;
    for (ParamNode *parameter : parameters)
        parameterTypes.push_back(Token::getType(parameter->getType()));

    symbolTable[visibleScope[0]].addSymbol(new FuncSymbol(line, false, name, Token::getType(returnType), false, parameterTypes));
    if (includeParams)
    {
        visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
        for (ParamNode *parameter : parameters)
            symbolTable[visibleScope[visibleScope.size() - 1]].addSymbol(new VarSymbol(parameter->getLine(), false, parameter->getName(), Token::getType(parameter->getType())));
    }
}
void FuncNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                if (derivedSymbol->getNumParameters() == parameters.size())
                {
                    bool sameFuncFlag = true;
                    for (int i = 0; i < parameters.size(); i++)
                        if (Token::getType(parameters[i]->getType()) != derivedSymbol->getParameters()[i])
                        {
                            sameFuncFlag = false;
                            break;
                        }

                    if (sameFuncFlag)
                        throwSemanticError(line, "'" + name + "' has already been declared as a function.");
                }
            }
        }
    }

    addSignatureToTable(symbolTable, visibleScope);

    body->checkSemantics(symbolTable, visibleScope);

    symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    visibleScope.pop_back();
}
void FuncNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string funcName = name;
    for (int i = 0; i < parameters.size(); i++)
        funcName += "." + Token::getType(parameters[i]->getType());

    ir += "define " + mapTypeIR(Token::getType(returnType)) + " @" + funcName + "(";
    tempvarMap.push_back({});

    for (int i = 0; i < parameters.size(); i++)
    {
        if (i == parameters.size() - 1)
            ir += mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName();
        else
            ir += mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName() + ", ";
    }

    ir += ") {\n";
    for (int i = 0; i < parameters.size(); i++)
    {
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = alloca " + mapTypeIR(Token::getType(parameters[i]->getType())) + ", align " + mapNumBytesIR(Token::getType(parameters[i]->getType())) + "\n";
        ir += "store " + mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName() + ", ptr %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + ", align " + mapNumBytesIR(Token::getType(parameters[i]->getType())) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({parameters[i]->getName()});
    }

    for (Node *node : body->getStatements())
        node->mapToIR(ir, tempvarMap, false);

    if (Token::getType(returnType) == "UNKNOWN")
        ir += "ret void\n";

    if (ir[ir.size() - 2] == ':')
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    if (Token::getType(returnType) != "UNKNOWN" && !dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]))
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    ir += "}\n\n";

    tempvarMap.pop_back();
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

CallExprNode::CallExprNode(std::string name, std::vector<Node *> arguments)
{
    this->name = name;
    this->arguments = arguments;
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
void CallExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "CallExpressionNode: " << name << std::endl;
    std::cout << depthTabs << "\tArguments: " << std::endl;
    for (int i = 0; i < arguments.size(); i++)
        arguments[i]->print(depth + 2);
}
void CallExprNode::lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    argTypes = {};

    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                if (returnType == "" || symbol->getDataType() == returnType)
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(symbolTable, visibleScope, derivedSymbol->getParameters()[k]))
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
bool CallExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    this->returnType = type;
    argTypes = {};

    bool found = true;
    if (name == "print" || name == "input" || name == "exit")
    {
        for (int i = 0; i < Token::builtInFunctions.size(); i++)
        {
            found = true;
            if (Token::builtInFunctions[i][0] != name)
                continue;
            else
            {
                for (int j = 1; j < Token::builtInFunctions[i].size(); j++)
                {
                    std::string currentType = arguments[j - 1]->inferType(symbolTable, visibleScope);
                    if ((Token::builtInFunctions[i][j] == "var" || Token::builtInFunctions[i][j] == currentType) && arguments.size() == Token::builtInFunctions[i].size() - 1)
                    {
                        argTypes.push_back(currentType);
                        continue;
                    }
                    else
                        found = false;
                }

                for (Node *arg : arguments)
                    arg->checkType(symbolTable, visibleScope, arg->inferType(symbolTable, visibleScope));

                if (found)
                    return true;
            }
        }
    }

    if (name == "print" || name == "input" || name == "exit")
        throwSemanticError(line, "function '" + name + "' has the wrong arguments passed in.");

    bool overridenFunction = false;
    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                overridenFunction = true;
                if (type == "" || symbol->getDataType() == type)
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(symbolTable, visibleScope, derivedSymbol->getParameters()[k]))
                        {
                            wrongType = true;
                            break;
                        }
                    }

                    if (wrongType)
                        continue;
                    else
                    {
                        symbol->setUsed(true);
                        return true;
                    }
                }
                else
                    return false;
            }
        }
    }

    if (overridenFunction)
        throwSemanticError(line, "Coulnd't find proper function signature for '" + name + "' call expression.");

    throwSemanticError(line, "function '" + name + "' hasn't been declared.");
    return false;
}
void CallExprNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    checkType(symbolTable, visibleScope, "");
}

std::string CallExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (name == "print" || name == "input" || name == "exit")
        return "void";

    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
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
void CallExprNode::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
void CallExprNode::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    mapToIR(ir, tempvarMap, global);

    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size());

    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";
        return;
    }

    if (global)
    {
        ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";
        return;
    }

    bool globalVar = true;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == assignmentName)
            {
                globalVar = false;
                ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(returnType) + "\n";
                break;
            }
        }
    }

    if (globalVar)
        ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";

    tempvarMap[tempvarMap.size() - 1].push_back({name});
}
void CallExprNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string callName = name;
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
    else if (name == "exit")
        callName = "myst_exit";
    else if (name == "input")
        callName = name;

    std::vector<std::string> args;
    for (int i = 0; i < arguments.size(); i++)
    {
        std::string callArg = ".callarg_" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
        std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = alloca " + mapTypeIR(argTypes[i]) + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({callArg});

        std::vector<std::string> loadedVars;
        arguments[i]->loadVariables(ir, loadedVars, tempvarMap, global);

        arguments[i]->storeValue(ir, callArg, tempvarMap, false);
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load " + mapTypeIR(argTypes[i]) + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
        args.push_back("%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1));
        tempvarMap[tempvarMap.size() - 1].push_back({callArg});
    }

    if (returnType != "")
    {
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = call " + mapTypeIR(returnType) + " @" + callName + "(";
        tempvarMap[tempvarMap.size() - 1].push_back({name});
    }
    else
        ir += "call " + mapTypeIR(returnType) + " @" + callName + "(";

    for (int i = 0; i < args.size(); i++)
    {
        if (i == args.size() - 1)
            ir += mapTypeIR(argTypes[i]) + " " + args[i];
        else
            ir += mapTypeIR(argTypes[i]) + " " + args[i] + ", ";
    }
    ir += ")\n";
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
    return Token::getType(identifier->getType());
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
    std::cout << depthTabs << "\tLeft Side: " << getDataTypeStr(Token::getType(identifier->getType())) << " " << identifier->getName() << std::endl;
    std::cout << depthTabs << "\tRight Side: " << std::endl;
    value->print(depth + 2);
}
void AssignNode::lookForCallExprs(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    value->lookForCallExprs(symbolTable, visibleScope);
}
void AssignNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (identifier->getType() != TokenType::UNKNOWN)
    {
        auto scope = symbolTable[visibleScope[visibleScope.size() - 1]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName() && !(visibleScope.size() > 1))
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared.");
        }

        if (identifier->getType() == TokenType::VAR)
        {
            std::string t = value->inferType(symbolTable, visibleScope);
            identifier->setType(Token::getTokenDataType(t));
            value->checkType(symbolTable, visibleScope, Token::getType(identifier->getType()));
            symbolTable[visibleScope[visibleScope.size() - 1]].addSymbol(new VarSymbol(line, false, identifier->getName(), t));
        }
        else if (value->checkType(symbolTable, visibleScope, Token::getType(identifier->getType())))
            symbolTable[visibleScope[visibleScope.size() - 1]].addSymbol(new VarSymbol(line, false, identifier->getName(), Token::getType(identifier->getType())));
        else
            throwSemanticError(line, "Value in '" + identifier->getName() + "' must have type '" + getDataTypeStr(Token::getType(identifier->getType())) + "'.");
    }
    else
    {
        for (int i = 0; i < visibleScope.size(); i++)
        {
            auto scope = symbolTable[visibleScope[i]];
            for (Symbol *symbol : scope.getSymbols())
            {
                if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName())
                {
                    if (value->checkType(symbolTable, visibleScope, symbol->getDataType()))
                    {
                        identifier->setType(Token::getTokenDataType(symbol->getDataType()));
                        return;
                    }
                    else
                        throwSemanticError(line, "'" + identifier->getName() + "' has already been declared with type '" + getDataTypeStr(symbol->getDataType()) + "'.");
                }
            }
        }

        throwSemanticError(line, "'" + identifier->getName() + "' hasn't been declared.");
    }
}
void AssignNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    if (!global && declared)
    {
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = alloca " + mapTypeIR(Token::getType(identifier->getType())) + ", align " + mapNumBytesIR(Token::getType(identifier->getType())) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({identifier->getName()});
    }

    std::vector<std::string> loadedVars;
    value->loadVariables(ir, loadedVars, tempvarMap, global);
    value->storeValue(ir, identifier->getName(), tempvarMap, global);
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
void BlockNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    for (Node *statement : statements)
        statement->checkSemantics(symbolTable, visibleScope);
}
void StmtNode::print(int depth) const
{
    std::cout << "StatementNode: \n";
}
void BlockNode::mapToGlobalIR(std::string &ir)
{
    for (Node *node : statements)
    {
        if (AssignNode *assignment = dynamic_cast<AssignNode *>(node))
        {
            if (!assignment->isDeclared())
                continue;
            if (assignment->getType() == "CHARTYPE")
                ir += "@" + assignment->getName() + " = global i8 0, align 1\n";
            if (assignment->getType() == "INTTYPE")
                ir += "@" + assignment->getName() + " = global i32 0, align 4\n";
            if (assignment->getType() == "DECIMALTYPE")
                ir += "@" + assignment->getName() + " = global double 0.0, align 8\n";
            if (assignment->getType() == "BOOLTYPE")
                ir += "@" + assignment->getName() + " = global i1 0, align 1\n";
        }
    }

    ir += "\n";
}
void BlockNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    ir += "define i32 @main() {\n";

    for (Node *node : statements)
        node->mapToIR(ir, tempvarMap, true);

    ir += "ret i32 0\n";
    ir += "}";
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
    {
        statements[i]->findReturnStmts(stmts);
    }
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

        if (SkipNode *breakStmt = dynamic_cast<SkipNode *>(stmt))
        {
            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
void IfStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "If Statement condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);

    symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    visibleScope.pop_back();
}
bool IfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void IfStmtNode::mapCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".ifnode_" + loadTemp});

    condition->loadVariables(ir, loadedVars, tempvarMap, global);
    condition->storeValue(ir, ".ifnode_" + loadTemp, tempvarMap, false);
    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".ifnode_" + loadTemp});
}
void IfStmtNode::mapToIRInLoop(Node *loop, std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    for (Node *stmt : body->getStatements())
    {
        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->mapToIRInLoop(loop, ir, tempvarMap, global);
        else
            stmt->mapToIR(ir, tempvarMap, global);
    }

    if (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]))
    {
        WhileNode *loopNode = static_cast<WhileNode *>(loop);
        loopNode->setSkip(true);
    }
}
void IfStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempvarMap, global);
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

        if (SkipNode *breakStmt = dynamic_cast<SkipNode *>(stmt))
        {
            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseIfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseIfStmtNode::mapCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".elseifnode_" + loadTemp});

    condition->loadVariables(ir, loadedVars, tempvarMap, global);
    condition->storeValue(ir, ".elseifnode_" + loadTemp, tempvarMap, false);
    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".elseifnode_" + loadTemp});
}
void ElseIfStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempvarMap, global);
}
void ElseIfStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "Else If Statement condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);

    symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    visibleScope.pop_back();
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

        if (SkipNode *breakStmt = dynamic_cast<SkipNode *>(stmt))
        {
            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);

    symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    visibleScope.pop_back();
}
void ElseStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempvarMap, global);
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
void BranchBlockNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    for (Node *node : branches->getStatements())
        node->checkSemantics(symbolTable, visibleScope);
}
void BranchBlockNode::mapToIRInLoop(Node *loop, std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
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
            ifStmt->mapCondition(ir, tempvarMap, global);

            if (branches->getNumStatements() == 1)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() == 2 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %if_next_" + currentIf + "_" + currentElseIf + "\n";

            ir += "if_true_" + currentIf + ":\n";
            if (loop != nullptr)
                ifStmt->mapToIRInLoop(loop, ir, tempvarMap, global);
            else
                ifStmt->mapToIR(ir, tempvarMap, global);

            if (!ifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }

        if (dynamic_cast<ElseIfStmtNode *>(node) != nullptr)
        {
            ir += "if_next_" + currentIf + "_" + currentElseIf + ":\n";
            ElseIfStmtNode *elseifStmt = static_cast<ElseIfStmtNode *>(node);
            elseifStmt->mapCondition(ir, tempvarMap, false);

            if (i == branches->getNumStatements() - 1)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() >= 3 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_next_" + currentIf + "_" + nextElseIf + "\n";

            ir += "elseif_true_" + currentIf + "_" + currentElseIf + ":\n";
            elseifStmt->mapToIR(ir, tempvarMap, false);
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
            elseStmt->mapToIR(ir, tempvarMap, false);
            if (!elseStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }
    }

    ir += "if_end_" + currentIf + ":\n";
}
void BranchBlockNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    mapToIRInLoop(nullptr, ir, tempvarMap, global);
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
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void WhileNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "While loop condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);

    symbolTable[visibleScope[visibleScope.size() - 1]] = {};
    visibleScope.pop_back();
}
void WhileNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    numWhileStmt++;
    std::string currentCount = std::to_string(WhileNode::numWhileStmt);
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".whilenode_" + loadTemp});

    condition->loadVariables(ir, loadedVars, tempvarMap, global);
    condition->storeValue(ir, ".whilenode_" + loadTemp, tempvarMap, false);

    std::string condTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + condTemp + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".whilenode_" + loadTemp});
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

        if (SkipNode *breakStmt = dynamic_cast<SkipNode *>(stmt))
        {
            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }
    }

    for (Node *stmt : body->getStatements())
    {
        if (skip)
            break;

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->mapToIRInLoop(this, ir, tempvarMap, global);
        else
            stmt->mapToIR(ir, tempvarMap, global);
    }

    if (!endsWithBranchStmt())
        checkCondition(ir, tempvarMap, global, numWhileStmt, loadTemp);

    ir += "while_end_" + currentCount + ":\n";
}
void WhileNode::checkCondition(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global, int currentWhile, std::string loadTemp)
{
    std::vector<std::string> loadedVars;
    condition->loadVariables(ir, loadedVars, tempvarMap, global);
    condition->storeValue(ir, ".whilenode_" + loadTemp, tempvarMap, false);

    std::string condTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + condTemp + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".whilenode_" + loadTemp});

    ir += "br i1 %" + condTemp + ", label %while_true_" + std::to_string(currentWhile) + ", label %while_end_" + std::to_string(currentWhile) + "\n";
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
void ReturnStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC")
            {
                if (identifier == nullptr && returnType != "UNKNOWN")
                    throwSemanticError(line, "Return statement found without an identifier.");

                if (returnType == "UNKNOWN" && identifier != nullptr)
                    throwSemanticError(line, "Return statement found in a function that doesn't have a return type.");

                if (identifier != nullptr && !identifier->checkType(symbolTable, visibleScope, returnType))
                    throwSemanticError(line, "Return statement doesn't return the required return type.");

                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                derivedSymbol->setHasReturnFlag(true);
            }
        }
    }
}
void ReturnStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    std::string type = mapTypeIR(returnType);
    std::vector<std::string> loadedVars;

    if (identifier == nullptr)
        ir += "ret void\n";
    else
    {
        std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
        ir += "%" + loadTemp + " = alloca " + mapTypeIR(returnType) + ", align " + mapNumBytesIR(returnType) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({".return_" + loadTemp});

        identifier->loadVariables(ir, loadedVars, tempvarMap, false);
        identifier->storeValue(ir, ".return_" + loadTemp, tempvarMap, false);

        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load " + type + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(returnType) + "\n";
        ir += "ret " + type + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({".return_" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1)});
    }
}
void ReturnStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    stmts.push_back(this);
}

BreakNode::BreakNode()
{
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
void BreakNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    ir += "br label %while_end_" + std::to_string(currentWhileStmt) + "\n";
}

SkipNode::SkipNode()
{
}
SkipNode::~SkipNode()
{
}
void SkipNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Skip Statement\n";
}
void SkipNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
