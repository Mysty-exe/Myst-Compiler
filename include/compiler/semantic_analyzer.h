#pragma once
#include <iostream>
#include "lexer.h"
#include "ast.h"

class SemanticAnalyzer
{
private:
public:
    static void analyzeTree(Node *rootNode);
};
