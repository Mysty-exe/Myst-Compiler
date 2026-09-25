#include "lexer.h"

struct Node
{
};

struct ImportNode : public Node
{
private:
    std::string location;
};

struct FuncNode : public Node
{
private:
    std::string name, returnType;
    std::vector<ParamNode *> arguments;
    BlockNode *body;
};

struct CallExprNode : public Node
{
private:
    std::string name;
    std::vector<Node *> arguments;
};

struct ParamNode : public Node
{
private:
    std::string name, returnType;
};

struct BlockNode : public Node
{
private:
    std::vector<StmtNode *> statements;
};

struct StmtNode : public Node
{
private:
public:
};

struct ReturnStmtNode : public StmtNode
{
private:
public:
};

struct IfStmtNode : public Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

struct ElseIfStmtNode : public Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

struct ElseNode : public Node
{
private:
    BlockNode *body;
};

struct WhileNode : public Node
{
private:
    BinaryExprNode *condition;
    BlockNode *body;
};

struct VarDeclNode : public StmtNode
{
private:
    IdentifierNode *identifier;
};

struct BinaryExprNode : public Node
{
private:
    Node *left, *right;

public:
};

struct IdentifierNode : public Node
{
private:
    std::string name;
    std::string type;
    LiteralNode value;
};

struct LiteralNode : public Node
{
private:
    std::string value;
};

class AbstractSyntaxTree
{
private:
    std::vector<Node *> nodes;
};