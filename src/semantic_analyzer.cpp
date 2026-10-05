#include "compiler/semantic_analyzer.h"

void SemanticAnalyzer::analyzeTree(RootNode *root)
{
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> scope;
    std::vector<std::string> visibleScope;

    root->checkSemantics(scope, visibleScope);
    root->sendWarnings(scope);
}
