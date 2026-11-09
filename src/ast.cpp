#include "compiler/ast.h"

char extractChar(const std::string &str)
{
    if (str.size() == 1)
        return str[0];

    if (str[0] == '\\')
    {
        if (str[1] == 'a')
            return '\a';
        if (str[1] == 'b')
            return '\b';
        if (str[1] == 'f')
            return '\f';
        if (str[1] == 'n')
            return '\n';
        if (str[1] == 'r')
            return '\r';
        if (str[1] == 't')
            return '\t';
        if (str[1] == 'v')
            return '\v';
        if (str[1] == '\\')
            return '\\';
        if (str[1] == '\'')
            return '\'';
        if (str[1] == '"')
            return '\"';
        if (str[1] == '?')
            return '\?';
        if (str[1] == '0')
            return '\0';
    }

    return ' ';
}

AbstractSyntaxTree::AbstractSyntaxTree(const std::vector<std::vector<Token>> &tokens, std::string fileName)
{
    imported = false;
    root = new RootNode();
    root->setFileName(fileName);
    RootNode::setCurrentFile(fileName);
    buildTree(tokens);
}

AbstractSyntaxTree::~AbstractSyntaxTree()
{
}

RootNode *AbstractSyntaxTree::getRoot() const
{
    return root;
}

std::unordered_map<std::string, Scope> AbstractSyntaxTree::getSymbolTable() const
{
    return symbolTable;
}

void AbstractSyntaxTree::setSymbolTable(std::unordered_map<std::string, Scope> symbolTable)
{
    auto scope = symbolTable["1"];
    for (Symbol *symbol : scope.getSymbols())
    {
        if (std::find(exports.begin(), exports.end(), symbol->getName()) != exports.end())
        {
            symbol->setUsed(true);
            symbol->setExported(true);
        }
    }

    this->symbolTable = symbolTable;
}

const std::vector<std::vector<std::string>> AbstractSyntaxTree::operatorPrecedence =
    {
        {"EXPONENT"},
        {"STAR", "SLASH", "MODULUS"},
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

int AbstractSyntaxTree::getTokenIndex(const std::vector<Token> &tokenLine, const std::vector<std::string> &tokenStrs, bool includeOnlyValidTokens) const
{
    int result;
    for (const std::string &str : tokenStrs)
    {
        result = 0;
        for (int i = 0; i < tokenLine.size(); i++)
        {
            if (includeOnlyValidTokens && !isValidToken(tokenLine[i]))
                result--;

            if (tokenLine[i].getToken() == str)
                return result;

            result++;
        }
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
std::vector<std::string> AbstractSyntaxTree::getFunctionInformation(const std::vector<Token> &tokenLine, bool inContainer) const
{
    std::vector<std::string> result;
    Token name;
    if (inContainer)
    {
        name = getNthToken(tokenLine, 1);
        if (getNthToken(tokenLine, 2).getToken() != "LPAREN")
            throwSyntaxError(name.getLineNumber(), "Function couldn't be parsed.");
    }
    else
    {
        name = getNthToken(tokenLine, 2);
        if (getNthToken(tokenLine, 3).getToken() != "LPAREN")
            throwSyntaxError(name.getLineNumber(), "Function couldn't be parsed.");
    }

    if (name.getToken() == "IDENTIFIER")
        result.push_back(name.getValue());
    else
        throwSyntaxError(name.getLineNumber(), "Unexpected identifier after 'func' found.");

    int arrowIndex = getTokenIndex(tokenLine, "ARROW");
    std::string returnType = "void";
    if (arrowIndex != -1)
    {
        if (!Token::isDataType(tokenLine[arrowIndex + 1].getToken()) && tokenLine[arrowIndex + 1].getToken() != "IDENTIFIER")
            throwSyntaxError(tokenLine[arrowIndex + 1].getLineNumber(), "Unexpected idenifier '" + tokenLine[arrowIndex + 1].getValue() + "' used as the function return type.");
        returnType = tokenLine[arrowIndex + 1].getValue();
    }
    result.push_back(returnType);

    int colonIndex = getTokenIndex(tokenLine, "COLON", false);

    int startParams = 3;
    if (inContainer)
        startParams = 3;

    int endParams;
    if (arrowIndex == -1)
    {
        if (colonIndex == -1)
            endParams = tokenLine.size() - 3;
        else
            endParams = colonIndex - 2;
    }
    else
        endParams = arrowIndex - 2;

    std::string currentParameter = "";
    for (int i = startParams; i <= endParams; i++)
    {
        std::cout << tokenLine[i].getValue() << " ";
        if (tokenLine[i].getToken() == "COMMA")
        {
            result.push_back(currentParameter);
            currentParameter = "";
        }
        else
        {
            if (currentParameter.size() > 0)
            {
                if (tokenLine[i].getToken() == "IDENTIFIER" || tokenLine[i].getToken() == "INTTYPE" || tokenLine[i].getToken() == "DECIMALTYPE" || tokenLine[i].getToken() == "BOOLTYPE" || tokenLine[i].getToken() == "CHARTYPE")
                    currentParameter += " " + tokenLine[i].getValue();
                else
                    throwSyntaxError(tokenLine[i].getLineNumber(), "Unexpected identifier '" + tokenLine[i].getValue() + "' used as parameter name.");
            }
            else
            {

                if (tokenLine[i].getToken() == "REF" || tokenLine[i].getToken() == "IDENTIFIER" || tokenLine[i].getToken() == "INTTYPE" || tokenLine[i].getToken() == "DECIMALTYPE" || tokenLine[i].getToken() == "BOOLTYPE" || tokenLine[i].getToken() == "CHARTYPE")
                    currentParameter += tokenLine[i].getValue();
            }
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

void AbstractSyntaxTree::buildExports(const std::vector<std::vector<Token>> &tokens, int &startLine)
{
    ExportState state = ExportState::STANDBY;
    bool onComma = false;
    for (startLine; startLine < tokens.size(); startLine++)
    {
        if (!isValidLine(tokens[startLine]))
            continue;

        for (int token = 0; token < tokens[startLine].size(); token++)
        {
            if (tokens[startLine][token].getToken() == "INDENT")
                continue;

            if (state == ExportState::IN)
            {
                if (tokens[startLine][token].getToken() == "IDENTIFIER")
                {
                    if (onComma)
                        throwSyntaxError(startLine + 1, "Couldn't parse exports.");

                    exports.push_back(tokens[startLine][token].getValue());
                    onComma = true;
                }

                if (tokens[startLine][token].getToken() == "COMMA")
                {
                    if (!onComma)
                        throwSyntaxError(startLine + 1, "Couldn't parse exports.");

                    onComma = false;
                }
            }

            if (tokens[startLine][token].getToken() == "LCURLYBRACKET")
            {
                if (state != ExportState::STANDBY)
                    throwSyntaxError(startLine + 1, "Couldn't parse exports.");

                state = ExportState::IN;
            }

            if (tokens[startLine][token].getToken() == "RCURLYBRACKET")
            {
                if (!onComma || state != ExportState::IN)
                    throwSyntaxError(startLine + 1, "Couldn't parse exports.");

                state = ExportState::OUT;
                break;
            }
        }

        if (state == ExportState::OUT)
            break;
    }

    if (state != ExportState::OUT)
        throwSyntaxError(startLine + 1, "Couldn't parse exports.");
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
            char c = extractChar(line[start].getValue().substr(1, line[start].getValue().size() - 2));
            LiteralNode<char> *literal = new LiteralNode(c, line[start].getToken());
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
                LiteralNode<bool> *literal = new LiteralNode(false, line[start].getToken());
                literal->setLine(line[start].getLineNumber());
                return literal;
            }
        }
        else
            throwSyntaxError(line[start].getLineNumber(), "Unexpected Identifier '" + line[start].getValue() + "' found.");
    }
    else if (hasOperator(line, start, end))
    {
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
                        if (level >= currentLevel)
                        {
                            if ((line[i].getValue() == "+" || line[i].getValue() == "-") && line[i + 1].getToken() == "LPAREN")
                            {
                                if (i == start || (i - 1 >= start && std::find(Token::operators.begin(), Token::operators.end(), line[i - 1].getValue()) != Token::operators.end()))
                                    break;
                            }
                            splitIndex = i;
                            currentLevel = level;
                        }
                    }
                }
            }
        }

        if (splitIndex != -1)
        {
            BinaryExprNode *binaryExpr = new BinaryExprNode(line[splitIndex].getValue(), buildIdentifier(line, start, splitIndex - 1), buildIdentifier(line, splitIndex + 1, end));
            binaryExpr->setLine(line[splitIndex].getLineNumber());
            std::cout << line[splitIndex].getLineNumber() << std::endl;
            return binaryExpr;
        }

        if (line[start].getToken() == "PLUS" || line[start].getToken() == "MINUS" || line[start].getToken() == "NOT")
        {
            UnaryExprNode *unaryExpr = new UnaryExprNode(line[start].getValue(), buildIdentifier(line, start + 1, end));
            unaryExpr->setLine(line[start].getLineNumber());
            return unaryExpr;
        }
    }
    else
    {
        if (line[start].getToken() == "LPAREN" && line[end].getToken() == "RPAREN")
            return buildIdentifier(line, start + 1, end - 1);

        bool callExpr = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "LPAREN" && line[end].getToken() == "RPAREN";
        bool importedIdentifier = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "DOUBLECOLON" && line[end].getToken() == "IDENTIFIER";
        bool importedCallExpr = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "DOUBLECOLON" && line[start + 2].getToken() == "IDENTIFIER" && line[start + 3].getToken() == "LPAREN" && line[end].getToken() == "RPAREN";

        if (importedIdentifier)
        {
            IdentifierNode *identifier = new IdentifierNode(line[start + 2].getValue());
            identifier->setLine(line[start].getLineNumber());
            identifier->setImported(line[start].getValue());
            return identifier;
        }

        if (callExpr || importedCallExpr)
        {
            std::vector<Node *> arguments;
            int startArg = 0, lparen = 0, rparen = 0;
            if (callExpr)
                startArg = start + 2;
            else
                startArg = start + 4;

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

            CallExprNode *callExpression;
            if (callExpr)
                callExpression = new CallExprNode(line[start].getValue(), arguments);
            else
                callExpression = new CallExprNode(line[start + 2].getValue(), arguments);

            callExpression->setLine(line[start].getLineNumber());

            if (importedCallExpr)
                callExpression->setImported(line[start].getValue());

            return callExpression;
        }
    }

    throwSyntaxError(line[0].getLineNumber(), "Line couldn't be parsed.");
    exit(-1);
}

std::vector<Node *> AbstractSyntaxTree::buildContainer(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent)
{
    std::vector<Node *> body;
    for (int line = startLine; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
            continue;

        int currentIndent = getNumTabs(tokens[line]);
        if (currentIndent < expectedIndent)
        {
            startLine = line;
            return body;
        }

        if (getNthToken(tokens[line], 1).getToken() == "IDENTIFIER" && getNthToken(tokens[line], 2).getToken() == "LPAREN" && getNthToken(tokens[line], -1).getToken() == "COLON")
        {
            std::vector<std::string> information = getFunctionInformation(tokens[line], true);
            if (std::find(Token::reservedFunctions.begin(), Token::reservedFunctions.end(), information[0]) != Token::reservedFunctions.end())
                throwSyntaxError(line + 1, "Reserved function name used: '" + information[0] + "'.");

            line++;

            std::vector<ParamNode *> parameters;

            for (int i = 2; i < information.size(); i++)
            {
                ParamNode *parameter = new ParamNode(information[i], line);
                parameters.push_back(parameter);
            }

            BlockNode *funcBody = buildBlock(tokens, line, 2, true);
            if (funcBody->getNumStatements() == 0)
            {
                if (tokens[funcBody->getLine() - 2][tokens[funcBody->getLine() - 2].size() - 2].getToken() == "COLON")
                    throwSyntaxError(funcBody->getLine() - 1, "Missing body in function '" + information[0] + "'.");
            }
            else
            {
                if (tokens[funcBody->getLine() - 2][tokens[funcBody->getLine() - 2].size() - 2].getToken() != "COLON")
                    throwSyntaxError(funcBody->getLine() - 1, "Colon missing for the function '" + information[0] + "'.");
            }
            line--;

            FuncNode *func = new FuncNode(information[0], information[1], parameters, funcBody, line - 1);
            body.push_back(func);
            continue;
        }

        if (Token::isDataType(getNthToken(tokens[line], 1).getToken()) || getNthToken(tokens[line], 1).getToken() == "IDENTIFIER")
        {
            std::string type = getNthToken(tokens[line], 1).getValue();
            bool onComma = false;
            for (int token = currentIndent + 1; token < tokens[line].size(); token++)
            {
                if (tokens[line][token].getToken() == "INDENT")
                    break;

                if (tokens[line][token].getToken() == "COMMA")
                {
                    if (onComma)
                        onComma = false;
                    else
                        throwSyntaxError(line + 1, "Variables couldn't be parsed in container.");
                }
                if (tokens[line][token].getToken() == "IDENTIFIER")
                {
                    if (onComma)
                        throwSyntaxError(line + 1, "Variables couldn't be parsed in container.");
                    else
                    {
                        body.push_back(new IdentifierNode(tokens[line][token].getValue(), type));
                        onComma = true;
                    }
                }
            }

            if (!onComma)
                throwSyntaxError(line + 1, "Variables couldn't be parsed in container.");
        }

        startLine = line;
    }

    return body;
}

BlockNode *AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction, bool inLoop)
{
    BlockNode *blockNode = new BlockNode();
    blockNode->setLine(startLine + 1);
    int tabs = getNumTabs(tokens[startLine]);

    for (int line = startLine; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
            continue;

        if (getNthToken(tokens[line], 1).getToken() == "IMPORT")
            throwSyntaxError(line + 1, "Import Statements must be declared at the top of the file.");

        int currentIndent = getNumTabs(tokens[line]);
        if (currentIndent < expectedIndent)
        {
            startLine = line;
            return blockNode;
        }

        if (getNthToken(tokens[line], 1).getValue() == "container")
        {
            if (inFunction)
                throwSyntaxError(line + 1, "Container can't be defined in a function.");
            startLine = line - 1;
            return blockNode;
        }

        if (getNthToken(tokens[line], 1).getValue() == "func")
        {
            if (inFunction)
                throwSyntaxError(line + 1, "Function can't be defined in a function.");
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

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line - 1, "Missing body in 'if' statement.");

            BranchBlockNode *branchBlock = new BranchBlockNode();
            IfStmtNode *ifStmt = new IfStmtNode(condition, block);
            ifStmt->setLine(line - 1);

            branchBlock->addBranch(ifStmt);
            blockNode->addStatement(branchBlock);
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

            if (blockNode->getNumStatements() == 0 || !dynamic_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]))
                throwSyntaxError(line, "'elseif' statement found without a preceding 'if' statement.");
            if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
                throwSyntaxError(line, "Expected ':' at the end of elseif statement.");
            Node *condition = buildIdentifier(tokens[line - 1], elseIfIndex + 1, tokens[line - 1].size() - 3);
            condition->setLine(line);

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line - 1, "Missing body in 'elseif' statement.");

            ElseIfStmtNode *elseIfStmt = new ElseIfStmtNode(condition, block);
            elseIfStmt->setLine(line - 1);

            BranchBlockNode *branchBlock = static_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]);
            branchBlock->addBranch(elseIfStmt);

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
            if (blockNode->getNumStatements() == 0 || !dynamic_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]))
                throwSyntaxError(line, "'else' statement found without a preceding 'if' statement.");
            int colonIndex = getTokenIndex(tokens[line - 1], "COLON");
            if (colonIndex == -1)
                throwSyntaxError(line, "Expected ':' at the end of else statement.");

            int numValidTokens = getNumValidTokens(tokens[line - 1]);
            if (numValidTokens != 2)
                throwSyntaxError(line, "Line couldn't be parsed.");

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line - 1, "Missing body in 'else' statement.");

            ElseStmtNode *elseStmt = new ElseStmtNode(block);
            elseStmt->setLine(line - 1);

            BranchBlockNode *branchBlock = static_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]);
            branchBlock->addBranch(elseStmt);

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

            BlockNode *block = buildBlock(tokens, line, currentIndent + 1, inFunction, true);
            if (block->getNumStatements() == 0)
                throwSyntaxError(line - 1, "Missing body in 'while' loop.");

            WhileNode *whileStmt = new WhileNode(condition, block);
            whileStmt->setLine(line - 1);
            blockNode->addStatement(whileStmt);
            line--;
            startLine = line;
            continue;
        }

        int assignIndex = getTokenIndex(tokens[line], "ASSIGN");
        if (assignIndex != -1)
        {
            if (getNumValidTokens(tokens[line]) > 2 && assignIndex > 0)
            {
                if (tokens[line][assignIndex - 1].getToken() != "IDENTIFIER")
                    throwSyntaxError(tokens[line][0].getLineNumber(), "'" + tokens[line][assignIndex - 1].getValue() + "' can't be used as a variable name.");
            }

            if (getNumValidTokens(tokens[line]) > 4 && assignIndex > 2)
            {
                if (tokens[line][assignIndex - 1].getToken() == "IDENTIFIER" && tokens[line][assignIndex - 2].getToken() == "DOUBLECOLON" && tokens[line][assignIndex - 3].getToken() == "IDENTIFIER")
                    throwSyntaxError(tokens[line][0].getLineNumber(), "Imported variables are read-only.");
            }

            std::string type = splitLine(tokens[line], tabs, assignIndex - 2);
            if (type != "" && !Token::isDataType(tokens[line][assignIndex - 2].getToken()) && tokens[line][assignIndex - 2].getToken() != "VAR" && tokens[line][assignIndex - 2].getToken() != "IDENTIFIER")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected type used for variable '" + tokens[line][assignIndex - 1].getValue() + "'.");

            AssignNode *assignStmt = new AssignNode(tokens[line][assignIndex - 1].getValue(), splitLine(tokens[line], tabs, assignIndex - 2), buildIdentifier(tokens[line], assignIndex + 1, tokens[line].size() - 2));
            assignStmt->setLine(line + 1);
            blockNode->addStatement(assignStmt);
            continue;
        }

        int operatorAssignIndex = getTokenIndex(tokens[line], {"PLUSASSIGN", "MINUSASSIGN", "STARASSIGN", "SLASHASSIGN", "EXPASSIGN"});
        if (operatorAssignIndex != -1)
        {
            if (getNumValidTokens(tokens[line]) > 2 && assignIndex > 0)
            {
                if (tokens[line][operatorAssignIndex - 1].getToken() != "IDENTIFIER")
                    throwSyntaxError(tokens[line][0].getLineNumber(), "'" + tokens[line][operatorAssignIndex - 1].getValue() + "' can't be used as a variable name.");
            }

            if (getNumValidTokens(tokens[line]) > 4 && assignIndex > 2)
            {
                if (tokens[line][assignIndex - 1].getToken() == "IDENTIFIER" && tokens[line][assignIndex - 2].getToken() == "DOUBLECOLON" && tokens[line][assignIndex - 3].getToken() == "IDENTIFIER")
                    throwSyntaxError(tokens[line][0].getLineNumber(), "Imported variables are read-only.");
            }

            std::string type = splitLine(tokens[line], tabs, operatorAssignIndex - 2);
            if (type != "")
                throwSyntaxError(tokens[line][0].getLineNumber(), "Can't assign a type for the variable '" + tokens[line][operatorAssignIndex - 1].getValue() + "'.");

            std::vector<Token> l;
            l.push_back(tokens[line][operatorAssignIndex - 1]);

            if (tokens[line][operatorAssignIndex].getToken() == "PLUSASSIGN")
                l.push_back(Token("+", line + 1, 0));
            if (tokens[line][operatorAssignIndex].getToken() == "MINUSASSIGN")
                l.push_back(Token("-", line + 1, 0));
            if (tokens[line][operatorAssignIndex].getToken() == "STARASSIGN")
                l.push_back(Token("*", line + 1, 0));
            if (tokens[line][operatorAssignIndex].getToken() == "SLASHASSIGN")
                l.push_back(Token("/", line + 1, 0));
            if (tokens[line][operatorAssignIndex].getToken() == "EXPASSIGN")
                l.push_back(Token("^", line + 1, 0));

            for (int i = operatorAssignIndex + 1; i < tokens[line].size() - 1; i++)
                l.push_back(tokens[line][i]);

            AssignNode *assignStmt = new AssignNode(tokens[line][operatorAssignIndex - 1].getValue(), "", buildIdentifier(l, 0, l.size() - 1));
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
        int breakIndex = getTokenIndex(tokens[line], "BREAK");
        if (breakIndex != -1)
        {
            if (!inLoop)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'break' statement found outside of loop.");
            if (getNumValidTokens(tokens[line]) != 1)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'break' statement shouldn't have anything after it.");
            else
            {
                BreakNode *breakStmt = new BreakNode();
                breakStmt->setLine(line + 1);
                blockNode->addStatement(breakStmt);
            }

            continue;
        }
        int skipIndex = getTokenIndex(tokens[line], "SKIP");
        if (skipIndex != -1)
        {
            if (!inLoop)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'skip' statement found outside of loop.");
            if (getNumValidTokens(tokens[line]) != 1)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'skip' statement shouldn't have anything after it.");
            else
            {
                SkipNode *skipStmt = new SkipNode();
                skipStmt->setLine(line + 1);
                blockNode->addStatement(skipStmt);
            }

            continue;
        }

        blockNode->addStatement(buildIdentifier(tokens[line], tabs, tokens[line].size() - 2));
        if (!dynamic_cast<CallExprNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1]))
            throwSyntaxError(line + 1, "Line couldn't be parsed.");

        startLine = line;
    }

    startLine = tokens.size() - 1;
    return blockNode;
}

void AbstractSyntaxTree::buildTree(const std::vector<std::vector<Token>> &tokens)
{
    bool checkForImports = true;
    bool checkForExports = true;
    std::vector<ImportNode *> importNodes;
    for (int line = 0; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
            continue;

        if (checkForExports)
        {
            if (getNthToken(tokens[line], 1).getToken() == "EXPORT")
            {
                buildExports(tokens, line);
                checkForExports = false;
                continue;
            }
        }

        if (checkForImports)
        {
            if (getNthToken(tokens[line], 1).getToken() == "IMPORT")
            {
                if (getNumValidTokens(tokens[line]) != 4)
                    throwSyntaxError(line + 1, "Import Statement couldn't be parsed");

                if ((getNthToken(tokens[line], 2).getToken() != "MODULE" && getNthToken(tokens[line], 2).getToken() != "IDENTIFIER" || getNthToken(tokens[line], 3).getToken() != "AS" || getNthToken(tokens[line], 4).getToken() != "IDENTIFIER"))
                    throwSyntaxError(line + 1, "Import Statement couldn't be parsed");

                std::string file = getNthToken(tokens[line], 2).getValue(), prefix = getNthToken(tokens[line], 4).getValue();
                ImportNode *i = new ImportNode(file, prefix);

                if (std::find(prefixes.begin(), prefixes.end(), prefix) != prefixes.end())
                    throwSyntaxError(line + 1, "Module alias '" + prefix + "' is already defined in this scope.");

                prefixes.push_back(prefix);
                prefixSymbols[prefix] = {};
                imports.push_back(file);
                importNodes.push_back(i);
                continue;
            }
            else
            {
                checkForImports = false;

                if (importNodes.size() > 0)
                {
                    BlockNode *imports = new BlockNode();
                    for (int i = 0; i < importNodes.size(); i++)
                        imports->addStatement(importNodes[i]);

                    root->addNode(imports);
                }
            }
        }

        if (getNthToken(tokens[line], 1).getToken() == "CONTAINER")
        {
            if (getNumValidTokens(tokens[line]) != 3)
                throwSyntaxError(line + 1, "Couldn't parse container statement.");

            if (getNthToken(tokens[line], 2).getToken() != "IDENTIFIER" && getNthToken(tokens[line], 3).getToken() != "COLON")
                throwSyntaxError(line + 1, "Couldn't parse container statement.");

            std::string name = getNthToken(tokens[line], 2).getValue();

            int origLine = line + 1;

            line++;

            checkForExports = false;

            std::vector<Node *> body = buildContainer(tokens, line, 1);

            line--;
            ContainerNode *container = new ContainerNode(name, body, origLine);
            root->addNode(container);

            continue;
        }

        // Functions need a minimum of 4 tokens
        if (getNthToken(tokens[line], 1).getToken() == "FUNC")
        {
            checkForExports = false;

            std::vector<std::string> information = getFunctionInformation(tokens[line]);
            if (std::find(Token::reservedFunctions.begin(), Token::reservedFunctions.end(), information[0]) != Token::reservedFunctions.end())
                throwSyntaxError(line + 1, "Reserved function name used: '" + information[0] + "'.");

            line++;

            std::vector<ParamNode *> parameters;

            for (int i = 2; i < information.size(); i++)
            {
                ParamNode *parameter = new ParamNode(information[i], line);
                parameters.push_back(parameter);
            }

            BlockNode *body = buildBlock(tokens, line, 1, true);
            if (body->getNumStatements() == 0)
            {
                if (tokens[body->getLine() - 2][tokens[body->getLine() - 2].size() - 2].getToken() == "COLON")
                    throwSyntaxError(body->getLine() - 1, "Missing body in function '" + information[0] + "'.");
            }
            else
            {
                if (tokens[body->getLine() - 2][tokens[body->getLine() - 2].size() - 2].getToken() != "COLON")
                    throwSyntaxError(body->getLine() - 1, "Colon missing for the function '" + information[0] + "'.");
            }
            line--;

            FuncNode *func = new FuncNode(information[0], information[1], parameters, body, line - 1);
            root->addNode(func);
            continue;
        }

        root->addNode(buildBlock(tokens, line, 0));
        checkForExports = false;
    }
}

void AbstractSyntaxTree::format()
{
    BlockNode *globalCode = new BlockNode();
    for (auto it = root->getChildrenRef().begin(); it != root->getChildrenRef().end();)
    {
        if (BlockNode *d = dynamic_cast<BlockNode *>(*it))
        {
            for (Node *stmt : d->getStatements())
                globalCode->addStatement(stmt);

            d->clearStatments();

            delete *it;
            it = root->getChildrenRef().erase(it);
        }
        else
            ++it;
    }

    root->setGlobalCode(globalCode);
}

bool AbstractSyntaxTree::isImported() const
{
    return imported;
}

void AbstractSyntaxTree::setImported(bool imported)
{
    this->imported = imported;
}

bool AbstractSyntaxTree::hasImports() const
{
    return imports.size() > 0;
}

std::vector<std::string> AbstractSyntaxTree::getPrefixes() const
{
    return prefixes;
}

std::unordered_map<std::string, std::unordered_map<std::string, Scope>> AbstractSyntaxTree::getPrefixSymbols() const
{
    return prefixSymbols;
}

void AbstractSyntaxTree::setSymbols(std::string prefix, std::unordered_map<std::string, Scope> symbols)
{
    prefixSymbols[prefix] = symbols;
}

std::vector<std::string> AbstractSyntaxTree::getImports() const
{
    return imports;
}

void AbstractSyntaxTree::setImports(std::vector<std::string> imports)
{
    this->imports = imports;
}

void AbstractSyntaxTree::printTree() const
{
    root->print();
}

void AbstractSyntaxTree::freeMemory()
{
    delete root;
    delete this;
}
