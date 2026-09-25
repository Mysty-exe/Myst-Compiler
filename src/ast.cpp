#include "compiler/ast.h"

Node::Node(Token token)
{
    data = token;
}

Token Node::getToken() const
{
    return data;
}

AbstractSyntaxTree::AbstractSyntaxTree()
{
}