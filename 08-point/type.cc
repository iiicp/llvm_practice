#include "type.h"

std::shared_ptr<CType> CType::IntType = std::make_shared<CPrimaryType>(Kind::TY_Int, 4, 4);