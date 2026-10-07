#include "compiler/ast.h"

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
        {
            IdentifierNode *identifier = new IdentifierNode(line[start].getValue());
            identifier->setLine(line[start].getLineNumber());
            return identifier;
        }
        else if (tokenType == "CHAR")
        {
            LiteralNode<char> *literal = new LiteralNode(line[start].getValue()[0], line[start].getToken());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
        else if (tokenType == "STRING")
        {
            LiteralNode<std::string> *literal = new LiteralNode(line[start].getValue(), line[start].getToken());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
        else if (tokenType == "INT")
        {
            LiteralNode<int> *literal = new LiteralNode(std::stoi(line[start].getValue()), line[start].getToken());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
        else if (tokenType == "DECIMAL")
        {
            LiteralNode<double> *literal = new LiteralNode(std::stod(line[start].getValue()), line[start].getToken());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
        else if (tokenType == "BOOL")
        {
            if (line[start].getValue() == "true")
            {
                LiteralNode<bool> *literal = new LiteralNode(true, line[start].getToken());
                literal->setLine(line[start].getLineNumber());
                return literal;
            }
            else if (line[start].getValue() == "false")
            {
                LiteralNode<bool> *literal = new LiteralNode(true, line[start].getToken());
                literal->setLine(line[start].getLineNumber());
                return literal;
            }
        }
        else
            throwSyntaxError(line[start].getLineNumber(), "Unexpected Identifier '" + line[start].getValue() + "' found.");
    }
    else if (hasOperator(line, start, end))
    {
        if (line[start].getToken() == "PLUS" || line[start].getToken() == "MINUS" || line[start].getToken() == "NOT")
        {
            UnaryExprNode *unaryExpr = new UnaryExprNode(line[start].getValue(), buildIdentifier(line, start + 1, end));
            unaryExpr->setLine(line[start].getLineNumber());
            return unaryExpr;
        }

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

            CallExprNode *callExpr = new CallExprNode(line[start].getValue(), arguments);
            callExpr->setLine(line[start].getLineNumber());
            return callExpr;
        }
    }

    throwSyntaxError(line[0].getLineNumber(), "Line couldn't be parsed.");
    exit(-1);
}

BlockNode *AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction)
{
    BlockNode *blockNode = new BlockNode();
    blockNode->setLine(startLine + 1);
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
            if (inFunction)
                throwSyntaxError(line, "Function can't be defined in a function.");
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
            condition->setLine(line);

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'if' statement.");

            IfStmtNode *ifStmt = new IfStmtNode(condition, block);
            ifStmt->setLine(line);
            blockNode->addStatement(ifStmt);
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
            condition->setLine(line);

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'elseif' statement.");

            ElseIfStmtNode *elseIfStmt = new ElseIfStmtNode(condition, block);
            elseIfStmt->setLine(line);
            blockNode->addStatement(elseIfStmt);
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

            ElseStmtNode *elseStmt = new ElseStmtNode(block);
            elseStmt->setLine(line);
            blockNode->addStatement(elseStmt);
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
            condition->setLine(line);

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line, "Missing body in 'while' loop.");

            WhileNode *whileStmt = new WhileNode(condition, block);
            whileStmt->setLine(line);
            blockNode->addStatement(whileStmt);
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

            AssignNode *assignStmt = new AssignNode(tokens[line][assignIndex - 1].getValue(), splitLine(tokens[line], tabs, assignIndex - 2), buildIdentifier(tokens[line], assignIndex + 1, tokens[line].size() - 2));
            assignStmt->setLine(line + 1);
            blockNode->addStatement(assignStmt);
            continue;
        }
        int returnIndex = getTokenIndex(tokens[line], "RETURN");
        if (returnIndex != -1)
        {
            if (!inFunction)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'return' statement found outside of a function.");
            if (getNumValidTokens(tokens[line]) < 2)
            {
                ReturnStmtNode *returnStmt = new ReturnStmtNode();
                returnStmt->setLine(line + 1);
                blockNode->addStatement(returnStmt);
            }
            else
            {
                ReturnStmtNode *returnStmt = new ReturnStmtNode(buildIdentifier(tokens[line], returnIndex + 1, tokens[line].size() - 2));
                returnStmt->setLine(line + 1);
                blockNode->addStatement(returnStmt);
            }

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

            FuncNode *func = new FuncNode(information[0], information[1], parameters, body);
            func->setLine(line - 1);
            root->addNode(func);
            continue;
        }

        root->addNode(buildBlock(tokens, line, 0));
    }
}

void AbstractSyntaxTree::printTree() const
{
    root->print();
}
