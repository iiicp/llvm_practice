#include "scope.h"

Scope::Scope() {
    envs.push_back(std::make_shared<Env>());
}
void Scope::EnterScope() {
    envs.push_back(std::make_shared<Env>());
}
void Scope::ExitScope() {
    envs.pop_back();
}
std::shared_ptr<Symbol> Scope::FindVarSymbol(llvm::StringRef name) {
    for (auto it = envs.rbegin(); it != envs.rend(); ++it) {
        auto &table = (*it)->variableSymbolTable;
        if (table.count(name) > 0) {
            return table[name];
        }
    }
    return nullptr;
}
std::shared_ptr<Symbol> Scope::FindVarSymbolInCurEnv(llvm::StringRef name) {
    auto &table = envs.back()->variableSymbolTable;
    if (table.count(name) > 0) {
        return table[name];
    }
    return nullptr;
}
void Scope::AddSymbol(SymbolKind kind, CType *ty, llvm::StringRef name) {
    auto symbol = std::make_shared<Symbol>(kind, ty, name);
    envs.back()->variableSymbolTable.insert({name, symbol});
}