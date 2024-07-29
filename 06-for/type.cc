#include "type.h"

CType *CType::GetIntTy() {
    static CType type(TypeKind::Int, 4, 4);
    return &type;
}