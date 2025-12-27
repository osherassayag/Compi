//
// Created by roro1 on 27/12/2025.
//

#ifndef COMPI_BASICTYPE_H
#define COMPI_BASICTYPE_H
#include "nodes.hpp"
#include "SymbType.h"

class BasicType : public SymbType {
private:
    ast::BuiltInType declaredType;
public:
    explicit BasicType(ast::BuiltInType type);
    ~BasicType();
    ast::BuiltInType getDeclaredType() const;
    void setDeclaredType(ast::BuiltInType type);
};
#endif //COMPI_BASICTYPE_H
