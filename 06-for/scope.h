#pragma once
#include "llvm/ADT/StringMap.h"
#include "type.h"
#include <memory>

enum class SymbolKind {
    LocalVariable
};

class Symbol {
private:
    SymbolKind kind;
    CType *ty;
    llvm::StringRef name;
public:
    Symbol(SymbolKind kind, CType *ty, llvm::StringRef name):kind(kind), ty(ty), name(name) {}
    CType *GetTy() {return ty;}
};


class Env{
public:
    /// 符号信息
    llvm::StringMap<std::shared_ptr<Symbol>> variableSymbolTable;
};

class Scope {
private:
    std::vector<std::shared_ptr<Env>> envs;
public:
    Scope();
    void EnterScope();
    void ExitScope();
    std::shared_ptr<Symbol> FindVarSymbol(llvm::StringRef name);
    std::shared_ptr<Symbol> FindVarSymbolInCurEnv(llvm::StringRef name);
    void AddSymbol(SymbolKind kind, CType *ty, llvm::StringRef name);
};