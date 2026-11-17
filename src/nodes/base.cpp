#include "compiler/node.h"

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

    return str;
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

bool canCoerceLossless(std::string origType, std::string newType)
{
    if (origType == "DECIMALTYPE" && newType == "INTTYPE")
        return true;

    return false;
}

bool canCoerceLossy(std::string origType, std::string newType)
{
    if (origType == "INTTYPE" && newType == "DECIMALTYPE")
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

bool checkTempVarInScope(std::string currentTempVar, int &outOfScope)
{
    if (currentTempVar.substr(0, 7) == ".ifnode" || currentTempVar.substr(0, 11) == ".elseifnode" || currentTempVar.substr(0, 9) == ".elsenode" || currentTempVar.substr(0, 10) == ".whilenode")
        outOfScope -= 1;
    if (currentTempVar.substr(0, 9) == ".endScope")
        outOfScope += 1;
    if (outOfScope > 0)
        return false;

    return true;
}

void coerceValue(std::string &ir, LLVMInfo &llvmInfo, std::string name, std::string newTempName, std::string oldTempName, std::string origType, std::string newType, bool isExpression)
{
    if (origType == "INTTYPE")
    {
        if (newType == "DECIMALTYPE")
        {
            ir += "%" + newTempName + " = sitofp i32 %" + oldTempName + " to double\n";
            llvmInfo.addTempVar({name, origType, newType});
        }
        if (newType == "CHARTYPE" && !isExpression)
        {
            ir += "%" + newTempName + " = trunc i32 %" + oldTempName + " to i8\n";
            llvmInfo.addTempVar({name, origType, newType});
        }
        if (newType == "BOOLTYPE" && !isExpression)
        {
            ir += "%" + newTempName + " = icmp ne i32 %" + oldTempName + ", 0\n";
            llvmInfo.addTempVar({name, origType, newType});
        }
    }
    else if (origType == "DECIMALTYPE" && newType == "INTTYPE")
    {
        ir += "%" + newTempName + " = fptosi double %" + oldTempName + " to i32\n";
        llvmInfo.addTempVar({name, origType, newType});
    }
    else if (origType == "CHARTYPE" && newType == "INTTYPE" && !isExpression)
    {
        ir += "%" + newTempName + " = zext i8 %" + oldTempName + " to i32\n";
        llvmInfo.addTempVar({name, origType, newType});
    }
    else if (origType == "BOOLTYPE" && newType == "INTTYPE" && !isExpression)
    {
        ir += "%" + newTempName + " = zext i1 %" + oldTempName + " to i32\n";
        llvmInfo.addTempVar({name, origType, newType});
    }
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

void Node::lookForCallExprs(SemanticsInfo &semanticsInfo)
{
}

bool Node::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
{
    return true;
}
void Node::checkSemantics(SemanticsInfo &semanticsInfo)
{
}
std::string Node::inferType(SemanticsInfo &semanticsInfo)
{
    return "";
}
void Node::loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
}
void Node::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
}
void Node::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
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
        else if (fileName[i] != ':')
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
void RootNode::checkSemantics(SemanticsInfo &semanticsInfo)
{
    semanticsInfo.addVisibleScope();

    std::vector<AssignNode *> assignments;
    for (Node *child : children)
    {
        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->addSignatureToTable(semanticsInfo, false);

        if (ContainerNode *container = dynamic_cast<ContainerNode *>(child))
            container->checkSemantics(semanticsInfo);

        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (AssignNode *assignment = dynamic_cast<AssignNode *>(stmt))
                {
                    assignment->checkSemantics(semanticsInfo);
                    assignments.push_back(assignment);
                }
            }
        }
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        auto symbols = scope.getSymbols();
        auto it = symbols.begin();

        while (it != symbols.end())
        {
            if (dynamic_cast<FuncSymbol *>(*it) || dynamic_cast<ContainerSymbol *>(*it))
                it = symbols.erase(it);
            else
                ++it;
        }

        scope.setSymbols(symbols);
        semanticsInfo.setScope(semanticsInfo.getVisibleScope()[i], scope);
    }

    for (Node *child : children)
    {
        if (BlockNode *block = dynamic_cast<BlockNode *>(child))
        {
            for (Node *stmt : block->getStatements())
            {
                if (dynamic_cast<AssignNode *>(stmt) == nullptr)
                    stmt->checkSemantics(semanticsInfo);
            }
        }

        if (ContainerNode *container = dynamic_cast<ContainerNode *>(child))
            container->checkSemantics(semanticsInfo);

        if (FuncNode *func = dynamic_cast<FuncNode *>(child))
            func->checkSemantics(semanticsInfo);
    }

    for (AssignNode *assign : assignments)
        assign->lookForCallExprs(semanticsInfo);
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
                else if (symbol->getType() == "FUNC")
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
                if (!derivedSymbol->hasReturn() && getDataTypeStr(derivedSymbol->getDataType()) != "")
                    throwSemanticWarning(derivedSymbol->getLine(), "Function '" + symbol->getName() + "' returns '" + getDataTypeStr(derivedSymbol->getDataType()) + "' but return statement not found.");
            }
        }
    }
}

template <typename T>
LiteralNode<T>::LiteralNode(T value, std::string type)
{
    this->value = value;
    this->type = type;
}

template <typename T>
LiteralNode<T>::~LiteralNode()
{
}

template <typename T>
void LiteralNode<T>::print(int depth) const
{
    std::string depthTabs = repeat_string("\t", depth);
    std::cout << depthTabs << "LiteralNode: " << value << std::endl;
}

template <typename T>
bool LiteralNode<T>::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
{
    if (this->type == "CHAR" && type == "CHARTYPE")
        return true;
    if (this->type == "STRING" && type == "STRINGTYPE")
        return true;
    if (this->type == "INT" && type == "INTTYPE")
        return true;
    if (this->type == "DECIMAL" && type == "DECIMALTYPE")
        return true;
    if (this->type == "BOOL" && type == "BOOLTYPE")
        return true;

    return false;
}

template <typename T>
std::string LiteralNode<T>::inferType(SemanticsInfo &semanticsInfo)
{
    if constexpr (std::is_same_v<T, int>)
        return "INTTYPE";
    if constexpr (std::is_same_v<T, double>)
        return "DECIMALTYPE";
    if constexpr (std::is_same_v<T, char>)
        return "CHARTYPE";
    if constexpr (std::is_same_v<T, bool>)
        return "BOOLTYPE";

    return "";
}

template <typename T>
void LiteralNode<T>::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string localOrGlobal;
    std::string name = assignmentName;

    if (global)
        localOrGlobal = "@";
    else
    {
        bool globalVar = true;
        auto tempVarMap = llvmInfo.getTempVars();
        for (int i = tempVarMap.size() - 1; i >= 0; i--)
        {
            for (int j = tempVarMap[i].size() - 1; j >= 0; j--)
            {
                if (tempVarMap[i][j].size() == 2 && tempVarMap[i][j][0] == assignmentName && tempVarMap[i][j][1] == "ptr")
                {
                    globalVar = false;
                    localOrGlobal = "%";
                    name = std::to_string(j + 1);
                    break;
                }
            }

            if (!globalVar)
                break;
        }

        if (globalVar)
            localOrGlobal = "@";
    }

    if (name == ".return")
        localOrGlobal = "%";

    if constexpr (std::is_same_v<T, int>)
    {
        if (newType == "DECIMALTYPE")
            ir += "store double " + std::to_string(static_cast<double>(value)) + ", ptr " + localOrGlobal + name + ", align 8\n";
        else if (newType == "CHARTYPE")
            ir += "store i8 " + std::to_string(static_cast<char>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
        else if (newType == "BOOLTYPE")
            ir += "store i1 " + std::to_string(static_cast<bool>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
        else
            ir += "store i32 " + std::to_string(value) + ", ptr " + localOrGlobal + name + ", align 4\n";
    }
    if constexpr (std::is_same_v<T, double>)
    {
        if (newType == "INTTYPE")
            ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
        else
            ir += "store double " + std::to_string(value) + ", ptr " + localOrGlobal + name + ", align 8\n";
    }
    if constexpr (std::is_same_v<T, char>)
        if (newType == "INTTYPE")
            ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
        else
            ir += "store i8 " + std::to_string(static_cast<char>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
    if constexpr (std::is_same_v<T, bool>)
        if (newType == "INTTYPE")
            ir += "store i32 " + std::to_string(static_cast<int>(value)) + ", ptr " + localOrGlobal + name + ", align 4\n";
        else
            ir += "store i1 " + std::to_string(static_cast<bool>(value)) + ", ptr " + localOrGlobal + name + ", align 1\n";
}

template <typename T>
void LiteralNode<T>::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if constexpr (std::is_same_v<T, int>)
        ir += std::to_string(value);
    if constexpr (std::is_same_v<T, double>)
        ir += std::to_string(value);
    if constexpr (std::is_same_v<T, char>)
        ir += std::to_string(static_cast<int>(value));
    if constexpr (std::is_same_v<T, bool>)
        ir += std::to_string(static_cast<int>(value));
}

template class LiteralNode<bool>;
template class LiteralNode<char>;
template class LiteralNode<int>;
template class LiteralNode<float>;
template class LiteralNode<double>;
template class LiteralNode<std::string>;

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
    this->type = "";
    imported = false;
    modName = "";
}
IdentifierNode::IdentifierNode(std::string name, std::string type)
{
    this->name = name;
    imported = false;
    modName = "";

    if (type == "char")
        this->type = "CHARTYPE";
    else if (type == "int")
        this->type = "INTTYPE";
    else if (type == "decimal")
        this->type = "DECIMALTYPE";
    else if (type == "bool")
        this->type = "BOOLTYPE";
    else if (type == "var")
        this->type = "VAR";
    else
        this->type = type;
}
IdentifierNode::~IdentifierNode()
{
}
std::string IdentifierNode::getName() const
{
    return name;
}
std::string IdentifierNode::getType() const
{
    return type;
}
void IdentifierNode::setName(std::string name)
{
    this->name = name;
}
void IdentifierNode::setType(std::string type)
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
        std::cout << depthTabs << "Imported IdentifierNode: " << getDataTypeStr(type) << " " << name << std::endl;
        std::cout << depthTabs << "\tFrom Module: " + modName << std::endl;
    }
    else
        std::cout << depthTabs << "IdentifierNode: " << getDataTypeStr(type) << " " << name << std::endl;
}
bool IdentifierNode::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
{
    if (imported)
    {
        if (semanticsInfo.getPrefixSymbols().count(modName) == 0)
            throwSemanticError(line, "Module '" + modName + "' was not found.");

        auto scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name && symbol->isExported())
            {
                if (symbol->getDataType() == type)
                    this->type = symbol->getDataType();

                return (symbol->getDataType() == type);
            }
        }

        throwSemanticError(line, "'" + name + "' was not found in Module '" + modName + "'.");
        return false;
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
                if (symbol->getDataType() == type)
                    this->type = symbol->getDataType();

                return (symbol->getDataType() == type);
            }
        }
    }

    throwSemanticError(line, "'" + name + "' hasn't been declared.");
    return false;
}
std::string IdentifierNode::inferType(SemanticsInfo &semanticsInfo)
{
    if (imported)
    {
        if (semanticsInfo.getPrefixSymbols().count(modName) == 0)
            throwSemanticError(line, "Module '" + modName + "' was not found.");

        auto scope = semanticsInfo.getPrefixSymbols().at(modName).at("1");
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name && symbol->isExported())
            {
                this->type = symbol->getDataType();
                return symbol->getDataType();
            }
        }

        throwSemanticError(line, "'" + name + "' was not found in Module '" + modName + "'.");
        return "";
    }

    for (int i = semanticsInfo.getVisibleScope().size() - 1; i >= 0; i--)
    {
        auto scope = semanticsInfo.getScope(semanticsInfo.getVisibleScope()[i]);
        for (Symbol *symbol : scope.getSymbols())
        {
            if (symbol->getType() == "VAR" && symbol->getName() == name)
            {
                symbol->setUsed(true);
                this->type = symbol->getDataType();
                return symbol->getDataType();
            }
        }
    }

    throwSemanticError(line, "'" + name + "' hasn't been declared.");
    return "";
}
void IdentifierNode::loadVariables(std::string &ir, std::string &newType, std::vector<std::string> &loadedVars, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (std::find(loadedVars.begin(), loadedVars.end(), name) != loadedVars.end())
        return;

    if (imported)
        prefix = "." + llvmInfo.getImportName(modName) + ".";

    std::string tempVarName = std::to_string(llvmInfo.getNextTempVar());
    if (global)
    {
        ir += "%" + tempVarName + " = load " + mapTypeIR(type) + ", ptr @" + prefix + name + ", align " + mapNumBytesIR(type) + "\n";
        llvmInfo.addTempVar({name, "val"});
        if (type != newType)
        {
            coerceValue(ir, llvmInfo, name, std::to_string(llvmInfo.getNextTempVar()), tempVarName, type, newType);
        }
        loadedVars.push_back(name);
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    auto tempVarMap = llvmInfo.getTempVars();
    for (int i = tempVarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempVarMap[i].size() - 1; j >= 0; j--)
        {
            bool inScope = checkTempVarInScope(tempVarMap[i][j][0], outOfScope);
            if (!inScope)
                continue;

            if (tempVarMap[i][j].size() == 2 && tempVarMap[i][j][0] == name && tempVarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                ir += "%" + tempVarName + " = load " + mapTypeIR(type) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(type) + "\n";
                llvmInfo.addTempVar({name, "val"});
                if (type != newType)
                {
                    coerceValue(ir, llvmInfo, name, std::to_string(llvmInfo.getNextTempVar()), tempVarName, type, newType);
                }
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        ir += "%" + tempVarName + " = load " + mapTypeIR(type) + ", ptr @" + prefix + name + ", align " + mapNumBytesIR(type) + "\n";
        llvmInfo.addTempVar({name, "val"});
        if (type != newType)
        {
            coerceValue(ir, llvmInfo, name, std::to_string(llvmInfo.getNextTempVar()), tempVarName, type, newType);
        }
    }

    loadedVars.push_back(name);
}
void IdentifierNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string tempVarName = std::to_string(llvmInfo.getCurrentTempVar());
    std::string llvmType = (type != newType) ? newType : type;

    if (assignmentName == ".return")
    {
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
        return;
    }

    if (global)
    {
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
        return;
    }

    bool globalVar = true;
    int outOfScope = 0;
    auto tempVarMap = llvmInfo.getTempVars();
    for (int i = tempVarMap.size() - 1; i >= 0; i--)
    {
        for (int j = tempVarMap[i].size() - 1; j >= 0; j--)
        {
            bool inScope = checkTempVarInScope(tempVarMap[i][j][0], outOfScope);
            if (!inScope)
                continue;

            if (tempVarMap[i][j].size() == 2 && tempVarMap[i][j][0] == assignmentName && tempVarMap[i][j][1] == "ptr")
            {
                globalVar = false;
                ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(llvmType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
        ir += "store " + mapTypeIR(llvmType) + " %" + tempVarName + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(llvmType) + "\n";
}
void IdentifierNode::mapToIR(std::string &ir, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (imported)
        prefix = "." + llvmInfo.getImportName(modName) + ".";

    ir += prefix + name;
}
