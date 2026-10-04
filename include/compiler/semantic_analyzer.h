#pragma once
#include <iostream>
#include "lexer.h"
#include "ast.h"

class SemanticAnalyzer
{
public:
    static void analyzeTree(RootNode *rootNode);
};
