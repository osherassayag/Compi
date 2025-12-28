//
// Created by eyalk on 26/12/2025.
//

#include <iostream>
#include "MyVisitor.h"
#include "FuncType.h"
#include "BasicType.h"
#include <vector>
#include <stack>
#include <string>
#include <memory>

MyVisitor::MyVisitor() {
    hasMain = false;
    loop_depth = 0;
    tables.push(std::make_shared<SymbolTable>(SymbolTable()));
    scopeOffsets.push(0);
    funcDeclBeginScope = false;
    currentFuncType = ast::BuiltInType::VOID;
    declareBuiltInFunc("print", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::STRING)
    );
    declareBuiltInFunc("printi", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::INT)
    );
}

void MyVisitor::declareBuiltInFunc(std::string id, ast::BuiltInType return_type,
                                   const std::vector<ast::BuiltInType> &paramTypes) {
    scopePrinter.emitFunc(id, return_type, paramTypes);
    tables.top()->insert(id, std::make_shared<FuncType>(paramTypes, return_type), -100);
}

void MyVisitor::declareFunc(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> return_type,
                            const std::shared_ptr<ast::Formals> &formals) {
    Entry* e = lookup(id->value);
    if (e) {
        output::errorDef(id->line, id->value);
    }
    std::vector<ast::BuiltInType> paramTypes;
    for (auto formal : formals->formals) {
        paramTypes.push_back(formal->type->type);
    }
    tables.top()->insert(id->value, std::make_shared<FuncType>(paramTypes, return_type->type), -100);
    scopePrinter.emitFunc(id->value, return_type->type, paramTypes);
}

void MyVisitor::declareVar(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> type,
                           int offset) {
    Entry* e = lookup(id->value);
    if (e) {
        output::errorDef(id->line, id->value);
    }
    tables.top()->insert(id->value, std::make_shared<BasicType>(type->type), offset);
    scopePrinter.emitVar(id->value, type->type, offset);
}

void MyVisitor::beginScope() {
    scopePrinter.beginScope();
    scopeOffsets.push(scopeOffsets.top());
    tables.push(std::make_shared<SymbolTable>(SymbolTable()));
}


void MyVisitor::endScope() {
    scopePrinter.endScope();
    scopeOffsets.pop();
    tables.pop();
}
void MyVisitor::visit(ast::Num& node) {
    node.type = ast::BuiltInType::INT;
}

void MyVisitor::visit(ast::NumB& node) {
    if (node.value > 255)
        output::errorByteTooLarge(node.line, node.value);
    node.type = ast::BuiltInType::BYTE;
}

void MyVisitor::visit(ast::String& node) {
    node.type = ast::BuiltInType::STRING;
}

void MyVisitor::visit(ast::Bool& node) {
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::ID& node) {
    Entry* e = lookup(node.value);
    if (!node.isDeclaration) {
        if (!e) {
            if (node.isUsedAsFunction) {
                output::errorUndefFunc(node.line, node.value);
            } else {
                output::errorUndef(node.line, node.value);
            }
        } else {
            if (node.isUsedAsFunction) {
                FuncType *funcType = dynamic_cast<FuncType*>(e->type.get());
                if (funcType != nullptr) {
                    node.type = funcType->getReturnType();
                } else {
                    output::errorDefAsVar(node.line, node.value);
                }
            } else {
                BasicType *basicType = dynamic_cast<BasicType *>(e->type.get());
                if (basicType != nullptr) {
                    node.type = basicType->getDeclaredType();
                } else {
                    output::errorDefAsFunc(node.line, node.value);
                }
            }
        }
    } else {
        // declaration handled by the parent
    }
}


void MyVisitor::visit(ast::BinOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (!isNumeric(node.left->type) || !isNumeric(node.right->type))
        output::errorMismatch(node.line);

    if (node.left->type == ast::BuiltInType::INT || node.right->type == ast::BuiltInType::INT) {
        node.type = ast::BuiltInType::INT;
    } else {
        node.type = ast::BuiltInType::BYTE;
    }
}

bool MyVisitor::isNumeric(ast::BuiltInType type) {
    return (type == ast::BuiltInType::INT || type == ast::BuiltInType::BYTE);
}

void MyVisitor::visit(ast::RelOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (!isNumeric(node.left->type) || !isNumeric(node.right->type))
        output::errorMismatch(node.line);

    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Not &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.exp->type != ast::BuiltInType::BOOL)
        output::errorMismatch(node.line);
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::And &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.left->type != ast::BuiltInType::BOOL
        || node.right->type != ast::BOOL)
        output::errorMismatch(node.line);
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Or &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.left->type != ast::BuiltInType::BOOL
        || node.right->type != ast::BOOL)
        output::errorMismatch(node.line);
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Type &node) {

}

void MyVisitor::visit(ast::Cast &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.target_type.get() != nullptr) node.target_type->accept(*this);
    if((node.exp->type != ast::INT && node.exp->type != ast::BYTE)
        || (node.target_type->type != ast::INT && node.target_type->type != ast::BYTE))
            output::errorMismatch(node.line);
    node.type = node.target_type->type;
}

void MyVisitor::visit(ast::ExpList &node) {
    for (auto& expPtr: node.exps) {
        if (expPtr.get() != nullptr) expPtr->accept(*this);
    }
}

std::string MyVisitor::toString(ast::BuiltInType type) {
    switch (type) {
        case ast::BuiltInType::INT:
            return "int";
        case ast::BuiltInType::BOOL:
            return "bool";
        case ast::BuiltInType::BYTE:
            return "byte";
        case ast::BuiltInType::VOID:
            return "void";
        case ast::BuiltInType::STRING:
            return "string";
        default:
            return "unknown";
    }
}

std::vector<std::string> MyVisitor::getFuncParamTypeStrings(std::shared_ptr<ast::ExpList> args) {
    std::vector<std::string> types;
    for (auto exp: args->exps) {
        types.push_back(toString(exp->type));
    }
    return types;
}

void MyVisitor::visit(ast::Call &node) {
    if (node.func_id.get() != nullptr) {
        node.func_id->isUsedAsFunction = true;
        node.func_id->accept(*this);
    }
    if (node.args.get() != nullptr) node.args->accept(*this);
    Entry* e = lookup(node.func_id->value);
    if (!e)
        output::errorUndefFunc(node.line, node.func_id->value);

    auto ftype = dynamic_cast<FuncType*>(e->type.get());
    if (!ftype)
        output::errorDefAsVar(node.line, node.func_id->value);

    node.args->accept(*this);

    if (node.args->exps.size() != ftype->getArgTypes().size()) {
        auto types = getFuncParamTypeStrings(node.args);
        output::errorPrototypeMismatch(node.line, node.func_id->value, types);
    }

    for (size_t i = 0; i < node.args->exps.size(); ++i) {
        if (!isAssignable(ftype->getArgTypes()[i], node.args->exps[i]->type))
            output::errorMismatch(node.line);
    }

    node.type = ftype->getReturnType();
}

void MyVisitor::visit(ast::Statements &node) {
    bool createNewScope = !funcDeclBeginScope;
    if (createNewScope) {
        beginScope();
    }
    funcDeclBeginScope = false;
    for (auto& statementPtr: node.statements) {
        if (statementPtr.get() != nullptr) statementPtr->accept(*this);
    }
    if (createNewScope) {
        endScope();
    }
}

void MyVisitor::visit(ast::Break &node) {
    if(loop_depth == 0) output::errorUnexpectedBreak(node.line);
}

void MyVisitor::visit(ast::Continue &node) {
    if(loop_depth == 0) output::errorUnexpectedContinue(node.line);
}

void MyVisitor::visit(ast::Return &node) {
    ast::BuiltInType returnType = ast::BuiltInType::VOID;

    if (node.exp.get() != nullptr) {
        node.exp->accept(*this);
        returnType = node.exp->type;
    }

    if (!isAssignable(currentFuncType, returnType)) {
        output::errorMismatch(node.line);
    }
}

void MyVisitor::visit(ast::If &node) {
    beginScope();
    if (node.condition.get() != nullptr) node.condition->accept(*this);
    if (node.then.get() != nullptr) node.then->accept(*this);
    if(node.condition->type != ast::BOOL)
        output::errorMismatch(node.line);
    endScope();
    if (node.otherwise.get() != nullptr) {
        beginScope();
        node.otherwise->accept(*this);
        endScope();
    }

}

void MyVisitor::visit(ast::While &node) {
    beginScope();
    if (node.condition.get() != nullptr) node.condition->accept(*this);
    if(node.condition->type != ast::BOOL)
        output::errorMismatch(node.line);
    loop_depth++;
    if (node.body.get() != nullptr) node.body->accept(*this);
    loop_depth--;
    endScope();
}

void MyVisitor::visit(ast::VarDecl &node) {
    if (node.id.get() != nullptr) {
        node.id->isDeclaration = true;
    }
    if (node.type.get() != nullptr) node.type->accept(*this);
    if (node.init_exp.get() != nullptr) {
        node.init_exp->accept(*this);

        if (!isAssignable(node.type->type, node.init_exp->type))
            output::errorMismatch(node.line);
    }

    declareVar(node.id, node.type, scopeOffsets.top());
    scopeOffsets.top()++;
}

bool MyVisitor::isAssignable(ast::BuiltInType target, ast::BuiltInType source) {
    return target == source ||
           (target == ast::BuiltInType::INT && source == ast::BuiltInType::BYTE);
}

void MyVisitor::visit(ast::Assign &node) {
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (!isAssignable(node.id->type, node.exp->type))
        output::errorMismatch(node.line);
}

void MyVisitor::visit(ast::Formal &node) {
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.type.get() != nullptr) node.type->accept(*this);
}

void MyVisitor::visit(ast::Formals &node) {
    int i = 0;
    for (auto& formalsPtr: node.formals) {
        declareVar(formalsPtr->id, formalsPtr->type, --i);
        if (formalsPtr.get() != nullptr) formalsPtr->accept(*this);
    }
}

void MyVisitor::visit(ast::FuncDecl &node) {
    beginScope();
    funcDeclBeginScope = true;
    if (node.id.get() != nullptr) {
        node.id->isDeclaration = true;
        node.id->accept(*this);
    }
    if (node.return_type.get() != nullptr) {
        node.return_type->accept(*this);
        currentFuncType = node.return_type->type;
    }
    if (node.formals.get() != nullptr) node.formals->accept(*this);
    if (node.body.get() != nullptr)  node.body->accept(*this);
    endScope();
    currentFuncType = ast::BuiltInType::VOID;
}

void MyVisitor::visit(ast::Funcs &node) {
    // First, add all functions to the symbol table, so we can check their types later
    for (auto &funcPtr: node.funcs) {
        if (funcPtr.get() != nullptr) {
            declareFunc(funcPtr->id, funcPtr->return_type, funcPtr->formals);

            if (funcPtr->id->value == "main" &&
                funcPtr->return_type->type == ast::BuiltInType::VOID &&
                funcPtr->formals->formals.empty()) {
                hasMain = true;


            }
        }

        // Now start visiting the tree
        for (auto &funcPtr: node.funcs) {
            if (funcPtr.get() != nullptr) funcPtr->accept(*this);
        }

        if (!hasMain)
            output::errorMainMissing();
        std::cout << scopePrinter;
    }
}
    Entry *MyVisitor::lookup(const std::string &name) {
        std::stack<std::shared_ptr<SymbolTable>> tmp = tables;
        while (!tmp.empty()) {
            auto &table = tmp.top();
            if (table.get()->contains(name))
                return table.get()->get(name);
            tmp.pop();
        }
        return nullptr;
    }
    void MyVisitor::checkNotDefinedAnywhere(const std::string &name, int line) {
        if (lookup(name))
            output::errorDef(line, name);
    }


