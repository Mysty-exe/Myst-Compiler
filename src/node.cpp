#include "compiler/node.h"

int DIVBYZERO = 0;

void throwLexicalError(int line, std::string message)
{
    throw LexicalError(RootNode::getCurrentFile() + " - Lexical Error on line " + std::to_string(line) + ": " + message);
}

void throwSyntaxError(int line, std::string message)
{
    throw SyntaxError(RootNode::getCurrentFile() + " - Syntax Error on line " + std::to_string(line) + ": " + message);
}

void throwSemanticWarning(int line, std::string message)
{
    std::cout << "\033[38;5;208m";
    std::cout << RootNode::getCurrentFile() << " - Warning on line " << line << ": ";
    std::cerr << "\033[0m";
    std::cout << message << std::endl;
}

void throwSemanticError(int line, std::string message)
{
    throw SemanticError(RootNode::getCurrentFile() + " - Semantic Error on line " + std::to_string(line) + ": " + message);
}

void checkForDivisionByZero(std::string tempVar, std::string type, std::string &ir)
{
    if (type == "INTTYPE")
        ir += "%.is_zero_" + std::to_string(DIVBYZERO) + " = icmp eq i32 " + tempVar + ", 0";
    else
        ir += "%.is_zero_" + std::to_string(DIVBYZERO) + " = fcmp oeq double " + tempVar + ", 0.0";

    ir += "\nbr i1 %.is_zero_" + std::to_string(DIVBYZERO) + ", label %div_error_" + std::to_string(DIVBYZERO) + ", label %div_normal_" + std::to_string(DIVBYZERO) + "\n";
    ir += "div_error_" + std::to_string(DIVBYZERO) + ":\n";
    ir += R"(call void @print_runtime_error(ptr @div_by_zero_msg)
    call void @myst_exit(i64 1)
    unreachable

)";
    ir += "div_normal_" + std::to_string(DIVBYZERO) + ":\n";
    DIVBYZERO++;
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

std::string mapTypeIR(const std::string &type)
{
    if (type == "CHARTYPE")
        return "i8";
    if (type == "INTTYPE")
        return "i32";
    if (type == "DECIMALTYPE")
        return "double";
    if (type == "BOOLTYPE")
        return "i1";

    return "void";
}

std::string mapNumBytesIR(const std::string &type)
{
    if (type == "CHARTYPE")
        return "1";
    if (type == "INTTYPE")
        return "4";
    if (type == "DECIMALTYPE")
        return "8";
    if (type == "BOOLTYPE")
        return "1";

    return "0";
}

bool canCoerce(std::string origType, std::string newType)
{
    if (origType == "INTTYPE" && newType == "DECIMALTYPE")
        return true;
    if (origType == "DECIMALTYPE" && newType == "INTTYPE")
        return true;
    if (origType == "CHARTYPE" && newType == "INTTYPE")
        return true;
    if (origType == "BOOLTYPE" && newType == "INTTYPE")
        return true;

    return false;
}

std::string mapOperatorIR(const std::string &op, const std::string &type)
{
    if (op == "+")
    {
        if (type == "INTTYPE")
            return "add nsw";
        if (type == "DECIMALTYPE")
            return "fadd";
    }
    if (op == "-")
    {
        if (type == "INTTYPE")
            return "sub";
        if (type == "DECIMALTYPE")
            return "fsub";
    }
    if (op == "*")
    {
        if (type == "INTTYPE")
            return "mul";
        if (type == "DECIMALTYPE")
            return "fmul";
    }
    if (op == "/")
    {
        if (type == "INTTYPE")
            return "sdiv";
        if (type == "DECIMALTYPE")
            return "fdiv";
    }
    if (op == "%")
    {
        if (type == "INTTYPE")
            return "srem";
        if (type == "DECIMALTYPE")
            return "frem";
    }
    if (op == "==")
    {
        if (type == "DECIMALTYPE")
            return "fcmp oeq";
        else
            return "icmp eq";
    }
    if (op == "!=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp one";
        else
            return "icmp ne";
    }
    if (op == "<")
    {
        if (type == "DECIMALTYPE")
            return "fcmp olt";
        else
            return "icmp slt";
    }
    if (op == "<=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp ole";
        else
            return "icmp sle";
    }
    if (op == ">")
    {
        if (type == "DECIMALTYPE")
            return "fcmp ogt";
        else
            return "icmp sgt";
    }
    if (op == ">=")
    {
        if (type == "DECIMALTYPE")
            return "fcmp oge";
        else
            return "icmp sge";
    }
    if (op == "and" || op == "&&")
        return "and";
    if (op == "or" || op == "||")
        return "or";

    return op;
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

void Node::lookForCallExprs(Table &table)
{
}

bool Node::checkType(Table &table, const std::string &type)
{
    return true;
}
void Node::checkSemantics(Table &table)
{
}
std::string Node::inferType(Table &table)
{
    return "";
}
void Node::loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global, std::string prefix)
{
}
void Node::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global, std::string prefix)
{
}
void Node::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
}

void Node::findReturnStmts(std::vector<Node *> &stmts)
{
}

std::string RootNode::currentFile = "";
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
std::string RootNode::getCurrentFile()
{
    return currentFile;
}
std::string RootNode::getFileName()
{
    return fileName;
}
std::string RootNode::getFileNameWDots()
{
    return fileNameWDots;
}
void RootNode::setFileName(std::string name)
{
    fileName = name;

    fileNameWDots = "";
    for (int i = 0; i < fileName.size() - 3; i++)
    {
        if (fileName[i] == '/' || fileName[i] == '\\')
            fileNameWDots += '.';
        else
            fileNameWDots += fileName[i];
    }
}
void RootNode::setCurrentFile(std::string name)
{
    currentFile = name;
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
    std::cout << "RootNode: " + fileName << std::endl;
    for (Node *node : children)
    {
        node->print(depth + 1);
        std::cout << std::endl;
    }
}
void RootNode::checkSemantics(Table &table)
{
    table.addVisibleScope();

    std::vector<AssignNode *> assignments;
    for (Node *child : children)
    {
        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->addSignatureToTable(table, false);

        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (AssignNode *assignment = dynamic_cast<AssignNode *>(stmt))
                {
                    assignment->checkSemantics(table);
                    assignments.push_back(assignment);
                }
            }
        }
    }

    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        auto symbols = scope.getSymbols();
        auto it = symbols.begin();
        while (it != symbols.end())
        {
            if (FuncSymbol *symbol = dynamic_cast<FuncSymbol *>(*it))
                it = symbols.erase(it);
            else
                ++it;
        }
        scope.setSymbols(symbols);
        table.setScope(table.getVisibleScope()[i], scope);
    }

    for (Node *child : children)
    {
        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (dynamic_cast<AssignNode *>(stmt) == nullptr)
                    stmt->checkSemantics(table);
            }
        }

        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->checkSemantics(table);
    }

    for (AssignNode *assign : assignments)
        assign->lookForCallExprs(table);
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
                if (!derivedSymbol->hasReturn() && getDataTypeStr(derivedSymbol->getDataType()) != "void")
                    throwSemanticWarning(derivedSymbol->getLine(), "Function '" + symbol->getName() + "' returns '" + getDataTypeStr(derivedSymbol->getDataType()) + "' but return statement not found.");
            }
        }
    }
}

ImportNode::ImportNode(std::string fileName, std::string moduleName)
{
    this->fileName = fileName;
    this->moduleName = moduleName;
}

ImportNode::~ImportNode()
{
}

void ImportNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ImportNode: " << std::endl;
    std::cout << depthTabs << "\tFolder/File Name: " << fileName << std::endl;
    std::cout << depthTabs << "\tModule Name: " << moduleName << std::endl;
}

IdentifierNode::IdentifierNode(std::string name)
{
    this->name = name;
    this->type = TokenType::UNKNOWN;
    imported = false;
    modName = "";
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
std::string IdentifierNode::getModName() const
{
    return modName;
}
bool IdentifierNode::isImported()
{
    return imported;
}
void IdentifierNode::setImported(std::string modName)
{
    imported = true;
    this->modName = modName;
}
void IdentifierNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    if (imported)
    {
        std::cout << depthTabs << "Imported IdentifierNode: " << getDataTypeStr(Token::getType(type)) << " " << name << std::endl;
        std::cout << depthTabs << "\tFrom Module: " + modName << std::endl;
    }
    else
        std::cout << depthTabs << "IdentifierNode: " << getDataTypeStr(Token::getType(type)) << " " << name << std::endl;
}
bool IdentifierNode::checkType(Table &table, const std::string &type)
{
    if (imported)
    {
        auto scope = table.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name && symbol->isExported())
            {
                if (symbol->getDataType() == type)
                    this->type = Token::getTokenDataType(symbol->getDataType());

                return (symbol->getDataType() == type);
            }
        }

        throwSemanticError(line, "'" + name + "' was not found in Module '" + modName + "'.");
        return false;
    }

    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
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
std::string IdentifierNode::inferType(Table &table)
{
    if (imported)
    {
        auto scope = table.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name && symbol->isExported())
            {
                this->type = Token::getTokenDataType(symbol->getDataType());
                return symbol->getDataType();
            }
        }

        throwSemanticError(line, "'" + name + "' was not found in Module '" + modName + "'.");
        return "";
    }

    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
                this->type = Token::getTokenDataType(symbol->getDataType());
                return symbol->getDataType();
            }
        }
    }

    throwSemanticError(line, "'" + name + "' hasn't been declared.");
    return "";
}
void IdentifierNode::loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (std::find(loadedVars.begin(), loadedVars.end(), name) != loadedVars.end())
        return;

    if (imported)
        prefix = "." + prefixImports.at(modName) + ".";

    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    if (global)
    {
        ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr @" + prefix + name + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({name, "val"});
        if (Token::getType(type) != newType)
        {
            if (Token::getType(type) == "INTTYPE")
            {
                if (newType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + tempVarName + " to double\n";
                if (newType == "CHARTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = trunc i32 %" + tempVarName + " to i8\n";
                if (newType == "BOOLTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = icmp ne i32 %" + tempVarName + ", 0\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "CHARTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i8 %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "BOOLTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i1 %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
        }
        loadedVars.push_back(name);
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if (tempvarMap[i][j][0].substr(0, 7) == ".ifnode" || tempvarMap[i][j][0].substr(0, 11) == ".elseifnode" || tempvarMap[i][j][0].substr(0, 9) == ".elsenode" || tempvarMap[i][j][0].substr(0, 10) == ".whilenode")
                outOfScope -= 1;
            if (tempvarMap[i][j][0] == ".endScope")
                outOfScope += 1;
            if (outOfScope > 0)
                continue;

            if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == name && tempvarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, "val"});
                if (Token::getType(type) != newType)
                {
                    if (Token::getType(type) == "INTTYPE")
                    {
                        if (newType == "DECIMALTYPE")
                            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + tempVarName + " to double\n";
                        if (newType == "CHARTYPE")
                            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = trunc i32 %" + tempVarName + " to i8\n";
                        if (newType == "BOOLTYPE")
                            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = icmp ne i32 %" + tempVarName + ", 0\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
                    }
                    else if (Token::getType(type) == "DECIMALTYPE" && newType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + tempVarName + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
                    }
                    else if (Token::getType(type) == "CHARTYPE" && newType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i8 %" + tempVarName + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
                    }
                    else if (Token::getType(type) == "BOOLTYPE" && newType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i1 %" + tempVarName + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
                    }
                }
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        ir += "%" + tempVarName + " = load " + mapTypeIR(Token::getType(type)) + ", ptr @" + prefix + name + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({name, "val"});
        if (Token::getType(type) != newType)
        {
            if (Token::getType(type) == "INTTYPE")
            {
                if (newType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + tempVarName + " to double\n";
                if (newType == "CHARTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = trunc i32 %" + tempVarName + " to i8\n";
                if (newType == "BOOLTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = icmp ne i32 %" + tempVarName + ", 0\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "CHARTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i8 %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
            else if (Token::getType(type) == "BOOLTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i1 %" + tempVarName + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({name, Token::getType(type), newType});
            }
        }
    }

    loadedVars.push_back(name);
}
void IdentifierNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size());

    if (assignmentName == ".return")
    {
        if (Token::getType(type) != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";

        return;
    }

    if (global)
    {
        if (Token::getType(type) != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr @" + prefix + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if ((tempvarMap[i][j][0].substr(0, 7) == ".ifnode" || tempvarMap[i][j][0].substr(0, 11) == ".elseifnode" || tempvarMap[i][j][0].substr(0, 9) == ".elsenode" || tempvarMap[i][j][0].substr(0, 10) == ".whilenode"))
                outOfScope -= 1;
            if (tempvarMap[i][j][0] == ".endScope")
                outOfScope += 1;
            if (outOfScope > 0)
                continue;

            if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == assignmentName && tempvarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                if (Token::getType(type) != newType)
                    ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(newType) + "\n";
                else
                    ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        if (Token::getType(type) != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(Token::getType(type)) + " %" + tempVarName + ", ptr @" + prefix + assignmentName + ", align " + mapNumBytesIR(Token::getType(type)) + "\n";
    }
}
void IdentifierNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (imported)
        prefix = "." + prefixImports.at(modName) + ".";

    ir += prefix + name;
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
std::string BinaryExprNode::getOperandType() const
{
    return operandType;
}
std::string BinaryExprNode::getResultType() const
{
    return resultType;
}
void BinaryExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "BinaryExpressionNode: " << op << std::endl;
    std::cout << depthTabs << "Operand Type: " << getDataTypeStr(operandType) << std::endl;
    std::cout << depthTabs << "Result Type: " << getDataTypeStr(resultType) << std::endl;
    std::cout << depthTabs << "\tLeft:\n";
    left->print(depth + 2);
    std::cout << depthTabs << "\tRight:\n";
    right->print(depth + 2);
}
void BinaryExprNode::lookForCallExprs(Table &table)
{
    left->lookForCallExprs(table);
    right->lookForCallExprs(table);
}
bool BinaryExprNode::checkType(Table &table, const std::string &type)
{
    if ((op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=") && type != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }

    if (op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        std::string leftSide = left->inferType(table), rightSide = right->inferType(table);
        if (!((leftSide == "INTTYPE" || leftSide == "DECIMALTYPE") && (rightSide == "INTTYPE" || rightSide == "DECIMALTYPE")))
        {
            throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
            return false;
        }
    }

    if (op == "==" || op == "!=")
    {
        std::string leftSide = left->inferType(table), rightSide = right->inferType(table);
        if (leftSide == "BOOLTYPE" || rightSide == "BOOLTYPE")
        {
            if (leftSide != rightSide)
                throwSemanticError(line, "Can't compare these two types with '" + op + "' operator.");
        }
    }

    right->checkType(table, type);
    left->checkType(table, type);

    std::string leftType = left->inferType(table), rightType = right->inferType(table);
    if (op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=" || op == "==" || op == "!=")
    {
        if (leftType == "DECIMALTYPE" || rightType == "DECIMALTYPE")
            operandType = "DECIMALTYPE";
        else if (leftType == "INTTYPE" || rightType == "INTTYPE")
            operandType = "INTTYPE";
        else if (leftType == "CHARTYPE" || rightType == "CHARTYPE")
            operandType = "CHARTYPE";
        else
            operandType = "BOOLTYPE";
    }
    else
    {
        if (leftType == "DECIMALTYPE" || rightType == "DECIMALTYPE")
            this->operandType = "DECIMALTYPE";
        else
            this->operandType = "INTTYPE";
    }
    this->resultType = type;

    return true;
}
std::string BinaryExprNode::inferType(Table &table)
{
    std::string leftType = left->inferType(table), rightType = right->inferType(table);

    if (op == "and" || op == "or" || op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        return "BOOLTYPE";
    }

    if ((op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=") && (leftType != "BOOLTYPE" || rightType != "BOOLTYPE"))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }
    if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^") && ((leftType != "INTTYPE" && leftType != "DECIMALTYPE") || (rightType != "INTTYPE" && rightType != "DECIMALTYPE")))
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return "";
    }

    if (canCoerce(leftType, rightType))
    {
        if (leftType == "DECIMALTYPE" || rightType == "DECIMALTYPE")
            return "DECIMALTYPE";
        return "INTTYPE";
    }

    if (leftType != rightType)
    {
        throwSemanticError(line, "Mismatched types found with '" + op + "' operator.");
        return "";
    }

    return leftType;
}
void BinaryExprNode::loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, TemporaryVariables &tempVars, bool global, std::string prefix)
{
}
void BinaryExprNode::traverseBinary(std::string &ir, std::string &newType, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (dynamic_cast<BinaryExprNode *>(left) != nullptr || dynamic_cast<BinaryExprNode *>(right) != nullptr)
    {
        if (dynamic_cast<BinaryExprNode *>(left) != nullptr && dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            BinaryExprNode *leftIdentifier, *rightIdentifier;
            leftIdentifier = static_cast<BinaryExprNode *>(left);
            rightIdentifier = static_cast<BinaryExprNode *>(right);

            leftIdentifier->traverseBinary(ir, newType, tempVars, global, prefix);
            std::string l = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            if (leftIdentifier->getOperandType() != operandType)
            {
                if (leftIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                }
                else if (leftIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                }
                l = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }

            rightIdentifier->traverseBinary(ir, newType, tempVars, global, prefix);
            std::string r = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            if (rightIdentifier->getOperandType() != operandType)
            {
                if (rightIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                }
                else if (rightIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                }
                r = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }

            if (op == "/")
                checkForDivisionByZero("%" + r, operandType, ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 %" + l + ", i32 %" + r + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double %" + l + ", double %" + r + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + l + ", %" + r + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, std::to_string(tempvarMap[tempvarMap.size() - 1].size() - 1), std::to_string(tempvarMap.size())});
        }
        else if (dynamic_cast<BinaryExprNode *>(left) != nullptr)
        {
            BinaryExprNode *leftIdentifier;
            leftIdentifier = static_cast<BinaryExprNode *>(left);
            leftIdentifier->traverseBinary(ir, newType, tempVars, global, prefix);

            if (leftIdentifier->getOperandType() != operandType)
            {
                if (leftIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                }
                else if (leftIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                }
            }

            std::string l = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            std::string r;
            if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
            {
                rightIdentifier->traverseUnary(ir, newType, tempVars, global);
                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

                if (rightIdentifier->getOperandType() != operandType)
                {
                    if (rightIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    else if (rightIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                    r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
                }
            }
            else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
            {
                rightIdentifier->mapToIR(ir, tempVars, false);

                if (rightIdentifier->getReturnType() != operandType)
                {
                    if (rightIdentifier->getReturnType() == "INTTYPE" && operandType == "DECIMALTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
                    }
                    else if (rightIdentifier->getReturnType() == "DECIMALTYPE" && operandType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
                    }
                }

                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
            {
                std::vector<std::string> loadedVars;
                rightIdentifier->loadVariables(ir, operandType, loadedVars, tempVars, global, prefix);
                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else
            {
                right->mapToIR(r, tempVars);
                if (operandType == "DECIMALTYPE")
                    r = std::to_string(std::stod(r));
                if (operandType == "INTTYPE")
                    r = std::to_string(std::stoi(r));
            }

            if (op == "/")
                checkForDivisionByZero(r, operandType, ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 %" + l + ", i32 " + r + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double %" + l + ", double " + r + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + l + ", " + r + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, std::to_string(tempvarMap[tempvarMap.size() - 1].size()), r});
        }
        else if (dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            BinaryExprNode *rightIdentifier;
            rightIdentifier = static_cast<BinaryExprNode *>(right);
            rightIdentifier->traverseBinary(ir, newType, tempVars, global, prefix);

            if (rightIdentifier->getOperandType() != operandType)
            {
                if (rightIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                }
                else if (rightIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                }
            }

            std::string r = std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            std::string l;
            if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
            {
                leftIdentifier->traverseUnary(ir, newType, tempVars, global);
                l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

                if (leftIdentifier->getOperandType() != operandType)
                {
                    if (leftIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    else if (leftIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                    l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
                }
            }
            else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
            {
                leftIdentifier->mapToIR(ir, tempVars, false);

                if (leftIdentifier->getReturnType() != operandType)
                {
                    if (leftIdentifier->getReturnType() == "INTTYPE" && operandType == "DECIMALTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getReturnType(), operandType});
                    }
                    else if (leftIdentifier->getReturnType() == "DECIMALTYPE" && operandType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getReturnType(), operandType});
                    }
                }

                l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
            {
                std::vector<std::string> loadedVars;
                leftIdentifier->loadVariables(ir, operandType, loadedVars, tempVars, global, prefix);
                l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
            else
            {
                left->mapToIR(l, tempVars);
                if (operandType == "DECIMALTYPE")
                    l = std::to_string(std::stod(l));
                if (operandType == "INTTYPE")
                    l = std::to_string(std::stoi(l));
            }

            if (op == "/")
                checkForDivisionByZero("%" + r, operandType, ir);
            if (op == "^")
            {
                if (operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 " + l + ", i32 %" + r + ")\n";
                else if (operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double " + l + ", double %" + r + ")\n";
            }
            else
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", %" + r + "\n";

            tempvarMap[tempvarMap.size() - 1].push_back({op, l, r});
        }
    }
    else
    {
        std::string l, r;

        if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
        {
            leftIdentifier->traverseUnary(ir, newType, tempVars, global);
            l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

            if (leftIdentifier->getOperandType() != operandType)
            {
                if (leftIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                else if (leftIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getOperandType(), operandType});
                l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
        }
        else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
        {
            leftIdentifier->mapToIR(ir, tempVars, false);

            if (leftIdentifier->getReturnType() != operandType)
            {
                if (leftIdentifier->getReturnType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getReturnType(), operandType});
                }
                else if (leftIdentifier->getReturnType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), leftIdentifier->getReturnType(), operandType});
                }
            }

            l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
        {
            std::vector<std::string> loadedVars;
            leftIdentifier->loadVariables(ir, operandType, loadedVars, tempVars, global, prefix);
            l = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else
        {
            left->mapToIR(l, tempVars);
            if (operandType == "DECIMALTYPE")
                l = std::to_string(std::stod(l));
            if (operandType == "INTTYPE")
                l = std::to_string(std::stoi(l));
        }

        if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
        {
            rightIdentifier->traverseUnary(ir, newType, tempVars, global);
            r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

            if (rightIdentifier->getOperandType() != operandType)
            {
                if (rightIdentifier->getOperandType() == "INTTYPE" && operandType == "DECIMALTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                else if (rightIdentifier->getOperandType() == "DECIMALTYPE" && operandType == "INTTYPE")
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getOperandType(), operandType});
                r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
            }
        }
        else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
        {
            rightIdentifier->mapToIR(ir, tempVars, false);

            if (rightIdentifier->getReturnType() != operandType)
            {
                if (rightIdentifier->getReturnType() == "INTTYPE" && operandType == "DECIMALTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
                }
                else if (rightIdentifier->getReturnType() == "DECIMALTYPE" && operandType == "INTTYPE")
                {
                    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                    tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
                }
            }

            r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
        {
            std::vector<std::string> loadedVars;
            rightIdentifier->loadVariables(ir, operandType, loadedVars, tempVars, global, prefix);
            r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());
        }
        else
        {
            right->mapToIR(r, tempVars);
            if (operandType == "DECIMALTYPE")
                r = std::to_string(std::stod(r));
            if (operandType == "INTTYPE")
                r = std::to_string(std::stoi(r));
        }

        if (op == "/")
            checkForDivisionByZero(r, operandType, ir);
        if (op == "^")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call i32 @pow_int(i32 " + l + ", i32 " + r + ")\n";
            else if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + "call double @pow_double(double " + l + ", double " + r + ")\n";
        }
        else
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", " + r + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, l, r});
    }
}
void BinaryExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    traverseBinary(ir, newType, tempVars, global, prefix);

    if (assignmentName == ".return")
    {
        if (operandType != newType)
        {
            if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
            else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    if (global)
    {
        if (operandType != newType)
        {
            if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
            else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + prefix + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if ((tempvarMap[i][j][0].substr(0, 7) == ".ifnode" || tempvarMap[i][j][0].substr(0, 11) == ".elseifnode" || tempvarMap[i][j][0].substr(0, 9) == ".elsenode" || tempvarMap[i][j][0].substr(0, 10) == ".whilenode"))
                outOfScope -= 1;
            if (tempvarMap[i][j][0] == ".endScope")
                outOfScope += 1;
            if (outOfScope > 0)
                continue;

            if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == assignmentName && tempvarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                if (operandType != newType)
                {
                    if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
                    }
                    else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
                    }
                }

                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        if (operandType != newType)
        {
            if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
            else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + prefix + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
    }
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
std::string UnaryExprNode::getOperandType() const
{
    return operandType;
}
std::string UnaryExprNode::getResultType() const
{
    return resultType;
}
void UnaryExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "UnaryExpressionNode: " << op << std::endl;
    std::cout << depthTabs << "Operand Type: " << getDataTypeStr(operandType) << std::endl;
    std::cout << depthTabs << "Result Type: " << getDataTypeStr(resultType) << std::endl;
    std::cout << depthTabs << "\tRight:" << std::endl;
    right->print(depth + 2);
}
void UnaryExprNode::lookForCallExprs(Table &table)
{
    right->lookForCallExprs(table);
}
bool UnaryExprNode::checkType(Table &table, const std::string &type)
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

    right->checkType(table, type);
    this->operandType = right->inferType(table);
    this->resultType = type;

    return true;
}
std::string UnaryExprNode::inferType(Table &table)
{
    std::string rightType = right->inferType(table);
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

    if (op == "!" || op == "not")
        return "BOOLTYPE";
    return rightType;
}
void UnaryExprNode::traverseUnary(std::string &ir, std::string &newType, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (dynamic_cast<BinaryExprNode *>(right) != nullptr || dynamic_cast<UnaryExprNode *>(right) != nullptr)
    {
        if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
            rightIdentifier->traverseUnary(ir, newType, tempVars, global, prefix);
        if (BinaryExprNode *rightIdentifier = dynamic_cast<BinaryExprNode *>(right))
            rightIdentifier->traverseBinary(ir, newType, tempVars, global, prefix);

        if (op == "-")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sub " + mapTypeIR(operandType) + " 0, %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";
            if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fneg " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + "\n";
        }
        else if (op != "+")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = xor " + mapTypeIR(operandType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", true\n";
        }
        tempvarMap[tempvarMap.size() - 1].push_back({op, "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size())});
    }
    else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
    {
        rightIdentifier->mapToIR(ir, tempVars, false);

        if (rightIdentifier->getReturnType() != operandType)
        {
            if (rightIdentifier->getReturnType() == "INTTYPE" && operandType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
            }
            else if (rightIdentifier->getReturnType() == "DECIMALTYPE" && operandType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), rightIdentifier->getReturnType(), operandType});
            }
        }

        std::string r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

        if (op == "-")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
            if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
        }
        else if (op != "+")
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
    else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
    {
        std::vector<std::string> loadedVars;
        rightIdentifier->loadVariables(ir, operandType, loadedVars, tempVars, global, prefix);
        std::string r = "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size());

        if (op == "-")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
            if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
        }
        else if (op != "+")
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
    else
    {
        std::string r;
        right->mapToIR(r, tempVars);

        if (op == "-")
        {
            if (operandType == "INTTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
            if (operandType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
        }
        else if (op != "+")
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
        tempvarMap[tempvarMap.size() - 1].push_back({op, r});
    }
}
void UnaryExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    traverseUnary(ir, newType, tempVars, global, prefix);

    if (assignmentName == ".return")
    {
        if (operandType != newType)
        {
            if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
            else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if ((tempvarMap[i][j][0].substr(0, 7) == ".ifnode" || tempvarMap[i][j][0].substr(0, 11) == ".elseifnode" || tempvarMap[i][j][0].substr(0, 9) == ".elsenode" || tempvarMap[i][j][0].substr(0, 10) == ".whilenode"))
                outOfScope -= 1;
            if (tempvarMap[i][j][0] == ".endScope")
                outOfScope += 1;
            if (outOfScope > 0)
                continue;

            if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == assignmentName && tempvarMap[i][j][1] == "ptr")
            {
                globalVar = false;

                if (operandType != newType)
                {
                    if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
                    }
                    else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
                    {
                        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                        tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
                    }
                }

                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        if (operandType != newType)
        {
            if (operandType == "INTTYPE" && newType == "DECIMALTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
            else if (operandType == "DECIMALTYPE" && newType == "INTTYPE")
            {
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
                tempvarMap[tempvarMap.size() - 1].push_back({assignmentName, operandType, newType});
            }
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", ptr @" + prefix + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
    }
}

ParamNode::ParamNode(std::string identifier, int line)
{
    reference = false;
    this->line = line;
    std::string refStr, typeStr, nameStr;
    for (int i = identifier.size() - 1; i > -1; i--)
    {
        if (identifier[i] == ' ')
        {
            name = identifier.substr(i + 1);

            std::string type = identifier.substr(0, i);
            if (identifier.substr(0, 3) == "ref")
            {
                type = identifier.substr(4, i - 4);
                reference = true;
            }

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
ParamNode::ParamNode(std::string name, std::string type, int line)
{
    reference = false;
    this->name = name;
    this->line = line;

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
ParamNode::~ParamNode()
{
}
void ParamNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
}
std::string ParamNode::getName() const
{
    return name;
}
TokenType ParamNode::getType() const
{
    return type;
}
bool ParamNode::isReference() const
{
    return reference;
}
void ParamNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "ParameterNode: " << getDataTypeStr(Token::getType(type)) << " " << name << (reference ? ", Reference" : "") << std::endl;
}

FuncNode::FuncNode(std::string name, std::string returnType, std::vector<ParamNode *> parameters, BlockNode *body, int line)
{
    this->name = name;
    this->parameters = parameters;
    this->body = body;
    this->line = line;

    emptyFunction = false;
    if (body->getNumStatements() == 0)
        emptyFunction = true;

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

    if (!emptyFunction)
    {
        std::vector<Node *> returnStmts;
        findReturnStmts(returnStmts);

        if (returnStmts.size() != 0)
        {
            for (int i = 0; i < returnStmts.size(); i++)
            {
                ReturnStmtNode *returnStmt = static_cast<ReturnStmtNode *>(returnStmts[i]);
                returnStmt->setType(Token::getType(this->returnType));
            }
        }
        else
        {
            if (Token::getType(this->returnType) != "UNKNOWN")
                throwSemanticError(line, "Return Statement not found in function.");
        }
    }
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
    if (!emptyFunction)
        body->print(depth + 1);
}
void FuncNode::addSignatureToTable(Table &table, bool includeParams)
{
    std::vector<std::string> parameterTypes;
    for (ParamNode *parameter : parameters)
    {
        if (parameter->isReference())
            parameterTypes.push_back("REF" + Token::getType(parameter->getType()));
        else
            parameterTypes.push_back(Token::getType(parameter->getType()));
    }

    table.addSymbol(table.getVisibleScope()[0], new FuncSymbol(line, false, name, Token::getType(returnType), false, parameterTypes));
    if (includeParams)
    {
        table.addVisibleScope();
        std::string scope = table.getLatestVisibleScope();
        for (ParamNode *parameter : parameters)
            table.addSymbol(scope, new VarSymbol(parameter->getLine(), false, parameter->getName(), Token::getType(parameter->getType())));
    }
}
void FuncNode::checkSemantics(Table &table)
{
    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
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

            if (symbol->getType() == "VAR" && symbol->getName() == name)
                throwSemanticError(line, "'" + name + "' has already been declared as a variable.");
        }
    }

    addSignatureToTable(table);

    if (!emptyFunction)
        body->checkSemantics(table);

    table.removeScope();
    table.removeVisibleScope();
}
void FuncNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string callName = name;
    if (name == "main")
        callName = "__main__";

    std::string funcName = prefix + callName;
    for (int i = 0; i < parameters.size(); i++)
    {
        if (parameters[i]->isReference())
            funcName += ".REF" + Token::getType(parameters[i]->getType());
        else
            funcName += "." + Token::getType(parameters[i]->getType());
    }

    ir += "define " + mapTypeIR(Token::getType(returnType)) + " @" + funcName + "(";
    tempvarMap.push_back({});

    for (int i = 0; i < parameters.size(); i++)
    {
        if (parameters[i]->isReference())
        {
            if (i == parameters.size() - 1)
                ir += "ptr %" + std::to_string(i + 1);
            else
                ir += "ptr %" + std::to_string(i + 1) + ", ";
        }
        else
        {
            if (i == parameters.size() - 1)
                ir += mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName();
            else
                ir += mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName() + ", ";
        }
    }

    ir += ") {\nentry:\n";
    for (int i = 0; i < parameters.size(); i++)
    {
        if (!parameters[i]->isReference())
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = alloca " + mapTypeIR(Token::getType(parameters[i]->getType())) + ", align " + mapNumBytesIR(Token::getType(parameters[i]->getType())) + "\n";
            ir += "store " + mapTypeIR(Token::getType(parameters[i]->getType())) + " %" + parameters[i]->getName() + ", ptr %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + ", align " + mapNumBytesIR(Token::getType(parameters[i]->getType())) + "\n";
        }
        tempvarMap[tempvarMap.size() - 1]
            .push_back({parameters[i]->getName(), "ptr"});
    }

    if (!emptyFunction)
    {
        for (Node *node : body->getStatements())
            node->mapToIR(ir, tempVars, false, prefix);
    }

    if (Token::getType(returnType) == "UNKNOWN")
        ir += "ret void\n";
    else
    {
        if (emptyFunction)
            throwSemanticError(line, "An Empty Function can't be declared with a return type.");
    }

    if (ir[ir.size() - 2] == ':')
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    if (Token::getType(returnType) != "UNKNOWN" && !dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]))
        throwSemanticError(line, "Code Path without a Return Value Detected.");

    ir += "}\n\n";

    tempvarMap.pop_back();
}
void FuncNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

CallExprNode::CallExprNode(std::string name, std::vector<Node *> arguments)
{
    this->name = name;
    this->arguments = arguments;
    imported = false;
    modName = "";
}
CallExprNode::~CallExprNode()
{
    for (Node *arg : arguments)
    {
        delete arg;
        arg = nullptr;
    }
}
std::string CallExprNode::getName() const
{
    return name;
}
std::string CallExprNode::getReturnType() const
{
    return returnType;
}
std::string CallExprNode::getModName() const
{
    return modName;
}
void CallExprNode::setImported(std::string modName)
{
    imported = true;
    this->modName = modName;
}
void CallExprNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    if (imported)
    {
        std::cout << depthTabs << "Imported CallExpressionNode: " << name << std::endl;
        std::cout << depthTabs << "\tFrom Module: " + modName << std::endl;
    }
    else
        std::cout << depthTabs << "CallExpressionNode: " << name << std::endl;

    std::cout << depthTabs << "\tArguments: " << std::endl;
    for (int i = 0; i < arguments.size(); i++)
        arguments[i]->print(depth + 2);
}
void CallExprNode::lookForCallExprs(Table &table)
{
    if (name == "toChar" || name == "toBool" || name == "toInt" || name == "toDecimal")
        return;

    argTypes = {};

    if (imported)
    {
        auto scope = table.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                if (returnType == "UNKNOWN" || symbol->getDataType() == returnType || canCoerce(symbol->getDataType(), returnType))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(table, derivedSymbol->getParameters()[k]))
                            break;
                    }
                }
            }
        }

        return;
    }

    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                if (returnType == "UNKNOWN" || symbol->getDataType() == returnType || canCoerce(symbol->getDataType(), returnType))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        if (!arguments[k]->checkType(table, derivedSymbol->getParameters()[k]))
                        {
                            wrongType = true;
                            break;
                        }
                    }

                    if (!wrongType)
                        symbol->setUsed(true);
                }
            }
        }
    }
}
bool CallExprNode::checkType(Table &table, const std::string &type)
{
    argTypes = {};

    if (name == "toChar" || name == "toBool" || name == "toInt" || name == "toDecimal")
    {
        if (arguments.size() == 1)
        {
            std::string argType = arguments[0]->inferType(table);
            if (name == "toChar")
                if (argType == "INTTYPE")
                    returnType = "CHARTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toBool")
                if (argType == "INTTYPE")
                    returnType = "BOOLTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toInt")
                if (argType == "DECIMALTYPE" || argType == "CHARTYPE" || argType == "BOOLTYPE")
                    returnType = "INTTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");
            if (name == "toDecimal")
                if (argType == "INTTYPE")
                    returnType = "DECIMALTYPE";
                else
                    throwSemanticError(line, "'" + name + "' can't be used with the type " + getDataTypeStr(argType) + ".");

            argTypes.push_back(argType);
            arguments[0]->checkType(table, argType);
            return true;
        }
        else
            throwSemanticError(line, "'" + name + "' has the wrong arguments passed in.");
    }

    bool found = true;
    if (name == "print" || name == "input" || name == "exit")
    {
        this->returnType = "UNKNOWN";

        for (int i = 0; i < Token::builtInFunctions.size(); i++)
        {
            found = true;
            if (Token::builtInFunctions[i][0] != name)
                continue;
            else
            {
                if (arguments.size() != Token::builtInFunctions[i].size() - 1)
                    continue;

                for (int j = 1; j < Token::builtInFunctions[i].size(); j++)
                {
                    std::string currentType = arguments[j - 1]->inferType(table);
                    if ((Token::builtInFunctions[i][j] == "var" || Token::builtInFunctions[i][j] == currentType))
                    {
                        argTypes.push_back(currentType);
                        continue;
                    }
                    else
                        found = false;
                }

                for (Node *arg : arguments)
                    arg->checkType(table, arg->inferType(table));

                if (found)
                    return true;
            }
        }
    }

    if (name == "print" || name == "input" || name == "exit")
        throwSemanticError(line, "function '" + name + "' has the wrong arguments passed in.");

    bool overridenFunction = false;
    if (imported)
    {
        auto scope = table.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                overridenFunction = true;
                if (type == "" || symbol->getDataType() == type || canCoerce(symbol->getDataType(), type))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        std::string p = derivedSymbol->getParameters()[k];
                        if (p.substr(0, 3) == "REF")
                        {
                            p = p.substr(3);
                            if (!dynamic_cast<IdentifierNode *>(arguments[k]))
                                throwSemanticError(line, "Must use an identifier (variable) for a reference.");

                            if (!arguments[k]->checkType(table, p))
                            {
                                wrongType = true;
                                break;
                            }
                        }

                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        arguments[k]->checkType(table, derivedSymbol->getParameters()[k]);
                        if (!canCoerce(arguments[k]->inferType(table), p))
                        {
                            if (!arguments[k]->checkType(table, p))
                            {
                                wrongType = true;
                                break;
                            }
                        }
                    }

                    if (wrongType)
                        continue;
                    else
                    {
                        this->returnType = symbol->getDataType();
                        return true;
                    }
                }
                else
                    return false;
            }
        }

        if (overridenFunction)
            throwSemanticError(line, "Coulnd't find proper function signature for '" + name + "' call expression in Module '" + modName + "'.");

        throwSemanticError(line, "function '" + name + "' was not found in Module '" + modName + "'.");
        return false;
    }

    overridenFunction = false;
    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                overridenFunction = true;
                if (type == "" || symbol->getDataType() == type || canCoerce(symbol->getDataType(), type))
                {
                    FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                    if (arguments.size() != derivedSymbol->getNumParameters())
                        continue;

                    bool wrongType = false;
                    for (int k = 0; k < derivedSymbol->getParameters().size(); k++)
                    {
                        std::string p = derivedSymbol->getParameters()[k];
                        if (p.substr(0, 3) == "REF")
                        {
                            p = p.substr(3);
                            if (!dynamic_cast<IdentifierNode *>(arguments[k]))
                                throwSemanticError(line, "Must use an identifier (variable) for a reference.");

                            if (!arguments[k]->checkType(table, p))
                            {
                                wrongType = true;
                                break;
                            }
                        }

                        this->argTypes.push_back(derivedSymbol->getParameters()[k]);
                        arguments[k]->checkType(table, derivedSymbol->getParameters()[k]);
                        if (!canCoerce(arguments[k]->inferType(table), p))
                        {
                            if (!arguments[k]->checkType(table, p))
                            {
                                wrongType = true;
                                break;
                            }
                        }
                    }

                    if (wrongType)
                        continue;
                    else
                    {
                        this->returnType = symbol->getDataType();
                        symbol->setUsed(true);
                        return true;
                    }
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
void CallExprNode::checkSemantics(Table &table)
{
    checkType(table, "");
}

std::string CallExprNode::inferType(Table &table)
{
    if (name == "print" || name == "input" || name == "exit")
        return "UNKNOWN";

    if (name == "toChar")
        return "CHARTYPE";
    if (name == "toBool")
        return "BOOLTYPE";
    if (name == "toInt")
        return "INTTYPE";
    if (name == "toDecimal")
        return "DECIMALTYPE";

    if (imported)
    {
        auto scope = table.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name && symbol->isExported())
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
            }
        }

        throwSemanticError(line, "function '" + name + "' was not found in Module '" + modName + "'.");
        return "";
    }

    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC" && symbol->getName() == name)
            {
                returnType = symbol->getDataType();
                return symbol->getDataType();
            }
        }
    }

    throwSemanticError(line, "function '" + name + "' hasn't been declared.");
    return "";
}
void CallExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    mapToIR(ir, tempVars, global, prefix);

    if (imported)
        prefix = "." + prefixImports.at(modName) + ".";

    std::string tempVarName = std::to_string(tempvarMap[tempvarMap.size() - 1].size());

    if (returnType != newType)
    {
        if (returnType == "INTTYPE")
        {
            if (newType == "DECIMALTYPE")
                ir += "%" + tempVarName + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
            if (newType == "CHARTYPE")
                ir += "%" + tempVarName + " = trunc i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i8\n";
            if (newType == "BOOLTYPE")
                ir += "%" + tempVarName + " = icmp ne i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", 0\n";
            tempvarMap[tempvarMap.size() - 1].push_back({name, returnType, newType});
        }
        else if (returnType == "DECIMALTYPE" && newType == "INTTYPE")
            ir += "%" + tempVarName + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
        else if (returnType == "CHARTYPE" && newType == "INTTYPE")
            ir += "%" + tempVarName + " = zext i8 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
        else if (returnType == "BOOLTYPE" && newType == "INTTYPE")
            ir += "%" + tempVarName + " = zext i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
        tempvarMap[tempvarMap.size() - 1].push_back({name, returnType, newType});
    }

    if (assignmentName == ".return")
    {
        if (returnType != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";
        return;
    }

    if (global)
    {
        if (returnType != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    for (int i = tempvarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
        {
            if ((tempvarMap[i][j][0].substr(0, 7) == ".ifnode" || tempvarMap[i][j][0].substr(0, 11) == ".elseifnode" || tempvarMap[i][j][0].substr(0, 9) == ".elsenode" || tempvarMap[i][j][0].substr(0, 10) == ".whilenode"))
                outOfScope -= 1;
            if (tempvarMap[i][j][0] == ".endScope")
                outOfScope += 1;
            if (outOfScope > 0)
                continue;

            if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == assignmentName && tempvarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                if (returnType != newType)
                    ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(newType) + "\n";
                else
                    ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(returnType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        if (returnType != newType)
            ir += "store " + mapTypeIR(newType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(newType) + "\n";
        else
            ir += "store " + mapTypeIR(returnType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(returnType) + "\n";
    }

    tempvarMap[tempvarMap.size() - 1].push_back({name, "ptr"});
}
void CallExprNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (imported)
        prefix = "." + prefixImports.at(modName) + ".";

    std::string callName = prefix + name;
    for (int i = 0; i < argTypes.size(); i++)
        callName += "." + argTypes[i];

    if (name == "print")
    {
        if (argTypes[0] == "CHARTYPE")
            callName = "print_char";
        else if (argTypes[0] == "DECIMALTYPE")
            callName = "print_double";
        else
            callName = "print_int";

        if (argTypes.size() > 1)
            callName += "_end";
    }
    else if (name == "input")
        callName = name;
    else if (name == "exit")
        callName = "myst_exit";
    else if (name == "main")
        callName = "__main__";

    if (name == "toInt" || name == "toDecimal" || name == "toChar" || name == "toBool")
        callName = name;

    std::vector<std::string> args;
    for (int i = 0; i < arguments.size(); i++)
    {
        bool isRef = argTypes[i].substr(0, 3) == "REF";
        if (!isRef)
        {
            std::string callArg = ".callarg_" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
            std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
            ir += "%" + loadTemp + " = alloca " + mapTypeIR(argTypes[i]) + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
            tempvarMap[tempvarMap.size() - 1].push_back({callArg, "ptr"});

            std::vector<std::string> loadedVars;
            arguments[i]->loadVariables(ir, argTypes[i], loadedVars, tempVars, global, prefix);
            arguments[i]->storeValue(ir, argTypes[i], callArg, tempVars, false, prefix);

            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load " + mapTypeIR(argTypes[i]) + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(argTypes[i]) + "\n";
            tempvarMap[tempvarMap.size() - 1].push_back({callArg, "ptr"});
            args.push_back("%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()));
        }
        else
        {
            IdentifierNode *identifier = static_cast<IdentifierNode *>(arguments[i]);
            if (global)
                args.push_back("@" + identifier->getName());
            else
            {
                bool globalVar = true;
                for (int i = tempvarMap.size() - 1; i > 0; i--)
                {
                    for (int j = tempvarMap[i].size() - 1; j >= 0; j--)
                    {
                        if (tempvarMap[i][j].size() == 2 && tempvarMap[i][j][0] == identifier->getName() && tempvarMap[i][j][1] == "ptr")
                        {
                            globalVar = false;
                            args.push_back("%" + std::to_string(j + 1));
                            break;
                        }
                    }

                    if (!globalVar)
                        break;
                }

                if (globalVar)
                    args.push_back("@" + identifier->getName());
            }
        }
    }

    if (name == "toInt" || name == "toDecimal" || name == "toChar" || name == "toBool")
    {
        if (argTypes[0] == "INTTYPE")
        {
            if (returnType == "DECIMALTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = sitofp i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to double\n";
            if (returnType == "CHARTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = trunc i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i8\n";
            if (returnType == "BOOLTYPE")
                ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = icmp ne i32 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", 0\n";
            tempvarMap[tempvarMap.size() - 1].push_back({name, argTypes[0], returnType});
        }
        else if (argTypes[0] == "DECIMALTYPE" && returnType == "INTTYPE")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = fptosi double %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
            tempvarMap[tempvarMap.size() - 1].push_back({name, argTypes[0], returnType});
        }
        else if (argTypes[0] == "CHARTYPE" && returnType == "INTTYPE")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i8 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
            tempvarMap[tempvarMap.size() - 1].push_back({name, argTypes[0], returnType});
        }
        else if (argTypes[0] == "BOOLTYPE" && returnType == "INTTYPE")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = zext i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + " to i32\n";
            tempvarMap[tempvarMap.size() - 1].push_back({name, argTypes[0], returnType});
        }
    }
    else
    {
        if (returnType != "UNKNOWN")
        {
            ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = call " + mapTypeIR(returnType) + " @" + callName + "(";
            tempvarMap[tempvarMap.size() - 1].push_back({name, "ptr"});
        }
        else
            ir += "call " + mapTypeIR(returnType) + " @" + callName + "(";

        for (int i = 0; i < args.size(); i++)
        {
            if (argTypes[i].substr(0, 3) == "REF")
            {
                if (i == args.size() - 1)
                    ir += "ptr " + args[i];
                else
                    ir += "ptr " + args[i] + ", ";
            }
            else
            {
                if (i == args.size() - 1)
                    ir += mapTypeIR(argTypes[i]) + " " + args[i];
                else
                    ir += mapTypeIR(argTypes[i]) + " " + args[i] + ", ";
            }
        }
        ir += ")\n";
    }
}

AssignNode::AssignNode(std::string name, std::string type, Node *value)
{
    declared = true;
    if (type == "")
        declared = false;
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
Node *AssignNode::getValue() const
{
    return value;
}
bool AssignNode::isDeclared() const
{
    return declared;
}
void AssignNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "AssignmentNode: " << std::endl;
    std::cout << depthTabs << "\tLeft Side: " << getDataTypeStr(Token::getType(identifier->getType())) << " " << identifier->getName() << std::endl;
    std::cout << depthTabs << "\tRight Side: " << std::endl;
    value->print(depth + 2);
}
void AssignNode::lookForCallExprs(Table &table)
{
    value->lookForCallExprs(table);
}
void AssignNode::checkSemantics(Table &table)
{
    if (identifier->getType() != TokenType::UNKNOWN)
    {
        auto scope = table.getScope(table.getLatestVisibleScope());
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName() && !(table.getVisibleScope().size() > 1))
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared.");

            if (symbol->getType() == "FUNC" && symbol->getName() == identifier->getName())
                throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a function.");
        }

        if (identifier->getType() == TokenType::VAR)
        {
            std::string t = value->inferType(table);
            identifier->setType(Token::getTokenDataType(t));
            value->checkType(table, Token::getType(identifier->getType()));
            table.addSymbol(table.getLatestVisibleScope(), new VarSymbol(line, false, identifier->getName(), t));
        }
        else if (value->checkType(table, Token::getType(identifier->getType())) || canCoerce(value->inferType(table), Token::getType(identifier->getType())))
            table.addSymbol(table.getLatestVisibleScope(), new VarSymbol(line, false, identifier->getName(), Token::getType(identifier->getType())));
        else
            throwSemanticError(line, "Value in '" + identifier->getName() + "' must have type '" + getDataTypeStr(Token::getType(identifier->getType())) + "'.");
    }
    else
    {
        for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
        {
            auto scope = table.getScope(table.getVisibleScope()[i]);
            for (Symbol *symbol : scope.getSymbols())
            {
                if (symbol->getType() == "VAR" && symbol->getName() == identifier->getName())
                {
                    if (value->checkType(table, symbol->getDataType()) || canCoerce(symbol->getDataType(), value->inferType(table)))
                    {
                        identifier->setType(Token::getTokenDataType(symbol->getDataType()));
                        return;
                    }
                    else
                        throwSemanticError(line, "'" + identifier->getName() + "' has already been declared with type '" + getDataTypeStr(symbol->getDataType()) + "'.");
                }

                if (symbol->getType() == "FUNC" && symbol->getName() == identifier->getName())
                    throwSemanticError(line, "'" + identifier->getName() + "' has already been declared as a function.");
            }
        }

        throwSemanticError(line, "'" + identifier->getName() + "' hasn't been declared.");
    }
}
void AssignNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    if (!global && declared)
    {
        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = alloca " + mapTypeIR(Token::getType(identifier->getType())) + ", align " + mapNumBytesIR(Token::getType(identifier->getType())) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({identifier->getName(), "ptr"});
    }

    std::vector<std::string> loadedVars;
    std::string newType = Token::getType(identifier->getType());
    value->loadVariables(ir, newType, loadedVars, tempVars, global, prefix);
    value->storeValue(ir, newType, identifier->getName(), tempVars, global, prefix);
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
void BlockNode::checkSemantics(Table &table)
{
    for (Node *statement : statements)
        statement->checkSemantics(table);
}
void BlockNode::mapToGlobalIR(std::string &ir, std::string prefix)
{
    for (Node *node : statements)
    {
        if (AssignNode *assignment = dynamic_cast<AssignNode *>(node))
        {
            if (!assignment->isDeclared())
                continue;
            if (assignment->getType() == "CHARTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i8 0, align 1\n";
            if (assignment->getType() == "INTTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i32 0, align 4\n";
            if (assignment->getType() == "DECIMALTYPE")
                ir += "@" + prefix + assignment->getName() + " = global double 0.0, align 8\n";
            if (assignment->getType() == "BOOLTYPE")
                ir += "@" + prefix + assignment->getName() + " = global i1 0, align 1\n";
        }
    }

    ir += "\n";
}
void BlockNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    ir += "define i32 @" + prefix + "main() {\nentry:\n";

    for (Node *node : statements)
        node->mapToIR(ir, tempVars, true, prefix);

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::mapImportsToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::vector<std::string> imports)
{
    ir += "define i32 @main() {\nentry:\n";

    for (const std::string &import : imports)
    {
        ir += "call i32 @." + import + ".main()\n";
    }

    for (Node *node : statements)
        node->mapToIR(ir, tempVars, true, "");

    ir += "ret i32 0\n";
    ir += "}\n\n";
}
void BlockNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < statements.size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(statements[i]))
        {
            if (i != statements.size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < statements.size(); i++)
    {
        statements[i]->findReturnStmts(stmts);
    }
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
void IfStmtNode::setCurrentWhile(int currentStmt)
{
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
void IfStmtNode::checkSemantics(Table &table)
{
    if (!condition->checkType(table, "BOOLTYPE"))
        throwSemanticError(line, "If Statement condition doesn't evaluate to a boolean.");

    table.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(table);

    table.removeScope();
    table.removeVisibleScope();
}
bool IfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void IfStmtNode::mapCondition(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".ifcondition_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, tempVars, global, prefix);
    condition->storeValue(ir, newType, ".ifcondition_" + loadTemp, tempVars, false, prefix);
    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".ifnode_" + loadTemp, "ptr"});
}
void IfStmtNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempVars, global, prefix);

    tempvarMap[tempvarMap.size() - 1].push_back({".endScope", "ptr"});
}
void IfStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
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
void ElseIfStmtNode::setCurrentWhile(int currentStmt)
{
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseIfStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseIfStmtNode::mapCondition(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string currentCount = std::to_string(BranchBlockNode::getNumberIf());
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".elseifcondition_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, tempVars, global, prefix);
    condition->storeValue(ir, newType, ".elseifcondition_" + loadTemp, tempVars, false, prefix);
    ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".elseifnode_" + loadTemp, "ptr"});
}
void ElseIfStmtNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempVars, global, prefix);
}
void ElseIfStmtNode::checkSemantics(Table &table)
{
    if (!condition->checkType(table, "BOOLTYPE"))
        throwSemanticError(line, "Else If Statement condition doesn't evaluate to a boolean.");

    table.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(table);

    table.removeVisibleScope();
    table.removeScope();
}
void ElseIfStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
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
void ElseStmtNode::setCurrentWhile(int currentStmt)
{
    for (Node *stmt : body->getStatements())
    {
        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(currentStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }

        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(currentStmt);
    }
}
bool ElseStmtNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void ElseStmtNode::checkSemantics(Table &table)
{
    table.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(table);

    table.removeScope();
    table.removeVisibleScope(0);
}
void ElseStmtNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    tempvarMap[tempvarMap.size() - 1].push_back({".elsenode_" + loadTemp, "ptr"});

    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempVars, global, prefix);
}
void ElseStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
}

int BranchBlockNode::numIfStmt = 0;
BranchBlockNode::BranchBlockNode()
{
    branches = new BlockNode();
}

BranchBlockNode::~BranchBlockNode()
{
    delete branches;
    branches = nullptr;
}

void BranchBlockNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "BranchBlockNode:" << std::endl;
    branches->print(depth + 1);
}
int BranchBlockNode::getNumberIf()
{
    return numIfStmt;
}
void BranchBlockNode::addBranch(Node *branch)
{
    branches->addStatement(branch);
}
void BranchBlockNode::setCurrentWhile(int currentWhile)
{
    for (Node *stmt : branches->getStatements())
    {
        if (dynamic_cast<IfStmtNode *>(stmt) != nullptr)
        {
            IfStmtNode *ifStmt = static_cast<IfStmtNode *>(stmt);
            ifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast<ElseIfStmtNode *>(stmt) != nullptr)
        {
            ElseIfStmtNode *elseifStmt = static_cast<ElseIfStmtNode *>(stmt);
            elseifStmt->setCurrentWhile(currentWhile);
        }

        if (dynamic_cast<ElseStmtNode *>(stmt) != nullptr)
        {
            ElseStmtNode *elseStmt = static_cast<ElseStmtNode *>(stmt);
            elseStmt->setCurrentWhile(currentWhile);
        }
    }
}
void BranchBlockNode::checkSemantics(Table &table)
{
    for (Node *node : branches->getStatements())
        node->checkSemantics(table);
}
void BranchBlockNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    BranchBlockNode::numIfStmt++;
    int numElseIfStmt = 1;
    std::string currentIf = std::to_string(numIfStmt);
    std::string currentElseIf = std::to_string(numElseIfStmt);
    std::string nextElseIf = std::to_string(numElseIfStmt + 1);
    for (int i = 0; i < branches->getNumStatements(); i++)
    {
        Node *node = branches->getStatements()[i];

        if (dynamic_cast<IfStmtNode *>(node) != nullptr)
        {
            IfStmtNode *ifStmt = static_cast<IfStmtNode *>(node);
            ifStmt->mapCondition(ir, tempVars, global, prefix);

            if (branches->getNumStatements() == 1)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() == 2 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %if_true_" + currentIf + ", label %if_next_" + currentIf + "_" + currentElseIf + "\n";

            ir += "if_true_" + currentIf + ":\n";
            ifStmt->mapToIR(ir, tempVars, global, prefix);

            if (!ifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }

        if (dynamic_cast<ElseIfStmtNode *>(node) != nullptr)
        {
            ir += "if_next_" + currentIf + "_" + currentElseIf + ":\n";
            ElseIfStmtNode *elseifStmt = static_cast<ElseIfStmtNode *>(node);
            elseifStmt->mapCondition(ir, tempVars, false, prefix);

            if (i == branches->getNumStatements() - 1)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_end_" + currentIf + "\n";
            else if (branches->getNumStatements() >= 3 && dynamic_cast<ElseStmtNode *>(branches->getStatements()[i + 1]) != nullptr)
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %else_" + currentIf + "\n";
            else
                ir += "br i1 %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size()) + ", label %elseif_true_" + currentIf + "_" + currentElseIf + ", label %if_next_" + currentIf + "_" + nextElseIf + "\n";

            ir += "elseif_true_" + currentIf + "_" + currentElseIf + ":\n";
            elseifStmt->mapToIR(ir, tempVars, false, prefix);
            if (!elseifStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";

            numElseIfStmt++;
            currentElseIf = std::to_string(numElseIfStmt);
            nextElseIf = std::to_string(numElseIfStmt + 1);
        }

        if (dynamic_cast<ElseStmtNode *>(node) != nullptr)
        {
            ElseStmtNode *elseStmt = static_cast<ElseStmtNode *>(node);
            ir += "else_" + currentIf + ":\n";
            elseStmt->mapToIR(ir, tempVars, false, prefix);
            if (!elseStmt->endsWithBranchStmt())
                ir += "br label %if_end_" + currentIf + "\n";
        }
    }

    ir += "if_end_" + currentIf + ":\n";
}
void BranchBlockNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < branches->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(branches->getStatements()[i]))
        {
            if (i != branches->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < branches->getStatements().size(); i++)
    {
        branches->getStatements()[i]->findReturnStmts(stmts);
    }
}

int WhileNode::numWhileStmt = 0;
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
bool WhileNode::getSkip()
{
    return skip;
}

void WhileNode::setSkip(bool skip)
{
    this->skip = skip;
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

bool WhileNode::endsWithBranchStmt()
{
    return (dynamic_cast<ReturnStmtNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<BreakNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr) || (dynamic_cast<SkipNode *>(body->getStatements()[body->getNumStatements() - 1]) != nullptr);
}
void WhileNode::checkSemantics(Table &table)
{
    if (!condition->checkType(table, "BOOLTYPE"))
        throwSemanticError(line, "While loop condition doesn't evaluate to a boolean.");

    table.addVisibleScope();
    for (Node *stmtNode : body->getStatements())
        stmtNode->checkSemantics(table);

    table.removeScope();
    table.removeVisibleScope();
}
void WhileNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    numWhileStmt++;
    std::string currentCount = std::to_string(WhileNode::numWhileStmt);
    std::vector<std::string> loadedVars;
    std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);

    ir += "br label %while_cond_" + currentCount + "\n";
    ir += "while_cond_" + currentCount + ":\n";
    ir += "%" + loadTemp + " = alloca i1, align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".whilenode_" + loadTemp, "ptr"});

    std::string newType = "BOOLTYPE";
    condition->loadVariables(ir, newType, loadedVars, tempVars, global, prefix);
    condition->storeValue(ir, newType, ".whilenode_" + loadTemp, tempVars, false, prefix);

    std::string condTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
    ir += "%" + condTemp + " = load i1" + ", ptr %" + loadTemp + ", align 1" + "\n";
    tempvarMap[tempvarMap.size() - 1].push_back({".whilenode_" + loadTemp, "ptr"});
    ir += "br i1 %" + condTemp + ", label %while_true_" + currentCount + ", label %while_end_" + currentCount + "\n";
    ir += "while_true_" + currentCount + ":\n";

    for (Node *stmt : body->getStatements())
    {
        if (BranchBlockNode *branchBlock = dynamic_cast<BranchBlockNode *>(stmt))
            branchBlock->setCurrentWhile(numWhileStmt);

        if (BreakNode *breakStmt = dynamic_cast<BreakNode *>(stmt))
        {
            breakStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'break' statement.");
        }

        if (SkipNode *skipStmt = dynamic_cast<SkipNode *>(stmt))
        {
            skipStmt->setCurrentWhile(numWhileStmt);

            if (body->getStatements()[body->getNumStatements() - 1] != stmt)
                throwSemanticError(stmt->getLine(), "Unreachable code detected after 'skip' statement.");
        }
    }

    for (Node *stmt : body->getStatements())
        stmt->mapToIR(ir, tempVars, global, prefix);

    if (!endsWithBranchStmt())
        ir += "br label %while_cond_" + currentCount + "\n";

    ir += "while_end_" + currentCount + ":\n";
}
void WhileNode::findReturnStmts(std::vector<Node *> &stmts)
{
    for (int i = 0; i < body->getStatements().size(); i++)
    {
        if (ReturnStmtNode *returnStmt = dynamic_cast<ReturnStmtNode *>(body->getStatements()[i]))
        {
            if (i != body->getStatements().size() - 1)
                throwSemanticError(returnStmt->getLine(), "Unreachable code detected after return statement.");
        }
    }

    for (int i = 0; i < body->getStatements().size(); i++)
    {
        body->getStatements()[i]->findReturnStmts(stmts);
    }
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
void ReturnStmtNode::setType(std::string type)
{
    this->returnType = type;
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
void ReturnStmtNode::checkSemantics(Table &table)
{
    for (int i = table.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = table.getScope(table.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "FUNC")
            {
                if (identifier == nullptr && returnType != "UNKNOWN")
                    throwSemanticError(line, "Return statement found without an identifier.");

                if (returnType == "UNKNOWN" && identifier != nullptr)
                    throwSemanticError(line, "Return statement found in a function that doesn't have a return type.");

                if (identifier != nullptr && !identifier->checkType(table, returnType) && !canCoerce(identifier->inferType(table), returnType))
                    throwSemanticError(line, "Return statement doesn't return the required return type.");

                FuncSymbol *derivedSymbol = static_cast<FuncSymbol *>(symbol);
                derivedSymbol->setHasReturnFlag(true);
            }
        }
    }
}
void ReturnStmtNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    std::string type = mapTypeIR(returnType);
    std::vector<std::string> loadedVars;

    if (identifier == nullptr)
        ir += "ret void\n";
    else
    {
        std::string loadTemp = std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1);
        ir += "%" + loadTemp + " = alloca " + mapTypeIR(returnType) + ", align " + mapNumBytesIR(returnType) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({".return_" + loadTemp, "ptr"});

        identifier->loadVariables(ir, returnType, loadedVars, tempVars, false, prefix);
        identifier->storeValue(ir, returnType, ".return_" + loadTemp, tempVars, false, prefix);

        ir += "%" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + " = load " + type + ", ptr %" + loadTemp + ", align " + mapNumBytesIR(returnType) + "\n";
        ir += "ret " + type + " %" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1) + "\n";
        tempvarMap[tempvarMap.size() - 1].push_back({".return_" + std::to_string(tempvarMap[tempvarMap.size() - 1].size() + 1), "ptr"});
    }
}
void ReturnStmtNode::findReturnStmts(std::vector<Node *> &stmts)
{
    stmts.push_back(this);
}

BreakNode::BreakNode(int line)
{
    this->line = line;
    currentWhileStmt = 0;
}
BreakNode::~BreakNode()
{
}
void BreakNode::setCurrentWhile(int currentStmt)
{
    currentWhileStmt = currentStmt;
}
void BreakNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Break Statement\n";
}
void BreakNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    ir += "br label %while_end_" + std::to_string(currentWhileStmt) + "\n";
}

SkipNode::SkipNode(int line)
{
    this->line = line;
    currentWhileStmt = 0;
}
SkipNode::~SkipNode()
{
}
void SkipNode::setCurrentWhile(int currentStmt)
{
    currentWhileStmt = currentStmt;
}
void SkipNode::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "Skip Statement\n";
}
void SkipNode::mapToIR(std::string &ir, TemporaryVariables &tempVars, bool global, std::string prefix)
{
    ir += "br label %while_cond_" + std::to_string(currentWhileStmt) + "\n";
}
