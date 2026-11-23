#include "compiler/ast.h"

// Get the escape sequence from a string
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
    root = std::make_unique<RootNode>();
    root->setFileName(fileName);
    RootNode::setCurrentFile(fileName);
    buildTree(tokens);
}

AbstractSyntaxTree::~AbstractSyntaxTree()
{
}

std::unique_ptr<RootNode> &AbstractSyntaxTree::getRoot()
{
    return root;
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

std::unordered_map<std::string, std::unordered_map<std::string, Scope>> &AbstractSyntaxTree::getPrefixSymbols()
{
    return prefixSymbols;
}

void AbstractSyntaxTree::setSymbols(std::string prefix, std::unordered_map<std::string, Scope> &symbols)
{
    prefixSymbols[prefix] = std::move(symbols);
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

void AbstractSyntaxTree::format()
{
    std::unique_ptr<BlockNode> globalCode(new BlockNode(1));
    for (auto it = root->getChildren().begin(); it != root->getChildren().end();)
    {
        if (auto *d = dynamic_cast<BlockNode *>((*it).get()))
        {
            for (std::unique_ptr<Node> &stmt : d->getStatements())
                globalCode->addStatement(std::move(stmt));

            d->clearStatments();
            it = root->getChildren().erase(it);
        }
        else
            ++it;
    }

    root->setGlobalCode(std::move(globalCode));
}

std::unordered_map<std::string, Scope> &AbstractSyntaxTree::getSymbolTable()
{
    return symbolTable;
}

void AbstractSyntaxTree::setSymbolTable(std::unordered_map<std::string, Scope> &symbolTable)
{
    for (std::unique_ptr<Symbol> &symbol : symbolTable["1"].getSymbols())
    {
        if (std::find(exports.begin(), exports.end(), symbol->getName()) != exports.end())
        {
            symbol->setUsed(true);
            symbol->setExported(true);
        }
    }

    this->symbolTable = std::move(symbolTable);
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
    return Token();
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

// Assumes Function isn't Tabbed
std::vector<std::vector<std::string>> AbstractSyntaxTree::getFunctionInformation(const std::vector<Token> &tokenLine, bool inContainer, std::string &name, std::string &returnType) const
{
    std::vector<std::vector<std::string>> parameters;
    Token nameToken = (inContainer) ? getNthToken(tokenLine, 1) : getNthToken(tokenLine, 2);

    if (inContainer)
    {
        if (getNthToken(tokenLine, 2).getToken() != "LPAREN")
            throwSyntaxError(nameToken.getLineNumber(), "Function couldn't be parsed.");
    }
    else
    {
        if (getNthToken(tokenLine, 3).getToken() != "LPAREN")
            throwSyntaxError(nameToken.getLineNumber(), "Function couldn't be parsed.");
    }

    if (nameToken.getToken() == "IDENTIFIER")
        name = nameToken.getValue();
    else
        throwSyntaxError(nameToken.getLineNumber(), "Unexpected identifier after 'func' found.");

    int arrowIndex = getTokenIndex(tokenLine, "ARROW");

    if (arrowIndex != -1)
    {
        if (!isDataType(tokenLine[arrowIndex + 1].getToken()) && tokenLine[arrowIndex + 1].getToken() != "IDENTIFIER")
            throwSyntaxError(tokenLine[arrowIndex + 1].getLineNumber(), "Unexpected idenifier '" + tokenLine[arrowIndex + 1].getValue() + "' used as the function return type.");
        returnType = tokenLine[arrowIndex + 1].getValue();
    }

    int colonIndex = getTokenIndex(tokenLine, "COLON", false);

    int startParams = 3;
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

    std::vector<std::string> currentParam;
    for (int i = startParams; i <= endParams; i++)
    {
        if (tokenLine[i].getToken() == "COMMA")
        {
            parameters.push_back(currentParam);
            currentParam.clear();
        }
        else
        {
            if (tokenLine[i].getToken() == "IDENTIFIER" || tokenLine[i].getToken() == "REF" || isDataType(tokenLine[i].getToken()))
                currentParam.push_back(tokenLine[i].getValue());
            else
                throwSyntaxError(tokenLine[arrowIndex + 1].getLineNumber(), "Unexpected idenifier '" + tokenLine[i].getValue() + "' used in the function parameters.");
        }
    }

    if (currentParam.size() > 0)
        parameters.push_back(currentParam);

    return parameters;
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

std::unique_ptr<Node> AbstractSyntaxTree::buildLiteral(const std::vector<Token> &line, int start)
{
    std::string tokenType = line[start].getToken();
    if (tokenType == "CHAR")
    {
        char c = extractChar(line[start].getValue().substr(1, line[start].getValue().size() - 2));
        std::unique_ptr<LiteralNode<char>> literal = std::make_unique<LiteralNode<char>>(c, line[start].getToken(), line[start].getLineNumber());
        return literal;
    }
    else if (tokenType == "STRING")
    {
        std::unique_ptr<LiteralNode<std::string>> literal = std::make_unique<LiteralNode<std::string>>(line[start].getValue(), line[start].getToken(), line[start].getLineNumber());
        literal->setLine(line[start].getLineNumber());
        return literal;
    }
    else if (tokenType == "INT")
    {
        std::unique_ptr<LiteralNode<int>> literal = std::make_unique<LiteralNode<int>>(std::stoi(line[start].getValue()), line[start].getToken(), line[start].getLineNumber());
        literal->setLine(line[start].getLineNumber());
        return literal;
    }
    else if (tokenType == "DECIMAL")
    {
        std::unique_ptr<LiteralNode<double>> literal = std::make_unique<LiteralNode<double>>(std::stod(line[start].getValue()), line[start].getToken(), line[start].getLineNumber());
        literal->setLine(line[start].getLineNumber());
        return literal;
    }
    else if (tokenType == "BOOL")
    {
        if (line[start].getValue() == "true")
        {
            std::unique_ptr<LiteralNode<bool>> literal = std::make_unique<LiteralNode<bool>>(true, line[start].getToken(), line[start].getLineNumber());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
        else if (line[start].getValue() == "false")
        {
            std::unique_ptr<LiteralNode<bool>> literal = std::make_unique<LiteralNode<bool>>(false, line[start].getToken(), line[start].getLineNumber());
            literal->setLine(line[start].getLineNumber());
            return literal;
        }
    }

    throwSyntaxError(line[start].getLineNumber(), "Unexpected Literal '" + line[start].getValue() + "' found.");
    return nullptr;
}

std::unique_ptr<CallExprNode> AbstractSyntaxTree::buildCallExpr(const std::vector<Token> &line, int start, int end)
{
    bool callExpr = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "LPAREN" && line[end].getToken() == "RPAREN";
    bool importedCallExpr = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "DOUBLECOLON" && line[start + 2].getToken() == "IDENTIFIER" && line[start + 3].getToken() == "LPAREN" && line[end].getToken() == "RPAREN";

    if (callExpr || importedCallExpr)
    {
        std::vector<std::unique_ptr<Node>> arguments;
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

        std::string name = (callExpr) ? line[start].getValue() : line[start + 2].getValue();
        bool inContainer = std::find(containers.begin(), containers.end(), name) != containers.end();
        std::unique_ptr<CallExprNode> callExpression = std::make_unique<CallExprNode>(name, arguments, line[start].getLineNumber());

        if (importedCallExpr)
            callExpression->setImported(line[start].getValue());

        return callExpression;
    }

    throwSyntaxError(line[0].getLineNumber(), "Line couldn't be parsed.");
    return nullptr;
}

std::unique_ptr<Node> AbstractSyntaxTree::buildExpression(const std::vector<Token> &line, int start, int end)
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
                        if ((line[i].getValue() == "+" || line[i].getValue() == "-"))
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

    if (splitIndex != -1 && splitIndex != start)
    {
        std::unique_ptr<BinaryExprNode> binaryExpr = std::make_unique<BinaryExprNode>(line[splitIndex].getValue(), buildIdentifier(line, start, splitIndex - 1), buildIdentifier(line, splitIndex + 1, end), line[splitIndex].getLineNumber());
        return binaryExpr;
    }

    if (line[start].getToken() == "PLUS" || line[start].getToken() == "MINUS" || line[start].getToken() == "NOT")
    {
        int lparen = 0, rparen = 0, endIdentifier = end;
        for (int i = start + 1; i <= end; i++)
        {
            endIdentifier = i;
            if (line[i].getToken() == "LPAREN")
                lparen++;
            if (line[i].getToken() == "RPAREN")
                rparen++;
            if (std::find(Token::operators.begin(), Token::operators.end(), line[i].getValue()) != Token::operators.end() && lparen != rparen)
                endIdentifier = i - 1;
        }

        std::unique_ptr<UnaryExprNode> unaryExpr = std::make_unique<UnaryExprNode>(line[start].getValue(), buildIdentifier(line, start + 1, endIdentifier), line[start].getLineNumber());
        return unaryExpr;
    }

    return nullptr;
}

std::unique_ptr<Node> AbstractSyntaxTree::buildIdentifier(const std::vector<Token> &line, int start, int end)
{
    if (end - start == 0)
    {
        if (line[start].getToken() == "IDENTIFIER")
        {
            std::unique_ptr<IdentifierNode> identifier = std::make_unique<IdentifierNode>(line[start].getValue(), line[start].getLineNumber());
            return identifier;
        }
        else
            return buildLiteral(line, start);
    }
    else if (hasOperator(line, start, end))
    {
        return buildExpression(line, start, end);
    }
    else
    {
        if (line[start].getToken() == "LPAREN" && line[end].getToken() == "RPAREN")
            return buildIdentifier(line, start + 1, end - 1);

        bool importedIdentifier = line[start].getToken() == "IDENTIFIER" && line[start + 1].getToken() == "DOUBLECOLON" && line[end].getToken() == "IDENTIFIER";

        if (importedIdentifier)
        {
            std::unique_ptr<IdentifierNode> identifier = std::make_unique<IdentifierNode>(line[start + 2].getValue(), line[start].getLineNumber());
            identifier->setImported(line[start].getValue());
            return identifier;
        }

        return buildCallExpr(line, start, end);
    }
}

std::unique_ptr<BranchBlockNode> AbstractSyntaxTree::getIfStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop)
{
    if (getNthToken(tokens[line], 1).getToken() != "IF")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'if' statement found.");

    line++;
    int ifLine = line;

    if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
        throwSyntaxError(line - 1, "Expected ':' at the end of 'if' statement.");

    std::unique_ptr<Node> condition = buildIdentifier(tokens[line - 1], index + 1, tokens[line - 1].size() - 3);

    std::unique_ptr<BlockNode> block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
    if (block->getNumStatements() == 0)
        throwSyntaxError(ifLine, "Missing body in 'if' statement.");

    std::unique_ptr<BranchBlockNode> branchBlock = std::make_unique<BranchBlockNode>(ifLine + 1);
    std::unique_ptr<IfStmtNode> ifStmt(new IfStmtNode(condition, block, ifLine));

    branchBlock->addBranch(std::move(ifStmt));
    return branchBlock;
}

void AbstractSyntaxTree::addElseIfStatement(int index, std::unique_ptr<BlockNode> &blockNode, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop)
{
    if (getNthToken(tokens[line], 1).getToken() != "ELSEIF")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'elseif' statement found.");

    line++;
    int elseIfLine = line;

    if (blockNode->getNumStatements() == 0 || !dynamic_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1].get()))
        throwSyntaxError(line, "'elseif' statement found without a preceding 'if' statement.");
    if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
        throwSyntaxError(line - 1, "Expected ':' at the end of elseif statement.");
    std::unique_ptr<Node> condition = buildIdentifier(tokens[line - 1], index + 1, tokens[line - 1].size() - 3);
    condition->setLine(line);

    std::unique_ptr<BlockNode> block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
    if (block->getNumStatements() == 0)
        throwSyntaxError(elseIfLine, "Missing body in 'elseif' statement.");

    std::unique_ptr<ElseIfStmtNode> elseIfStmt(new ElseIfStmtNode(condition, block, elseIfLine));

    auto *branchBlock = static_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1].get());
    branchBlock->addBranch(std::move(elseIfStmt));
}

void AbstractSyntaxTree::addElseStatement(int index, std::unique_ptr<BlockNode> &blockNode, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop)
{
    if (getNthToken(tokens[line], 1).getToken() != "ELSE")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'else' statement found.");

    line++;
    int elseLine = line;

    if (blockNode->getNumStatements() == 0 || !dynamic_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1].get()))
        throwSyntaxError(line, "'else' statement found without a preceding 'if' statement.");
    int colonIndex = getTokenIndex(tokens[line - 1], "COLON");
    if (colonIndex == -1)
        throwSyntaxError(line - 1, "Expected ':' at the end of else statement.");

    int numValidTokens = getNumValidTokens(tokens[line - 1]);
    if (numValidTokens != 2)
        throwSyntaxError(line - 1, "Line couldn't be parsed.");

    std::unique_ptr<BlockNode> block = buildBlock(tokens, line, currentIndent + 1, inFunction, inLoop);
    if (block->getNumStatements() == 0)
        throwSyntaxError(elseLine, "Missing body in 'else' statement.");

    std::unique_ptr<ElseStmtNode> elseStmt(new ElseStmtNode(block, elseLine));

    auto *branchBlock = static_cast<BranchBlockNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1].get());
    branchBlock->addBranch(std::move(elseStmt));
}

std::unique_ptr<WhileNode> AbstractSyntaxTree::getWhileStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int currentIndent, bool inFunction, bool inLoop)
{
    if (getNthToken(tokens[line], 1).getToken() != "WHILE")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected 'while' statement found.");

    line++;
    int whileLine = line;

    if (getNthToken(tokens[line - 1], -1).getToken() != "COLON")
        throwSyntaxError(line, "Expected ':' at the end of 'while' loop.");
    std::unique_ptr<Node> condition = buildIdentifier(tokens[line - 1], index + 1, tokens[line - 1].size() - 3);
    condition->setLine(line - 1);

    std::unique_ptr<BlockNode> block = buildBlock(tokens, line, currentIndent + 1, inFunction, true);
    if (block->getNumStatements() == 0)
        throwSyntaxError(whileLine, "Missing body in 'while' loop.");

    std::unique_ptr<WhileNode> whileStmt(new WhileNode(condition, block, whileLine));

    return whileStmt;
}

std::unique_ptr<AssignNode> AbstractSyntaxTree::getAssignStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int tabs)
{
    if (getNumValidTokens(tokens[line]) > 2 && index > 0)
    {
        if (tokens[line][index - 1].getToken() != "IDENTIFIER")
            throwSyntaxError(tokens[line][0].getLineNumber(), "'" + tokens[line][index - 1].getValue() + "' can't be used as a variable name.");
    }

    if (getNumValidTokens(tokens[line]) > 4 && index > 2)
    {
        if (tokens[line][index - 1].getToken() == "IDENTIFIER" && tokens[line][index - 2].getToken() == "DOUBLECOLON" && tokens[line][index - 3].getToken() == "IDENTIFIER")
            throwSyntaxError(tokens[line][0].getLineNumber(), "Imported variables are read-only.");
    }

    std::string type = splitLine(tokens[line], tabs, index - 2);
    std::string typeName = tokens[line][index - 2].getToken();
    if (type != "" && !isDataType(typeName) && typeName != "VAR" && typeName != "IDENTIFIER")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Unexpected type used for variable '" + tokens[line][index - 1].getValue() + "'.");

    std::unique_ptr<Node> value = buildIdentifier(tokens[line], index + 1, tokens[line].size() - 2);
    std::unique_ptr<AssignNode> assignStmt(new AssignNode(tokens[line][index - 1].getValue(), splitLine(tokens[line], tabs, index - 2), value, line + 1));

    return assignStmt;
}

std::unique_ptr<AssignNode> AbstractSyntaxTree::getOperatorAssignStatement(int index, const std::vector<std::vector<Token>> &tokens, int &line, int tabs)
{
    if (getNumValidTokens(tokens[line]) > 2 && index > 0)
    {
        if (tokens[line][index - 1].getToken() != "IDENTIFIER")
            throwSyntaxError(tokens[line][0].getLineNumber(), "'" + tokens[line][index - 1].getValue() + "' can't be used as a variable name.");
    }

    if (getNumValidTokens(tokens[line]) > 4 && index > 2)
    {
        if (tokens[line][index - 1].getToken() == "IDENTIFIER" && tokens[line][index - 2].getToken() == "DOUBLECOLON" && tokens[line][index - 3].getToken() == "IDENTIFIER")
            throwSyntaxError(tokens[line][0].getLineNumber(), "Imported variables are read-only.");
    }

    std::string type = splitLine(tokens[line], tabs, index - 2);
    if (type != "")
        throwSyntaxError(tokens[line][0].getLineNumber(), "Can't assign a type for the variable '" + tokens[line][index - 1].getValue() + "'.");

    std::vector<Token> l;
    l.push_back(tokens[line][index - 1]);

    if (tokens[line][index].getToken() == "PLUSASSIGN")
        l.push_back(Token("+", line + 1, 0));
    if (tokens[line][index].getToken() == "MINUSASSIGN")
        l.push_back(Token("-", line + 1, 0));
    if (tokens[line][index].getToken() == "STARASSIGN")
        l.push_back(Token("*", line + 1, 0));
    if (tokens[line][index].getToken() == "SLASHASSIGN")
        l.push_back(Token("/", line + 1, 0));
    if (tokens[line][index].getToken() == "EXPASSIGN")
        l.push_back(Token("^", line + 1, 0));

    for (int i = index + 1; i < tokens[line].size() - 1; i++)
        l.push_back(tokens[line][i]);

    std::unique_ptr<Node> value = buildIdentifier(l, 0, l.size() - 1);
    std::unique_ptr<AssignNode> assignStmt(new AssignNode(tokens[line][index - 1].getValue(), "", value, line + 1));

    return assignStmt;
}

std::vector<std::unique_ptr<Node>> AbstractSyntaxTree::buildContainer(const std::string &name, const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent)
{
    containers.push_back(name);

    std::vector<std::unique_ptr<Node>> body;
    for (int line = startLine; line < tokens.size(); line++)
    {
        if (!isValidLine(tokens[line]))
        {
            startLine = line;
            continue;
        }

        int currentIndent = getNumTabs(tokens[line]);
        if (currentIndent < expectedIndent)
        {
            startLine = line;
            return body;
        }

        if (getNthToken(tokens[line], 1).getToken() == "IDENTIFIER" && getNthToken(tokens[line], 2).getToken() == "LPAREN")
        {
            std::string funcName, returnType;
            std::vector<std::vector<std::string>> params = getFunctionInformation(tokens[line], true, funcName, returnType);
            if (std::find(Token::reservedFunctions.begin(), Token::reservedFunctions.end(), funcName) != Token::reservedFunctions.end())
                throwSyntaxError(line + 1, "Reserved function name used: '" + funcName + "'.");

            line++;

            std::vector<std::unique_ptr<ParamNode>> parameters;

            for (int i = 0; i < params.size(); i++)
            {
                std::unique_ptr<ParamNode> parameter(new ParamNode(params[i], line));
                parameters.push_back(std::move(parameter));
            }

            std::unique_ptr<BlockNode> funcBody = buildBlock(tokens, line, 2, true);
            if (funcBody->getNumStatements() == 0)
            {
                if (funcName == name && returnType != "" && returnType != name)
                    throwSyntaxError(funcBody->getLine() - 1, "Constructor must return '" + funcName + "'.");
                if (tokens[funcBody->getLine() - 2][tokens[funcBody->getLine() - 2].size() - 2].getToken() == "COLON")
                    throwSyntaxError(funcBody->getLine() - 1, "Missing body in function '" + funcName + "'.");
            }
            else
            {
                if (tokens[funcBody->getLine() - 2][tokens[funcBody->getLine() - 2].size() - 2].getToken() != "COLON")
                    throwSyntaxError(funcBody->getLine() - 1, "Colon missing for the function '" + funcName + "'.");
            }
            line--;

            std::unique_ptr<FuncNode> func = std::make_unique<FuncNode>(funcName, returnType, parameters, funcBody, line);
            if (func->getName() == name && func->getReturnType() == "")
                func->setReturnType(name);

            body.push_back(std::move(func));

            startLine = line;
            continue;
        }

        if (isDataType(getNthToken(tokens[line], 1).getToken()) || getNthToken(tokens[line], 1).getToken() == "IDENTIFIER")
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
                        body.push_back(std::make_unique<IdentifierNode>(tokens[line][token].getValue(), type, line + 1));
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

std::unique_ptr<BlockNode> AbstractSyntaxTree::buildBlock(const std::vector<std::vector<Token>> &tokens, int &startLine, int expectedIndent, bool inFunction, bool inLoop)
{
    std::unique_ptr<BlockNode> blockNode = std::make_unique<BlockNode>(startLine + 1);
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

        if (getNthToken(tokens[line], 1).getToken() == "CONTAINER")
        {
            if (inFunction)
                throwSyntaxError(line + 1, "Container can't be defined in a function.");
            startLine = line - 1;
            return blockNode;
        }

        if (getNthToken(tokens[line], 1).getToken() == "FUNC")
        {
            if (inFunction)
                throwSyntaxError(line + 1, "Function can't be defined in a function.");
            startLine = line - 1;
            return blockNode;
        }

        int ifIndex = getTokenIndex(tokens[line], "IF");
        int elseIfIndex = getTokenIndex(tokens[line], "ELSEIF");
        int elseIndex = getTokenIndex(tokens[line], "ELSE");
        int whileIndex = getTokenIndex(tokens[line], "WHILE");
        int assignIndex = getTokenIndex(tokens[line], "ASSIGN");
        int operatorAssignIndex = getTokenIndex(tokens[line], {"PLUSASSIGN", "MINUSASSIGN", "STARASSIGN", "SLASHASSIGN", "EXPASSIGN"});
        int returnIndex = getTokenIndex(tokens[line], "RETURN");
        int breakIndex = getTokenIndex(tokens[line], "BREAK");
        int skipIndex = getTokenIndex(tokens[line], "SKIP");

        if (ifIndex != -1)
        {
            blockNode->addStatement(getIfStatement(ifIndex, tokens, line, currentIndent, inFunction, inLoop));
            startLine = line;
            line--;
        }
        else if (elseIfIndex != -1)
        {
            addElseIfStatement(elseIfIndex, blockNode, tokens, line, currentIndent, inFunction, inLoop);
            startLine = line;
            line--;
        }
        else if (elseIndex != -1)
        {
            addElseStatement(elseIndex, blockNode, tokens, line, currentIndent, inFunction, inLoop);
            startLine = line;
            line--;
        }
        else if (whileIndex != -1)
        {
            blockNode->addStatement(getWhileStatement(whileIndex, tokens, line, currentIndent, inFunction, inLoop));
            line--;
            startLine = line;
        }
        else if (assignIndex != -1)
        {
            blockNode->addStatement(getAssignStatement(assignIndex, tokens, line, tabs));
        }
        else if (operatorAssignIndex != -1)
        {
            blockNode->addStatement(getOperatorAssignStatement(operatorAssignIndex, tokens, line, tabs));
        }
        else if (returnIndex != -1)
        {
            if (!inFunction)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'return' statement found outside of a function.");
            if (getNumValidTokens(tokens[line]) < 2)
            {
                std::unique_ptr<ReturnStmtNode> returnStmt(new ReturnStmtNode(line + 1));
                blockNode->addStatement(std::move(returnStmt));
            }
            else
            {
                std::unique_ptr<ReturnStmtNode> returnStmt(new ReturnStmtNode(line + 1, buildIdentifier(tokens[line], returnIndex + 1, tokens[line].size() - 2)));
                blockNode->addStatement(std::move(returnStmt));
            }
        }
        else if (breakIndex != -1)
        {
            if (!inLoop)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'break' statement found outside of loop.");
            if (getNumValidTokens(tokens[line]) != 1)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'break' statement shouldn't have anything after it.");
            else
            {
                std::unique_ptr<BreakNode> breakStmt = std::make_unique<BreakNode>(line + 1);
                blockNode->addStatement(std::move(breakStmt));
            }
        }
        else if (skipIndex != -1)
        {
            if (!inLoop)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'skip' statement found outside of loop.");
            if (getNumValidTokens(tokens[line]) != 1)
                throwSyntaxError(tokens[line][0].getLineNumber(), "'skip' statement shouldn't have anything after it.");
            else
            {
                std::unique_ptr<SkipNode> skipStmt = std::make_unique<SkipNode>(line + 1);
                blockNode->addStatement(std::move(skipStmt));
            }
        }
        else
        {
            blockNode->addStatement(buildIdentifier(tokens[line], tabs, tokens[line].size() - 2));

            if (!dynamic_cast<CallExprNode *>(blockNode->getStatements()[blockNode->getNumStatements() - 1].get()))
                throwSyntaxError(line + 1, "Line couldn't be parsed.");
        }

        startLine = line;
    }

    startLine = tokens.size() - 1;
    return blockNode;
}

void AbstractSyntaxTree::addImport(const std::vector<std::vector<Token>> &tokens, int &line)
{
    if (getNumValidTokens(tokens[line]) != 4)
        throwSyntaxError(line + 1, "Import Statement couldn't be parsed");

    if ((getNthToken(tokens[line], 2).getToken() != "MODULE" && getNthToken(tokens[line], 2).getToken() != "IDENTIFIER" || getNthToken(tokens[line], 3).getToken() != "AS" || getNthToken(tokens[line], 4).getToken() != "IDENTIFIER"))
        throwSyntaxError(line + 1, "Import Statement couldn't be parsed");

    std::string file = getNthToken(tokens[line], 2).getValue(), prefix = getNthToken(tokens[line], 4).getValue();
    std::unique_ptr<ImportNode> i(new ImportNode(file, prefix));

    if (std::find(prefixes.begin(), prefixes.end(), prefix) != prefixes.end())
        throwSyntaxError(line, "Module alias '" + prefix + "' is already defined in this scope.");

    prefixes.push_back(prefix);
    prefixSymbols[prefix].clear();
    imports.push_back(file);
    importNodes.push_back(std::move(i));
}

void AbstractSyntaxTree::addFunction(const std::vector<std::vector<Token>> &tokens, int &line)
{
    std::string funcName, returnType;
    std::vector<std::vector<std::string>> params = getFunctionInformation(tokens[line], false, funcName, returnType);
    if (std::find(Token::reservedFunctions.begin(), Token::reservedFunctions.end(), funcName) != Token::reservedFunctions.end())
        throwSyntaxError(line + 1, "Reserved function name used: '" + funcName + "'.");

    line++;
    int startLine = line;

    std::vector<std::unique_ptr<ParamNode>> parameters;

    for (int i = 0; i < params.size(); i++)
    {
        std::unique_ptr<ParamNode> parameter = std::make_unique<ParamNode>(params[i], line);
        parameters.push_back(std::move(parameter));
    }

    std::unique_ptr<BlockNode> body = buildBlock(tokens, line, 1, true);
    if (body->getNumStatements() == 0)
    {
        if (tokens[body->getLine() - 2][tokens[body->getLine() - 2].size() - 2].getToken() == "COLON")
            throwSyntaxError(body->getLine() - 1, "Missing body in function '" + funcName + "'.");
    }
    else
    {
        if (tokens[body->getLine() - 2][tokens[body->getLine() - 2].size() - 2].getToken() != "COLON")
            throwSyntaxError(body->getLine() - 1, "Colon missing for the function '" + funcName + "'.");
    }
    line--;

    std::unique_ptr<FuncNode> func(new FuncNode(funcName, returnType, parameters, body, startLine));
    root->addNode(std::move(func));
}

void AbstractSyntaxTree::buildTree(const std::vector<std::vector<Token>> &tokens)
{
    bool checkForImports = true;
    bool checkForExports = true;
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
                addImport(tokens, line);
                continue;
            }
            else
            {
                checkForImports = false;

                if (importNodes.size() > 0)
                {
                    std::unique_ptr<BlockNode> imports = std::make_unique<BlockNode>(line + 1);
                    for (int i = 0; i < importNodes.size(); i++)
                        imports->addStatement(std::move(importNodes[i]));

                    root->addNode(std::move(imports));
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

            std::vector<std::unique_ptr<Node>> body = buildContainer(name, tokens, line, 1);

            line--;
            auto container = std::make_unique<ContainerNode>(name, std::move(body), origLine);
            root->addNode(std::move(container));

            continue;
        }

        // Functions need a minimum of 4 tokens
        if (getNthToken(tokens[line], 1).getToken() == "FUNC")
        {
            checkForExports = false;
            addFunction(tokens, line);
            continue;
        }

        root->addNode(buildBlock(tokens, line, 0));
        checkForExports = false;
    }
}
