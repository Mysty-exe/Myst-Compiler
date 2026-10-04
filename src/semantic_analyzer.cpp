#include "compiler/semantic_analyzer.h"

void SemanticAnalyzer::analyzeTree(RootNode *root)
{
    std::unordered_map<std::string, std::vector<std::string>> scope;

    root->checkSemantics(scope, {"global"});
}
