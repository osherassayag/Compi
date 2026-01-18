//
// Created by roro1 on 27/12/2025.
//
#include "FuncType.h"

FuncType::FuncType(const std::vector<ast::BuiltInType>& argTypes,
                   ast::BuiltInType retType)
        : argTypes(argTypes), retType(retType) {}

FuncType::~FuncType() = default;
const std::vector<ast::BuiltInType>& FuncType::getArgTypes() const {
    return argTypes;
}

ast::BuiltInType FuncType::getReturnType() const {
    return retType;
}

void FuncType::setArgTypes(const std::vector<ast::BuiltInType>& argTypes) {
    this->argTypes = argTypes;
}

void FuncType::setReturnType(ast::BuiltInType retType) {
    this->retType = retType;
}
bool FuncType::IsSameType(const FuncType& other) const {
    if (retType != other.retType)
        return false;

    if (argTypes.size() != other.argTypes.size())
        return false;

    for (size_t i = 0; i < argTypes.size(); ++i) {
        if (argTypes[i] != other.argTypes[i])
            return false;
    }
    return true;
}