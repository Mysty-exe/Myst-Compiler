#include "compiler/node.h"

int DIVBYZERO = 0;

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
void BinaryExprNode::lookForCallExprs(SemanticsInfo &semanticsInfo)
{
    left->lookForCallExprs(semanticsInfo);
    right->lookForCallExprs(semanticsInfo);
}
bool BinaryExprNode::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
{
    if ((op == "and" || op == "or" || op == "&&" || op == "||" || op == ">" || op == "<" || op == ">=" || op == "<=") && type != "BOOLTYPE")
    {
        throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
        return false;
    }

    if (op == ">" || op == "<" || op == ">=" || op == "<=")
    {
        std::string leftSide = left->inferType(semanticsInfo), rightSide = right->inferType(semanticsInfo);
        if (!((leftSide == "INTTYPE" || leftSide == "DECIMALTYPE") && (rightSide == "INTTYPE" || rightSide == "DECIMALTYPE")))
        {
            throwSemanticError(line, "Unexpected type/s used with operator '" + op + "'.");
            return false;
        }
    }

    if (op == "==" || op == "!=")
    {
        std::string leftSide = left->inferType(semanticsInfo), rightSide = right->inferType(semanticsInfo);
        if (leftSide == "BOOLTYPE" || rightSide == "BOOLTYPE")
        {
            if (leftSide != rightSide)
                throwSemanticError(line, "Can't compare these two types with '" + op + "' operator.");
        }
    }

    right->checkType(semanticsInfo, type);
    left->checkType(semanticsInfo, type);

    std::string leftType = left->inferType(semanticsInfo), rightType = right->inferType(semanticsInfo);
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
std::string BinaryExprNode::inferType(SemanticsInfo &semanticsInfo)
{
    std::string leftType = left->inferType(semanticsInfo), rightType = right->inferType(semanticsInfo);

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
void BinaryExprNode::traverseBinaryBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    BinaryExprNode *leftIdentifier, *rightIdentifier;
    leftIdentifier = static_cast<BinaryExprNode *>(left);
    rightIdentifier = static_cast<BinaryExprNode *>(right);

    leftIdentifier->traverseBinary(ir, newType, llvmInfo, global, prefix);
    std::string l = std::to_string(llvmInfo.getCurrentTempVar());
    if (leftIdentifier->getOperandType() != operandType)
    {
        coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getOperandType(), operandType, true);
        l = std::to_string(llvmInfo.getCurrentTempVar());
    }

    rightIdentifier->traverseBinary(ir, newType, llvmInfo, global, prefix);
    std::string r = std::to_string(llvmInfo.getCurrentTempVar());
    if (rightIdentifier->getOperandType() != operandType)
    {
        coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getOperandType(), operandType, true);
        r = std::to_string(llvmInfo.getCurrentTempVar());
    }

    if (op == "/")
        checkForDivisionByZero("%" + r, operandType, ir);
    if (op == "^")
    {
        if (operandType == "INTTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call i32 @pow_int(i32 %" + l + ", i32 %" + r + ")\n";
        else if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call double @pow_double(double %" + l + ", double %" + r + ")\n";
    }
    else
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + l + ", %" + r + "\n";

    llvmInfo.addTempVar({op, std::to_string(llvmInfo.getPreviousTempVar()), std::to_string(llvmInfo.getCurrentTempVar())});
}
void BinaryExprNode::traverseBinaryExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    BinaryExprNode *leftIdentifier;
    leftIdentifier = static_cast<BinaryExprNode *>(left);
    leftIdentifier->traverseBinary(ir, newType, llvmInfo, global, prefix);

    if (leftIdentifier->getOperandType() != operandType)
    {
        coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getOperandType(), operandType, true);
    }

    std::string l = std::to_string(llvmInfo.getCurrentTempVar());
    std::string r;
    if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
    {
        rightIdentifier->traverseUnary(ir, newType, llvmInfo, global);
        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());

        if (rightIdentifier->getOperandType() != operandType)
        {
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getOperandType(), operandType, true);
            r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
        }
    }
    else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
    {
        rightIdentifier->mapToIR(ir, llvmInfo, false);

        if (rightIdentifier->getReturnType() != operandType)
        {
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getReturnType(), operandType, true);
        }

        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
    {
        std::vector<std::string> loadedVars;
        rightIdentifier->loadVariables(ir, operandType, loadedVars, llvmInfo, global, prefix);
        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else
    {
        right->mapToIR(r, llvmInfo);
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
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call i32 @pow_int(i32 %" + l + ", i32 " + r + ")\n";
        else if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call double @pow_double(double %" + l + ", double " + r + ")\n";
    }
    else
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " %" + l + ", " + r + "\n";

    llvmInfo.addTempVar({op, std::to_string(llvmInfo.getCurrentTempVar()), r});
}
void BinaryExprNode::traverseExprBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    BinaryExprNode *rightIdentifier;
    rightIdentifier = static_cast<BinaryExprNode *>(right);
    rightIdentifier->traverseBinary(ir, newType, llvmInfo, global, prefix);

    if (rightIdentifier->getOperandType() != operandType)
    {
        coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getOperandType(), operandType, true);
    }

    std::string r = std::to_string(llvmInfo.getCurrentTempVar());
    std::string l;
    if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
    {
        leftIdentifier->traverseUnary(ir, newType, llvmInfo, global);

        if (leftIdentifier->getOperandType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getOperandType(), operandType, true);

        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
    {
        leftIdentifier->mapToIR(ir, llvmInfo, false);

        if (leftIdentifier->getReturnType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getReturnType(), operandType, true);

        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
    {
        std::vector<std::string> loadedVars;
        leftIdentifier->loadVariables(ir, operandType, loadedVars, llvmInfo, global, prefix);
        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else
    {
        left->mapToIR(l, llvmInfo);
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
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call i32 @pow_int(i32 " + l + ", i32 %" + r + ")\n";
        else if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call double @pow_double(double " + l + ", double %" + r + ")\n";
    }
    else
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", %" + r + "\n";

    llvmInfo.addTempVar({op, l, r});
}
void BinaryExprNode::traverseExrpExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string l, r;

    if (UnaryExprNode *leftIdentifier = dynamic_cast<UnaryExprNode *>(left))
    {
        leftIdentifier->traverseUnary(ir, newType, llvmInfo, global);

        if (leftIdentifier->getOperandType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getOperandType(), operandType, true);

        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (CallExprNode *leftIdentifier = dynamic_cast<CallExprNode *>(left))
    {
        leftIdentifier->mapToIR(ir, llvmInfo, false);

        if (leftIdentifier->getReturnType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), leftIdentifier->getReturnType(), operandType, true);

        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (IdentifierNode *leftIdentifier = dynamic_cast<IdentifierNode *>(left))
    {
        std::vector<std::string> loadedVars;
        leftIdentifier->loadVariables(ir, operandType, loadedVars, llvmInfo, global, prefix);
        l = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else
    {
        left->mapToIR(l, llvmInfo);
        if (operandType == "DECIMALTYPE")
            l = std::to_string(std::stod(l));
        if (operandType == "INTTYPE")
            l = std::to_string(std::stoi(l));
    }

    if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
    {
        rightIdentifier->traverseUnary(ir, newType, llvmInfo, global);

        if (rightIdentifier->getOperandType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getOperandType(), operandType, true);

        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (CallExprNode *rightIdentifier = dynamic_cast<CallExprNode *>(right))
    {
        rightIdentifier->mapToIR(ir, llvmInfo, false);

        if (rightIdentifier->getReturnType() != operandType)
            coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getReturnType(), operandType, true);

        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else if (IdentifierNode *rightIdentifier = dynamic_cast<IdentifierNode *>(right))
    {
        std::vector<std::string> loadedVars;
        rightIdentifier->loadVariables(ir, operandType, loadedVars, llvmInfo, global, prefix);

        r = "%" + std::to_string(llvmInfo.getCurrentTempVar());
    }
    else
    {
        right->mapToIR(r, llvmInfo);
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
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call i32 @pow_int(i32 " + l + ", i32 " + r + ")\n";
        else if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + "call double @pow_double(double " + l + ", double " + r + ")\n";
    }
    else
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = " + mapOperatorIR(op, operandType) + " " + mapTypeIR(operandType) + " " + l + ", " + r + "\n";
    llvmInfo.addTempVar({op, l, r});
}
void BinaryExprNode::traverseBinary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (dynamic_cast<BinaryExprNode *>(left) != nullptr || dynamic_cast<BinaryExprNode *>(right) != nullptr)
    {
        if (dynamic_cast<BinaryExprNode *>(left) != nullptr && dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            traverseBinaryBinary(ir, newType, llvmInfo, global, prefix);
        }
        else if (dynamic_cast<BinaryExprNode *>(left) != nullptr)
        {
            traverseBinaryExpr(ir, newType, llvmInfo, global, prefix);
        }
        else if (dynamic_cast<BinaryExprNode *>(right) != nullptr)
        {
            traverseExprBinary(ir, newType, llvmInfo, global, prefix);
        }
    }
    else
    {
        traverseExrpExpr(ir, newType, llvmInfo, global, prefix);
    }
}
void BinaryExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    traverseBinary(ir, newType, llvmInfo, global, prefix);

    if (assignmentName == ".return")
    {
        if (operandType != newType)
        {
            coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
        return;
    }

    if (global)
    {
        if (operandType != newType)
        {
            coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
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
                if (operandType != newType)
                {
                    coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);
                }

                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
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
            coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);
        }

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
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
void UnaryExprNode::lookForCallExprs(SemanticsInfo &semanticsInfo)
{
    right->lookForCallExprs(semanticsInfo);
}
bool UnaryExprNode::checkType(SemanticsInfo &semanticsInfo, const std::string &type)
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

    right->checkType(semanticsInfo, type);
    this->operandType = right->inferType(semanticsInfo);
    this->resultType = type;

    return true;
}
std::string UnaryExprNode::inferType(SemanticsInfo &semanticsInfo)
{
    std::string rightType = right->inferType(semanticsInfo);
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
void UnaryExprNode::traverseBinaryOrUnary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (UnaryExprNode *rightIdentifier = dynamic_cast<UnaryExprNode *>(right))
        rightIdentifier->traverseUnary(ir, newType, llvmInfo, global, prefix);
    if (BinaryExprNode *rightIdentifier = dynamic_cast<BinaryExprNode *>(right))
        rightIdentifier->traverseBinary(ir, newType, llvmInfo, global, prefix);

    if (op == "-")
    {
        if (operandType == "INTTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = sub " + mapTypeIR(operandType) + " 0, %" + std::to_string(llvmInfo.getCurrentTempVar()) + "\n";
        if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = fneg " + mapTypeIR(operandType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + "\n";
    }
    else if (op != "+")
    {
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = xor " + mapTypeIR(operandType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", true\n";
    }
    llvmInfo.addTempVar({op, "%" + std::to_string(llvmInfo.getCurrentTempVar())});
}
void UnaryExprNode::traverseCallExpr(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    CallExprNode *rightIdentifier = static_cast<CallExprNode *>(right);
    rightIdentifier->mapToIR(ir, llvmInfo, false);

    if (rightIdentifier->getReturnType() != operandType)
        coerceValue(ir, llvmInfo, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), rightIdentifier->getReturnType(), operandType, true);

    std::string r = "%" + std::to_string(llvmInfo.getCurrentTempVar());

    if (op == "-")
    {
        if (operandType == "INTTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
        if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
    }
    else if (op != "+")
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
    llvmInfo.addTempVar({op, r});
}
void UnaryExprNode::traverseIdentifier(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    IdentifierNode *rightIdentifier = static_cast<IdentifierNode *>(right);
    std::vector<std::string> loadedVars;
    rightIdentifier->loadVariables(ir, operandType, loadedVars, llvmInfo, global, prefix);
    std::string r = "%" + std::to_string(llvmInfo.getCurrentTempVar());

    if (op == "-")
    {
        if (operandType == "INTTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
        if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
    }
    else if (op != "+")
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
    llvmInfo.addTempVar({op, r});
}
void UnaryExprNode::traverseLiteral(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    std::string r;
    right->mapToIR(r, llvmInfo);

    if (op == "-")
    {
        if (operandType == "INTTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = sub " + mapTypeIR(operandType) + " 0, " + r + "\n";
        if (operandType == "DECIMALTYPE")
            ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = fneg " + mapTypeIR(operandType) + " " + r + "\n";
    }
    else if (op != "+")
        ir += "%" + std::to_string(llvmInfo.getNextTempVar()) + " = xor " + mapTypeIR(operandType) + " " + r + ", true\n";
    llvmInfo.addTempVar({op, r});
}
void UnaryExprNode::traverseUnary(std::string &ir, std::string &newType, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    if (dynamic_cast<BinaryExprNode *>(right) != nullptr || dynamic_cast<UnaryExprNode *>(right) != nullptr)
    {
        traverseBinaryOrUnary(ir, newType, llvmInfo, global, prefix);
    }
    else if (dynamic_cast<CallExprNode *>(right))
    {
        traverseCallExpr(ir, newType, llvmInfo, global, prefix);
    }
    else if (dynamic_cast<IdentifierNode *>(right))
    {
        traverseIdentifier(ir, newType, llvmInfo, global, prefix);
    }
    else
    {
        traverseLiteral(ir, newType, llvmInfo, global, prefix);
    }
}
void UnaryExprNode::storeValue(std::string &ir, std::string &newType, const std::string &assignmentName, LLVMInfo &llvmInfo, bool global, std::string prefix)
{
    traverseUnary(ir, newType, llvmInfo, global, prefix);

    if (assignmentName == ".return")
    {
        if (operandType != newType)
            coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr %" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
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

                if (operandType != newType)
                    coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);

                ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr %" + std::to_string(j + 1) + ", align " + mapNumBytesIR(resultType) + "\n";
                break;
            }
        }

        if (!globalVar)
            break;
    }

    if (globalVar)
    {
        if (operandType != newType)
            coerceValue(ir, llvmInfo, assignmentName, std::to_string(llvmInfo.getNextTempVar()), std::to_string(llvmInfo.getCurrentTempVar()), operandType, newType, true);

        ir += "store " + mapTypeIR(resultType) + " %" + std::to_string(llvmInfo.getCurrentTempVar()) + ", ptr @" + assignmentName + ", align " + mapNumBytesIR(resultType) + "\n";
    }
}
