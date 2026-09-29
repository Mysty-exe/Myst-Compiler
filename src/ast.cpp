#include "compiler/ast.h"

Node::Node()
{
}

RootNode::RootNode()
{
}

void RootNode::addNode(Node *node)
{
    children.push_back(node);
}

ParamNode::ParamNode(std::string name, std::string returnType)
{
    this->name = name;
    this->returnType = returnType;
}

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body)
{
    this->name = name;
    this->returnType = returnType;
    this->parameters = parameters;
    this->body = body;
}

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
    this->name = name;
    this->type = type;
    this->value = value;
}

BlockNode::BlockNode()
{
}

void BlockNode::addStatement(StmtNode *stmt)
{
    statements.push_back(stmt);
}

ReturnStmtNode::ReturnStmtNode(Node *identifier)
{
    this->identifier = identifier;
}

AbstractSyntaxTree::AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens)
{
    root = new RootNode();
    buildTree(tokens);
}

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
    for (int i = start; i <= end; i++)
        std::cout << line[i].getValue() << std::endl;
    return new Node();
}

BlockNode *AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine)
{
    BlockNode *blockNode = new BlockNode();
    int tabs = getNumTabs(tokens[startLine]);

    for (int line = startLine; line < tokens.size(); line++)
    {

        if (!isValidLine(tokens[line]))
            continue;

        if (getNumTabs(tokens[line]) < tabs)
            break;

        if (getNthToken(tokens[line], 1).getValue() == "func")
            break;

        int assignIndex = getTokenIndex(tokens[line], "ASSIGN");
        if (assignIndex != -1)
            blockNode->addStatement(new AssignNode(tokens[line][assignIndex - 1].getValue(), splitLine(tokens[line], tabs, assignIndex - 2), buildIdentifier(tokens[line], assignIndex + 1, tokens[line].size() - 2)));

        int returnIndex = getTokenIndex(tokens[line], "RETURN");
        if (returnIndex != -1)
            blockNode->addStatement(new ReturnStmtNode(buildIdentifier(tokens[line], returnIndex + 1, tokens[line].size() - 2)));

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
            int nextDedentedLine = getNextDedentedLine(tokens, line);

            line++;
            BlockNode *body = buildBlock(tokens, line);
            continue;
        }

        buildBlock(tokens, line);
    }
}

void AbstractSyntaxTree::printTree() const
{
}
