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

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, Node *body)
{
    this->name = name;
    this->returnType = returnType;
    this->parameters = parameters;
    this->body = body;
}

AbstractSyntaxTree::AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens)
{
    root = new RootNode();
    root->addNode(buildTree(tokens));
}

int AbstractSyntaxTree::getNumTabs(const std::vector<Token> &tokenLine) const
{
    int tabs = 0;
    for (const Token &token : tokenLine)
    {
        if (token.getValue() == "tab")
            tabs++;
    }

    return tabs;
}

Token AbstractSyntaxTree::getFirstToken(const std::vector<Token> &tokenLine) const
{
    for (const Token &token : tokenLine)
    {
        if (token.getToken() != "INDENT" && token.getToken() != "NEWLINE")
            return token;
    }
}

Node *AbstractSyntaxTree::buildTree(const std::vector<std::vector<Token>> &tokens)
{
    for (int line = 0; line < tokens.size(); line++)
    {
        if (getFirstToken(tokens[line]).getValue() == "func")
        {
            int startToken = getNumTabs(tokens[line]) + 1;
            std::string funcName = tokens[line][startToken].getValue();
            std::vector<ParamNode> parameters;
            for (int token = startToken + 2; token < tokens[line].size(); token += 2)
            {
                return new Node();
            }
        }
    }
}

void AbstractSyntaxTree::printTree() const
{
}
