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
std::string Node::mapToIR(bool global)
{
    return "";
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
std::string IdentifierNode::mapToIR(bool global)
{
    return "";
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
    std::cout << depthTabs << "\tLeft:\n";
    left->print(depth + 2);
    std::cout << depthTabs << "\tRight:\n";
    right->print(depth + 2);
}
bool BinaryExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
    if ((op == "and" || op == "or" || op == "&&" || op == "||") && type != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }
    if ((op == "+" || op == "-" || op == "*" || op == "/" || op == ">" || op == "<" || op == ">=" || op == "<=") && (type != "INTTYPE" && type != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }
    if (op == "==" || op == "!=")
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

    if ((op == "and" || op == "or" || op == "&&" || op == "||") && leftType != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if ((op == "+" || op == "-" || op == "*" || op == "/" || op == ">" || op == "<" || op == ">=" || op == "<=") && (leftType != "INTTYPE" && leftType != "DECIMALTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if (op == "==" || op == "!=")
        return "BOOLTYPE";

    return leftType;
}
std::string BinaryExprNode::mapToIR(bool global)
{
    return "";
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
    std::cout << depthTabs << "\tRight:" << std::endl;
    right->print(depth + 2);
}
bool UnaryExprNode::checkType(std::unordered_map<std::string, Scope> &symbolTable, std::vector<std::string> &visibleScope, const std::string &type)
{
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
std::string UnaryExprNode::mapToIR(bool global)
{
    return "";
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
std::string ParamNode::mapToIR(bool global)
{
    return "";
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
std::string FuncNode::mapToIR(bool global)
{
    return "";
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
std::string CallExprNode::mapToIR(bool global)
{
    return "";
}

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
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
std::string AssignNode::mapToIR(bool global)
{
    std::string ir;
    if (identifier->getType() == TokenType::CHARTYPE)
        ir += "store i1 " + identifier->mapToIR() + ", ptr @" + identifier->getName() + ", align 1\n";
    if (identifier->getType() == TokenType::INTTYPE)
        ir += "store i32 " + identifier->mapToIR() + ", ptr @" + identifier->getName() + ", align 4\n";
    if (identifier->getType() == TokenType::DECIMALTYPE)
        ir += "store double " + identifier->mapToIR() + ", ptr @" + identifier->getName() + ", align 4\n";
    if (identifier->getType() == TokenType::BOOLTYPE)
        ir += "store i1 " + identifier->mapToIR() + ", ptr @" + identifier->getName() + ", align 1\n";
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
            if (assignment->getType() == "CHARTYPE")
                ir += "@" + assignment->getName() + " = global i8 0, align 1\n";
            if (assignment->getType() == "INTTYPE")
                ir += "@" + assignment->getName() + " = global i32 0, align 4\n";
            if (assignment->getType() == "DECIMALTYPE")
                ir += "@" + assignment->getName() + " = global double 0.0, align 4\n";
            if (assignment->getType() == "BOOLTYPE")
                ir += "@" + assignment->getName() + " = global i1 0, align 1\n";
        }
    }

    ir += "\n";

    return ir;
}
std::string BlockNode::mapToIR(bool global)
{
    std::string ir;
    ir += "define i32 @main() {\n";

    for (Node *node : statements)
        node->mapToIR(true);

    ir += "ret i32 0\n";
    ir += "}";

    return ir;
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
std::string IfStmtNode::mapToIR(bool global)
{
    return "";
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
std::string ElseIfStmtNode::mapToIR(bool global)
{
    return "";
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
std::string ElseStmtNode::mapToIR(bool global)
{
    return "";
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
std::string WhileNode::mapToIR(bool global)
{
    return "";
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
std::string ReturnStmtNode::mapToIR(bool global)
{
    return "";
}
