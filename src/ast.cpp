#include "compiler/ast.h"

Node::Node()
{
}
std::ostream &operator<<(std::ostream &os, Node const &node)
{
    node.print(os);
    return os;
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
void RootNode::print(std::ostream &os) const
{
    os << "Root:";
}

LiteralNode::LiteralNode(std::string value)
{
    this->value = value;
}
void LiteralNode::print(std::ostream &os) const
{
    os << "LiteralNode: " << value;
}

IdentifierNode::IdentifierNode(std::string name)
{
    this->name = name;
    this->type = "";
}
IdentifierNode::IdentifierNode(std::string name, std::string returnType)
{
    this->name = name;
    this->type = returnType;
}
void IdentifierNode::print(std::ostream &os) const
{
    os << "IdentifierNode: " << type << " " << name;
}

BinaryExprNode::BinaryExprNode(std::string op, Node *left, Node *right)
{
    this->op = op;
    this->left = left;
    this->right = right;
}
void BinaryExprNode::print(std::ostream &os) const
{
    os << "BinaryExpressionNode: " << op;
    os << "\t" << *left;
    os << "\t" << *right;
}

UnaryExprNode::UnaryExprNode(std::string op, Node *right)
{
    this->op = op;
    this->right = right;
}
void UnaryExprNode::print(std::ostream &os) const
{
    os << "UnaryExpressionNode: " << op;
    os << "\t" << *right;
}

ParamNode::ParamNode(std::string identifier)
{
    for (int i = identifier.size() - 1; i > -1; i--)
    {
        if (identifier[i] == ' ')
        {
            name = identifier.substr(i);
            returnType = identifier.substr(0, i);
            return;
        }
    }
}
ParamNode::ParamNode(std::string name, std::string returnType)
{
    this->name = name;
    this->returnType = returnType;
}
void ParamNode::print(std::ostream &os) const
{
    os << "ParameterNode: " << returnType << name;
}

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body)
{
    this->name = name;
    this->returnType = returnType;
    this->parameters = parameters;
    this->body = body;
}
void FuncNode::print(std::ostream &os) const
{
    os << "FunctionNode: " << name;
    os << "\tReturn Type: " << returnType;
    os << "\tParamters: ";
    for (int i = 0; i < parameters.size(); i++)
    {
        if (i == parameters.size() - 1)
            os << "\t\t" << parameters[i];
        else
            os << "\t\t" << parameters[i] << ", ";
    }
    os << "\t" << *body;
}

CallExprNode::CallExprNode(std::string name, std::vector<Node *> arguments)
{
    this->name = name;
    this->arguments = arguments;
}
void CallExprNode::print(std::ostream &os) const
{
    os << "CallExpressionNode: " << name;
    os << "Arguments: ";
    for (int i = 0; i < arguments.size(); i++)
    {
        if (i == arguments.size() - 1)
            os << "\t" << arguments[i];
        else
            os << "\t" << arguments[i] << ", ";
    }
}

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
    this->identifier = new IdentifierNode(name, type);
    this->value = value;
}
void AssignNode::print(std::ostream &os) const
{
    os << "AssignmentNode: \n";
}

BlockNode::BlockNode()
{
}
void BlockNode::addStatement(Node *stmt)
{
    statements.push_back(stmt);
}
void BlockNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

void StmtNode::print(std::ostream &os) const
{
    os << "StatementNode: \n";
}

IfStmtNode::IfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
void IfStmtNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

ElseIfStmtNode::ElseIfStmtNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
void ElseIfStmtNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

ElseNode::ElseNode(BlockNode *body)
{
    this->body = body;
}
void ElseNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

WhileNode::WhileNode(Node *condition, BlockNode *body)
{
    this->condition = condition;
    this->body = body;
}
void WhileNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

ReturnStmtNode::ReturnStmtNode(Node *identifier)
{
    this->identifier = identifier;
}
void ReturnStmtNode::print(std::ostream &os) const
{
    os << "BlockNode: \n";
}

AbstractSyntaxTree::AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens)
{
    root = new RootNode();
    buildTree(tokens);
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
        if (isValidToken(token))
            return true;
    }

    return false;
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

    throw std::invalid_argument("Invalid argument provided.");
}

int AbstractSyntaxTree::getNextDedentedLine(const std::vector<std::vector<Token>> &tokens, int startLine) const
{
    int currentTabs = getNumTabs(tokens[startLine]);
    for (int line = startLine + 1; line < tokens.size(); line++)
    {
        if (getNumTabs(tokens[line]) == currentTabs)
            return line;
    }

    return -1;
}

// Assumes Function isn't Tabbed
std::vector<std::string> AbstractSyntaxTree::getFunctionInformation(const std::vector<Token> &tokenLine) const
{
    std::vector<std::string> result;
    result.push_back(getNthToken(tokenLine, 2).getValue());

    int arrowIndex = getTokenIndex(tokenLine, "ARROW");
    std::string returnType = "void";
    if (arrowIndex != -1)
        returnType = tokenLine[arrowIndex + 1].getValue();
    result.push_back(returnType);

    int startParams = 3;
    int endParams;
    if (arrowIndex == -1)
        endParams = getTokenIndex(tokenLine, "COLON", true) - 2;
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
                currentParameter += " " + tokenLine[i].getValue();
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
        if (line[start].getToken() == "IDENTIFIER")
            return new IdentifierNode(line[start].getValue());
        else
            return new LiteralNode(line[start].getValue());
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

            arguments.push_back(buildIdentifier(line, startArg, end - 1));
            return new CallExprNode(line[start].getValue(), arguments);
        }
    }

    throw std::logic_error("Something is Wrong");
}

BlockNode *AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine)
{
    BlockNode *blockNode = new BlockNode();
    int tabs = getNumTabs(tokens[startLine]);

    for (int line = startLine; line < tokens.size(); line++)
    {
        if (getNumTabs(tokens[line]) < tabs)
        {
            startLine = line;
            break;
        }

        if (!isValidLine(tokens[line]))
            continue;

        if (getNthToken(tokens[line], 1).getValue() == "func")
        {
            startLine = line;
            break;
        }

        int ifIndex = getTokenIndex(tokens[line], "IF");
        if (ifIndex != -1)
        {
            line++;
            blockNode->addStatement(new IfStmtNode(buildIdentifier(tokens[line - 1], ifIndex + 1, tokens[line].size() - 3), buildBlock(tokens, line)));
            continue;
        }
        int elseIfIndex = getTokenIndex(tokens[line], "ELSEIF");
        if (elseIfIndex != -1)
        {
            line++;
            blockNode->addStatement(new ElseIfStmtNode(buildIdentifier(tokens[line - 1], elseIfIndex + 1, tokens[line].size() - 3), buildBlock(tokens, line)));
            continue;
        }
        int elseIndex = getTokenIndex(tokens[line], "ELSE");
        if (elseIndex != -1)
        {
            line++;
            blockNode->addStatement(new ElseNode(buildBlock(tokens, line)));
            continue;
        }

        int whileIndex = getTokenIndex(tokens[line], "WHILE");
        if (whileIndex != -1)
        {
            line++;
            blockNode->addStatement(new WhileNode(buildIdentifier(tokens[line - 1], whileIndex + 1, tokens[line].size() - 3), buildBlock(tokens, line)));
            continue;
        }

        int assignIndex = getTokenIndex(tokens[line], "ASSIGN");
        if (assignIndex != -1)
        {
            blockNode->addStatement(new AssignNode(tokens[line][assignIndex - 1].getValue(), splitLine(tokens[line], tabs, assignIndex - 2), buildIdentifier(tokens[line], assignIndex + 1, tokens[line].size() - 2)));
            continue;
        }
        int returnIndex = getTokenIndex(tokens[line], "RETURN");
        if (returnIndex != -1)
        {
            blockNode->addStatement(new ReturnStmtNode(buildIdentifier(tokens[line], returnIndex + 1, tokens[line].size() - 2)));
            continue;
        }

        blockNode->addStatement(buildIdentifier(tokens[line], tabs, tokens[line].size() - 2));

        startLine = line;
    }

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
            BlockNode *body = buildBlock(tokens, line);
            std::vector<ParamNode *> parameters;

            for (int i = 2; i < information.size(); i++)
            {
                parameters.push_back(new ParamNode(information[i]));
            }

            root->addNode(new FuncNode(information[0], information[1], parameters, body));
            continue;
        }

        buildBlock(tokens, line);
    }
}

void AbstractSyntaxTree::printTree() const
{
    std::cout << *root << std::endl;
    for (Node *node : root->getChildren())
    {
        std::cout << *node << std::endl;
    }
}
