//
// Created by roro1 on 27/12/2025.
//

#include "BasicType.h"


BasicType::BasicType(ast::BuiltInType type)
        : declaredType(type) {}


BasicType::~BasicType() = default;


ast::BuiltInType BasicType::getDeclaredType() const {
    return declaredType;
}

void BasicType::setDeclaredType(ast::BuiltInType type) {
    declaredType = type;
}

