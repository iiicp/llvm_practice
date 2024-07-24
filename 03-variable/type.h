#pragma once

enum class TypeKind {
    Int
};

class CType {
private:
    TypeKind kind;
    int size;       /// 字节数
    int align;      /// 对齐数
public:
    CType(TypeKind kind, int size, int align):kind(kind), size(size), align(align) {}

    static CType *GetIntTy();
};