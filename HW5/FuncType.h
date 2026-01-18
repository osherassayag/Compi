//
// Created by roro1 on 27/12/2025.
//

#pragma once
#include "nodes.hpp"
#include "SymbType.h"
#include <vector>
#include <string>
class FuncType : public SymbType {
private:
    std::vector<ast::BuiltInType> argTypes;
    ast::BuiltInType retType;
public:
    FuncType(const std::vector<ast::BuiltInType>& argTypes,
             ast::BuiltInType retType);
    ~FuncType();
    const std::vector<ast::BuiltInType>& getArgTypes() const;
    ast::BuiltInType getReturnType() const;
    void setArgTypes(const std::vector<ast::BuiltInType>& argTypes);
    void setReturnType(ast::BuiltInType retType);
    bool IsSameType(const FuncType& other) const;
};
