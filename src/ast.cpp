#include "compiler/ast.h"

void throwLexicalError(int line, std::string message)
{
    try
    {
        throw LexicalError(message);
    }
    catch (const LexicalError &e)
    {
        std::cerr << "\033[31m";
        std::cerr << "Lexical Error on line " << line << ": " << e.what() << std::endl;
        std::cerr << "\033[0m";
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
        std::cerr << "Syntax Error on line " << line << ": " << e.what() << std::endl;
        std::cerr << "\033[0m";
        exit(-1);
    }
}

void throwSemanticWarning(int line, std::string message)
{
    std::cout << "\033[38;5;208m";
    std::cout << "Warning on line " << line << ": " << message << std::endl;
    std::cout << "\033[38;5;208m";
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
        std::cerr << "Semantic Error on line " << line << ": " << e.what() << std::endl;
        std::cerr << "\033[0m";
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

    return "unknown";
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

void Node::checkSemantics(std::unordered_map<std::string, std::vector<std::string>> &scope, std::vector<std::string> visibleScope)
{
}

RootNode::RootNode()
{
}
void RootNode::addNode(Node *node)
{
    children.push_back(node);
}
std::vector<Node *> RootNode::getChildren()
{
    return children;
}
void RootNode::print(int depth) const
{
    std::cout << "Root:" << std::endl;
    for (Node *node : children)
    {
        node->print(depth + 1);
        std::cout << std::endl;
    }
}

void RootNode::checkSemantics(std::unordered_map<std::string, std::vector<std::string>> &scope, std::vector<std::string> visibleScope)
{
    for (Node *child : children)
        child->checkSemantics(scope, visibleScope);
}

template <typename T>
LiteralNode<T>::LiteralNode(T value)
{
    this->value = value;
}

template <typename T>
void LiteralNode<T>::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "LiteralNode: " << value << std::endl;
}

IdentifierNode::IdentifierNode(std::string name)
{
    this->name = name;
    this->type = TokenType::UNKNOWN;
}
IdentifierNode::IdentifierNode(std::string name, std::string returnType)
{
    this->name = name;

    if (returnType == "char")
        this->type = TokenType::CHARTYPE;
    if (returnType == "string")
        this->type = TokenType::STRINGTYPE;
    if (returnType == "int")
        this->type = TokenType::INTTYPE;
    if (returnType == "decimal")
        this->type = TokenType::DECIMALTYPE;
    if (returnType == "bool")
        this->type = TokenType::BOOLTYPE;
}
std::string IdentifierNode::getName() const
{
    return name;
}
TokenType IdentifierNode::getType() const
{
    return type;
}
void IdentifierNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "IdentifierNode: " << getDataTypeStr(Token::getType(type)) << " " << name << std::endl;
}

BinaryExprNode::BinaryExprNode(std::string op, Node *left, Node *right)
{
    this->op = op;
    this->left = left;
    this->right = right;
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

UnaryExprNode::UnaryExprNode(std::string op, Node *right)
{
    this->op = op;
    this->right = right;
}
void UnaryExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "UnaryExpressionNode: " << op << std::endl;
    std::cout << depthTabs << "\tRight:" << std::endl;
    right->print(depth + 2);
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

void FuncNode::checkSemantics(std::unordered_map<std::string, std::vector<std::string>> &scope, std::vector<std::string> visibleScope)
{
}

CallExprNode::CallExprNode(std::string name, std::vector<Node *> arguments)
{
    this->name = name;
    this->arguments = arguments;
}
void CallExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "CallExpressionNode: " << name << std::endl;
    std::cout << depthTabs << "\tArguments: " << std::endl;
    for (int i = 0; i < arguments.size(); i++)
        arguments[i]->print(depth + 2);
}

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
    this->identifier = new IdentifierNode(name, type);
    this->value = value;
}
void AssignNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "AssignmentNode: " << std::endl;
    std::cout << depthTabs << "\tLeft Side: " << getDataTypeStr(Token::getType(identifier->getType())) << " " << identifier->getName() << std::endl;
    std::cout << depthTabs << "\tRight Side: " << std::endl;
    value->print(depth + 2);
}
void BlockNode::checkSemantics(std::unordered_map<std::string, std::vector<std::string>> &scope, std::vector<std::string> visibleScope)
{
}

BlockNode::BlockNode()
{
}
void BlockNode::addStatement(Node *stmt)
{
    statements.push_back(stmt);
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
void BlockNode::checkSemantics(std::unordered_map<std::string, std::vector<std::string>> &scope, std::vector<std::string> visibleScope)
{
    for (Node *statement : statements)
        statement->checkSemantics(scope, visibleScope);
}
void StmtNode::print(int depth) const
{
    std::cout << "StatementNode: \n";
}

IfStmtNode::IfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
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

ElseIfStmtNode::ElseIfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
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

ElseNode::ElseNode(BlockNode *body)
{
    this->body = body;
}
void ElseNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ElseStatementNode:" << std::endl;
    std::cout << depthTabs << "\tBody:" << std::endl;
    body->print(depth + 2);
}

WhileNode::WhileNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
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

ReturnStmtNode::ReturnStmtNode(Node *identifier)
{
    this->identifier = identifier;
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

AbstractSyntaxTree::AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens)
{
    root = new RootNode();
    buildTree(tokens);
}

RootNode *AbstractSyntaxTree::getRoot() const
{
    return root;
}

const std::vector<std::vector<std::string>> AbstractSyntaxTree::operatorPrecedence =
    {
        {"STAR", "SLASH"},
        {"PLUS", "MINUS"},
        {"LESSTHAN", "GREATERTHAN", "LESSTHANEQ", "GREATERTHANEQ"},
        {"EQUALS", "NEQUALS"},
        {"AND"},
        {"OR"}};

int AbstractSyntaxTree::getTokenIndex(const std::vector<Token> &tokenLine, const std::string &tokenStr, bool includeOnlyValidTokens) const
{
    int result = 0;
    for (int i = 0; i < tokenLine.size(); i++)
    {
        if (includeOnlyValidTokens && !isValidToken(tokenLine[i]))
            result--;

        if (tokenLine[i].getToken() == tokenStr)
            return result;

        result++;
    }

    return -1;
}

int AbstractSyntaxTree::getNumTabs(const std::vector<Token> &tokenLine) const
{
    int tabs = 0;
    for (const Token &token : tokenLine)
    {
        if (token.getToken() == "INDENT")
            tabs++;
    }

    return tabs;
}

bool AbstractSyntaxTree::isDataType(const std::string &str) const
{
    return (str == "CHARTYPE" || str == "STRINGTYPE" || str == "INTTYPE" || str == "DECIMALTYPE" || str == "BOOLTYPE");
}

bool AbstractSyntaxTree::isValidToken(Token token) const
{
    std::string tokenStr = token.getToken();
    return tokenStr != "INDENT" && tokenStr != "NEWLINE" && tokenStr != "COMMENT" && tokenStr != "ENDFILE";
}

bool AbstractSyntaxTree::isValidLine(const std::vector<Token> &tokens) const
{
    if (tokens.size() == 0)
        return false;

    for (const Token &token : tokens)
    {
        if (token.getToken() == "UNKNOWN")
            throwLexicalError(token.getLineNumber(), "Unexpected token, '" + token.getValue() + "'");
    }

    for (const Token &token : tokens)
    {
        if (isValidToken(token))
            return true;
    }

    return false;
}

int AbstractSyntaxTree::getNumValidTokens(const std::vector<Token> &line) const
{
    int counter = 0;
    for (const Token &token : line)
    {
        if (isValidToken(token))
            counter++;
    }

    return counter;
}

bool AbstractSyntaxTree::hasOperator(const std::vector<Token> &line, int start, int end) const
{
    int lparen = 0, rparen = 0;
    for (int i = start; i <= end; i++)
    {
        if (line[i].getToken() == "LPAREN")
            lparen++;
        else if (line[i].getToken() == "RPAREN")
            rparen++;

        if (lparen != rparen)
            continue;

        if (std::find(Token::operators.begin(), Token::operators.end(), line[i].getValue()) != Token::operators.end())
            return true;
    }

    return false;
}

Token AbstractSyntaxTree::getNthToken(const std::vector<Token> &tokenLine, int n) const
{
    int count = 1;
    if (n > 0)
    {
        for (const Token &token : tokenLine)
        {
            if (isValidToken(token))
            {
                if (count == n)
                    return token;

                count += 1;
            }
        }
    }
    else if (n < 0)
    {
        n = abs(n);
        for (int i = tokenLine.size() - 1; i > -1; i--)
        {
            if (isValidToken(tokenLine[i]))
            {
                if (count == n)
                    return tokenLine[i];

                count += 1;
            }
        }
    }

    throwSyntaxError(tokenLine[0].getLineNumber(), "Line couldn't be parsed.");
    exit(-1);
}

// Assumes Function isn't Tabbed
std::vector<std::string> AbstractSyntaxTree::getFunctionInformation(const std::vector<Token> &tokenLine) const
{
    std::vector<std::string> result;
    Token name = getNthToken(tokenLine, 2);
    if (name.getToken() == "IDENTIFIER")
        result.push_back(name.getValue());
    else
        throwSyntaxError(name.getLineNumber(), "Unexpected identifier after 'func' found.");

    int arrowIndex = getTokenIndex(tokenLine, "ARROW");
    std::string returnType = "void";
    if (arrowIndex != -1)
    {
        if (!isDataType(tokenLine[arrowIndex + 1].getToken()))
            throwSyntaxError(tokenLine[arrowIndex + 1].getLineNumber(), "Unexpected idenifier '" + tokenLine[arrowIndex + 1].getValue() + "' used as the function return type.");
        returnType = tokenLine[arrowIndex + 1].getValue();
    }
    result.push_back(returnType);

    int colonIndex = getTokenIndex(tokenLine, "COLON", true);
    if (colonIndex == -1)
        throwSyntaxError(name.getLineNumber(), "Expected ':' at the end of the function signature.");

    int startParams = 3;
    int endParams;
    if (arrowIndex == -1)
        endParams = colonIndex - 2;
    else
        endParams = arrowIndex - 2;

    std::string currentParameter = "";
    for (int i = startParams; i <= endParams; i++)
    {
        if (tokenLine[i].getToken() == "COMMA")
        {
            result.push_back(currentParameter);
            currentParameter = "";
        }
        else
        {
            if (currentParameter.size() > 0)
            {
                if (tokenLine[i].getToken() == "IDENTIFIER")
                    currentParameter += " " + tokenLine[i].getValue();
                else
                    throwSyntaxError(tokenLine[i].getLineNumber(), "Unexpected identifier '" + tokenLine[i].getValue() + "' used as parameter name.");
            }
            else
                currentParameter += tokenLine[i].getValue();
        }
    }

    if (currentParameter.size() > 0)
        result.push_back(currentParameter);

    return result;
}

std::string AbstractSyntaxTree::splitLine(const std::vector<Token> &tokens, int startIndex, int endIndex) const
{
    std::string result;
    for (int token = startIndex; token <= endIndex; token++)
    {
        if (result.size() > 0)
            result += " " + tokens[token].getValue();
        else
            result += tokens[token].getValue();
    }

    return result;
}

Node *AbstractSyntaxTree::buildIdentifier(const std::vector<Token> &line, int start, int end)
{
    if (end - start == 0)
    {
        std::string tokenType = line[start].getToken();
        if (tokenType == "IDENTIFIER")
            return new IdentifierNode(line[start].getValue());
        else if (tokenType == "CHAR")
            return new LiteralNode(line[start].getValue()[0]);
        else if (tokenType == "STRING")
            return new LiteralNode(line[start].getValue());
        else if (tokenType == "INT")
            return new LiteralNode(std::stoi(line[start].getValue()));
        else if (tokenType == "DECIMAL")
            return new LiteralNode(std::stoi(line[start].getValue()));
        else if (tokenType == "BOOL")
        {
            if (line[start].getValue() == "true")
                return new LiteralNode(true);
            else if (line[start].getValue() == "false")
                return new LiteralNode(false);
        }
        else
            throwSyntaxError(line[start].getLineNumber(), "Unexpected Identifier '" + line[start].getValue() + "' found.");
    }
    else if (hasOperator(line, start, end))
    {
        if (line[start].getToken() == "PLUS" || line[start].getToken() == "MINUS" || line[start].getToken() == "NOT")
            return new UnaryExprNode(line[start].getValue(), buildIdentifier(line, start + 1, end));

        int lparen = 0, rparen = 0;
        int splitIndex = -1, currentLevel = -1;
        for (int i = start; i <= end; i++)
        {
            if (line[i].getToken() == "LPAREN")
                lparen++;

            else if (line[i].getToken() == "RPAREN")
                rparen++;

            if (lparen != rparen)
                continue;

            for (int level = 0; level < operatorPrecedence.size(); level++)
            {
                for (int op = 0; op < operatorPrecedence[level].size(); op++)
                {
                    if (line[i].getToken() == operatorPrecedence[level][op])
                    {
                        if (level > currentLevel)
                        {
                            splitIndex = i;
                            currentLevel = level;
                        }
                    }
                }
            }
        }

        return new BinaryExprNode(line[splitIndex].getValue(), buildIdentifier(line, start, splitIndex - 1), buildIdentifier(line, splitIndex + 1, end));
    }
    else
    {
        if (line[start].getToken() == "LPAREN" && line[end].getToken() == "RPAREN")
            return buildIdentifier(line, start + 1, end - 1);

        if (line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "LPAREN" && line[end].getToken() == "RPAREN")
        {
            std::vector<Node *> arguments;
            int startArg = start + 2, lparen = 0, rparen = 0;
            for (int i = startArg; i <= end - 1; i++)
            {
                if (line[i].getToken() == "LPAREN")
                    lparen++;
                if (line[i].getToken() == "RPAREN")
                    rparen++;

                if (line[i].getToken() == "COMMA" && lparen == rparen)
                {
                    rparen = 0, lparen = 0;
                    arguments.push_back(buildIdentifier(line, startArg, i - 1));
                    startArg = i + 1;
                }
            }

            if (startArg <= end - 1)
                arguments.push_back(buildIdentifier(line, startArg, end - 1));
            return new CallExprNode(line[start].getValue(), arguments);
        }
    }

    throwSyntaxError(line[0].getLineNumber(), "Line couldn't be parsed.");
    exit(-1);
}

BlockNode *AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction)
{
    BlockNode *blockNode = new BlockNode();
    int tabs = getNumTabs(tokens[startLine]);

    for (int line = startLine; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
            continue;

        int currentIndent = getNumTabs(tokens[line]);
        if (currentIndent < expectedIndent)
        {
            startLine = line;
            return blockNode;
        }

        if (getNthToken(tokens[line], 1).getValue() == "func")
        {
            startLine = line - 1;
            return blockNode;
        }

        int ifIndex = getTokenIndex(tokens[line], "IF");
        if (ifIndex != -1)
        {
            if (getNthToken(tokens[line], 1).getToken() != "IF")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'if' statement found.");

            line++;
            if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
                throwSyntaxError(line, "Expected ':' at the end of 'if' statement.");

            Node *condition = buildIdentifier(tokens[line - 1], ifIndex + 1, tokens[line - 1].size() - 3);
            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'if' statement.");

            blockNode->addStatement(new IfStmtNode(condition, block));
            startLine = line;
            line--;
            continue;
        }

        int elseIfIndex = getTokenIndex(tokens[line], "ELSEIF");
        if (elseIfIndex != -1)
        {
            if (getNthToken(tokens[line], 1).getToken() != "ELSEIF")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'elseif' statement found.");

            line++;

            if (blockNode->getNumStatements() == 0 || !dynamic_cast<IfStmtNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]))
                throwSyntaxError(line, "'elseif' statement found without a preceding 'if' statement.");
            if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
                throwSyntaxError(line, "Expected ':' at the end of elseif statement.");
            Node *condition = buildIdentifier(tokens[line - 1], elseIfIndex + 1, tokens[line - 1].size() - 3);
            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'elseif' statement.");

            blockNode->addStatement(new ElseIfStmtNode(condition, block));
            startLine = line;
            line--;
            continue;
        }
        int elseIndex = getTokenIndex(tokens[line], "ELSE");
        if (elseIndex != -1)
        {
            if (getNthToken(tokens[line], 1).getToken() != "ELSE")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'else' statement found.");

            line++;
            if (blockNode->getNumStatements() == 0 || !dynamic_cast<IfStmtNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]))
                throwSyntaxError(line, "'else' statement found without a preceding 'if' statement.");
            int colonIndex = getTokenIndex(tokens[line - 1], "COLON");
            if (colonIndex == -1)
                throwSyntaxError(line, "Expected ':' at the end of else statement.");

            int numValidTokens = getNumValidTokens(tokens[line - 1]);
            if (numValidTokens != 2)
                throwSyntaxError(line, "Line couldn't be parsed.");

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'else' statement.");

            blockNode->addStatement(new ElseNode(block));
            startLine = line;
            line--;
            continue;
        }

        int whileIndex = getTokenIndex(tokens[line], "WHILE");
        if (whileIndex != -1)
        {
            if (getNthToken(tokens[line], 1).getToken() != "WHILE")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'while' statement found.");

            line++;
            if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
                throwSyntaxError(line, "Expected ':' at the end of 'while' loop.");
            Node *condition = buildIdentifier(tokens[line - 1], whileIndex + 1, tokens[line - 1].size() - 3);
            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'while' loop.");

            blockNode->addStatement(new WhileNode(condition, block));
            line--;
            startLine = line;
            continue;
        }

        int assignIndex = getTokenIndex(tokens[line], "ASSIGN");
        if (assignIndex != -1)
        {
            if (tokens[line][assignIndex - 1].getToken() != "IDENTIFIER")
                throwSyntaxError(tokens[line][0].getLineNumber(), "'" + tokens[line][assignIndex - 1].getValue() + "' can't be used as a variable name.");

            std::string type = splitLine(tokens[line], tabs, assignIndex - 2);
            if (type != "" && !isDataType(tokens[line][assignIndex - 2].getToken()) && tokens[line][assignIndex - 2].getToken() != "VAR")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected type used for variable '" + tokens[line][assignIndex - 1].getValue() + "'.");

            blockNode->addStatement(new AssignNode(tokens[line][assignIndex - 1].getValue(), splitLine(tokens[line], tabs, assignIndex - 2), buildIdentifier(tokens[line], assignIndex + 1, tokens[line].size() - 2)));
            continue;
        }
        int returnIndex = getTokenIndex(tokens[line], "RETURN");
        if (returnIndex != -1)
        {
            if (!inFunction)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'return' statement found outside of a function.");
            if (getNumValidTokens(tokens[line]) < 2)
                blockNode->addStatement(new ReturnStmtNode());
            else
                blockNode->addStatement(new ReturnStmtNode(buildIdentifier(tokens[line], returnIndex + 1, tokens[line].size() - 2)));

            continue;
        }

        blockNode->addStatement(buildIdentifier(tokens[line], tabs, tokens[line].size() - 2));

        startLine = line;
    }

    startLine = tokens.size() - 1;
    return blockNode;
}

void AbstractSyntaxTree::buildTree(const std::vector<std::vector<Token>> &tokens)
{
    for (int line = 0; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
            continue;

        // Functions need a minimum of 4 tokens
        if (getNthToken(tokens[line], 1).getValue() == "func")
        {
            std::vector<std::string> information = getFunctionInformation(tokens[line]);
            line++;

            std::vector<ParamNode *> parameters;

            for (int i = 2; i < information.size(); i++)
            {
                ParamNode *parameter = new ParamNode(information[i]);
                parameter->setLine(line + 1);
                parameters.push_back(new ParamNode(information[i]));
            }

            BlockNode *body = buildBlock(tokens, line, 1, true);
            if (body->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in function '" + information[0] + "'.");
            line--;

            root->addNode(new FuncNode(information[0], information[1], parameters, body));
            continue;
        }

        root->addNode(buildBlock(tokens, line, 0));
    }
}

void AbstractSyntaxTree::printTree() const
{
    root->print();
}
