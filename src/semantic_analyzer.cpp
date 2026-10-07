#include "compiler/semantic_analyzer.h"

void SemanticAnalyzer::analyzeTree(RootNode *root)
{
    std::unordered_map<std::string, Scope> symbolTable;
    std::vector<std::string> visibleScope;

    root->checkSemantics(symbolTable, visibleScope);
    root->sendWarnings(symbolTable);
}
