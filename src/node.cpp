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
        std::cerr << "Lexical Error on line " << line << ": ";
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
        std::cerr << "Syntax Error on line " << line << ": ";
        std::cerr << "\033[0m";
        std::cout << e.what() << std::endl;
        exit(-1);
    }
}

void throwSemanticWarning(int line, std::string message)
{
    std::cout << "\033[38;5;208m";
    std::cout << "Warning on line " << line << ": ";
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
        std::cerr << "Semantic Error on line " << line << ": ";
        std::cerr << "\033[0m";
        std::cout << e.what() << std::endl;
        exit(-1);
    }
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

bool Node::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    return true;
}
void Node::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
}
std::string Node::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
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
    visibleScope.push_back("1");
    for (Node *child : children)
    {
        child->checkSemantics(symbolTable, visibleScope);
    }
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
                if (!derivedSymbol->hasReturn())
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
    for (int i = 0; i < visibleScope.size(); i++)
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
std::string IdentifierNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
{
    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
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
                ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
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
    }

    if (op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        if (right->inferType(symbolTable, visibleScope) != left->inferType(symbolTable, visibleScope))
            throwSemanticError(line, "Can't compare these two types with '" + op + "' operator.");

        return type == "BOOLTYPE";
    }

    return right->checkType(symbolTable, visibleScope, type) && left->checkType(symbolTable, visibleScope, type);
}
std::string BinaryExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
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
        return "BOOLTYPE";

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

            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1) + ", %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";
            tempvarMap[tempvarMap.size() - 1].push_back({op, std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1), std::to_string(tempvarMap.size())});
        }
        else if (dynamic_cast<BinaryExprNode *>(left) != nullptr)
        {
            BinaryExprNode *leftIdentifier;
            leftIdentifier = static_cast<BinaryExprNode *>(left);
            leftIdentifier->traverseBinary(ir, tempvarMap);

            std::string r;
            if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(left))
            {
                rightIdentifier->traverseUnary(ir, tempvarMap);
                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
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

            ir += "%" + std::to_string(tempvarMap.size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", %" + std::to_string(tempvarMap.size()) + "\n";
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
        else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
        {
            for (int i = tempvarMap.size() - 1; i >= 0; i--)
            {
                for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                {
                    if (tempvarMap[i][j].size() == 1 && tempvarMap[i][j][0] == leftIdentifier->getName() && r.size() == 0)
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
std::string UnaryExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
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

ParamNode::ParamNode(std::string identifier)
{
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
ParamNode::ParamNode(std::string name, std::string type)
{
    this->name = name;

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

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;

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

    std::vector<std::string> parameterTypes;
    for (ParamNode *parameter : parameters)
        parameterTypes.push_back(Token::getType(parameter->getType()));

    symbolTable[visibleScope[visibleScope.size() - 1]].addSymbol(new FuncSymbol(line, false, name, Token::getType(returnType), false, parameterTypes));
    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (ParamNode *parameter : parameters)
        symbolTable[visibleScope[visibleScope.size() - 1]].addSymbol(new VarSymbol(line, false, parameter->getName(), Token::getType(parameter->getType())));
    body->checkSemantics(symbolTable, visibleScope);
    visibleScope.pop_back();
}
void FuncNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    ir += "define " + mapTypeIR(Token::getType(returnType)) + " @" + name + "(";
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

    for (Node *stmt : body->getStatements())
    {
        if (AssignNode *assignment = dynamic_cast<AssignNode *>(stmt))
        {
            if (!assignment->isDeclared())
                continue;
            ir += "%" + assignment->getName() + " = alloca " + mapTypeIR(assignment->getType()) + ", align " + mapNumBytesIR(assignment->getType()) + "\n";
        }

        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(stmt))
        {
            returnStmt->setType(Token::getType(returnType));
            ir += "%.return = alloca " + mapTypeIR(Token::getType(returnType)) + ", align " + mapNumBytesIR(Token::getType(returnType)) + "\n";
        }
    }

    for (Node *node : body->getStatements())
        node->mapToIR(ir, tempvarMap, false);

    if (Token::getType(returnType) == "UNKNOWN")
        ir += "ret void";

    ir += "}\n\n";

    tempvarMap.pop_back();
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
void CallExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "CallExpressionNode: " << name << std::endl;
    std::cout << depthTabs << "\tArguments: " << std::endl;
    for (int i = 0; i < arguments.size(); i++)
        arguments[i]->print(depth + 2);
}
bool CallExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
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
                        if (arguments[k]->checkType(symbolTable, visibleScope, derivedSymbol->getParameters()[k]))
                        {
                            symbol->setUsed(true);
                            return true;
                        }
                        else
                        {
                            wrongType = true;
                            continue;
                        }
                    }

                    if (wrongType)
                        continue;
                    ;
                    return true;
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

std::string CallExprNode::inferType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope) const
{
    for (int i = 0; i < visibleScope.size(); i++)
    {
        auto scope = symbolTable[visibleScope[i]];
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
                return symbol->getDataType();
        }
    }

    throwSemanticError(line, "function '" + name + "' hasn't been declared.");
    return "false";
}
void CallExprNode::loadVariables(std::string &ir, std::vector<std::string> &loadedVars, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    for (Node *arg : arguments)
        arg->loadVariables(ir, loadedVars, tempvarMap);
}
void CallExprNode::storeValue(std::string &ir, const std::string &assignmentName, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}
void CallExprNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
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
void AssignNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (identifier->getType() != TokenType::UNKNOWN)
    {
        for (int i = 0; i < visibleScope.size(); i++)
        {
            auto scope = symbolTable[visibleScope[i]];
            for (Symbol *symbol : scope.getSymbols())
            {
                if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName())
                    throwSemanticError(line, "'" + identifier->getName() + "' has already been declared.");
            }
        }

        if (identifier->getType() == TokenType::VAR)
        {
            std::string t = value->inferType(symbolTable, visibleScope);
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
std::string BlockNode::mapToGlobalIR()
{
    std::string ir;
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

    return ir;
}
void BlockNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
    ir += "define i32 @main() {\n";

    for (Node *node : statements)
        node->mapToIR(ir, tempvarMap, true);

    ir += "ret i32 0\n";
    ir += "}";
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
void IfStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "If Statement condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);
    visibleScope.pop_back();
}
void IfStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
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
void ElseIfStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "Else If Statement condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);
    visibleScope.pop_back();
}
void ElseIfStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
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
void ElseStmtNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);
    visibleScope.pop_back();
}
void ElseStmtNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
}

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
void WhileNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "WhileStatementNode:" << std::endl;
    std::cout << depthTabs << "\tCondition:" << std::endl;
    condition->print(depth + 2);
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}
void WhileNode::checkSemantics(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope)
{
    if (!condition->checkType(symbolTable, visibleScope, "BOOLTYPE"))
        throwSemanticError(line, "While loop condition doesn't evaluate to a boolean.");

    visibleScope.push_back(std::to_string(std::stoi(visibleScope[visibleScope.size() - 1]) + 1));
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(symbolTable, visibleScope);
    visibleScope.pop_back();
}
void WhileNode::mapToIR(std::string &ir, std::vector<std::vector<std::vector<std::string>>> &tempvarMap, bool global)
{
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
                if (symbol->getDataType() == "UNKNOWN" && identifier != nullptr)
                    throwSemanticError(line, "Return statement found in a function that doesn't have a return type.");
                if (identifier != nullptr && !identifier->checkType(symbolTable, visibleScope, symbol->getDataType()))
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
    identifier->loadVariables(ir, loadedVars, tempvarMap, false);
    identifier->storeValue(ir, ".return", tempvarMap, false);

    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load " + type + ", ptr %.return, align " + mapNumBytesIR(returnType) + "\n";
    ir += "ret " + type + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + "\n";
}
