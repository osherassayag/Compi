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
    scopeOffsets.push(0);
    funcDeclBeginScope = false;
    declareBuiltInFunc("print", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::STRING)
    );
    declareBuiltInFunc("printi", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::INT)
    );
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
    if (node.value > 255)
        output::errorByteTooLarge(node.line, node.value);
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
    if (!e)
        output::errorUndef(node.line, node.value);

    if (dynamic_cast<FuncType*>(e->type.get()))
        output::errorDefAsFunc(node.line, node.value);

    node.type = dynamic_cast<BasicType*>(e->type.get())->getDeclaredType();
}


void MyVisitor::visit(ast::BinOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.left->type != ast::BuiltInType::INT ||
        node.right->type != ast::BuiltInType::INT)
        output::errorMismatch(node.line);

    node.type = ast::BuiltInType::INT;

}

void MyVisitor::visit(ast::RelOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.left->type != node.right->type)
        output::errorMismatch(node.line);

    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Not &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
}

void MyVisitor::visit(ast::And &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
}

void MyVisitor::visit(ast::Or &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
}

void MyVisitor::visit(ast::Type &node) {

}

void MyVisitor::visit(ast::Cast &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.target_type.get() != nullptr) node.target_type->accept(*this);
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
    if (node.func_id.get() != nullptr) node.func_id->accept(*this);
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
        if (node.args->exps[i]->type != ftype->getArgTypes()[i])
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

}

void MyVisitor::visit(ast::Continue &node) {

}

void MyVisitor::visit(ast::Return &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
}

void MyVisitor::visit(ast::If &node) {
    beginScope();
    if (node.condition.get() != nullptr) node.condition->accept(*this);
    if (node.then.get() != nullptr) node.then->accept(*this);
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
    if (node.body.get() != nullptr) node.body->accept(*this);
    endScope();
}

void MyVisitor::declareVar(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> type,
                           int offset) {
    scopePrinter.emitVar(id->value, type->type, offset);
}

void MyVisitor::visit(ast::VarDecl &node) {
    declareVar(node.id, node.type, scopeOffsets.top());
    scopeOffsets.top()++;
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.type.get() != nullptr) node.type->accept(*this);
    if (node.init_exp.get() != nullptr) node.init_exp->accept(*this);
}

void MyVisitor::visit(ast::Assign &node) {
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.id->type != node.exp->type)
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

void MyVisitor::declareBuiltInFunc(std::string id, ast::BuiltInType return_type,
                                   const std::vector<ast::BuiltInType> &paramTypes) {
    scopePrinter.emitFunc(id, return_type, paramTypes);
}

void MyVisitor::declareFunc(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> return_type,
                            const std::shared_ptr<ast::Formals> &formals) {
    std::vector<ast::BuiltInType> paramTypes;
    for (auto formal : formals->formals) {
        paramTypes.push_back(formal->type->type);
    }
    scopePrinter.emitFunc(id->value, return_type->type, paramTypes);
}

void MyVisitor::visit(ast::FuncDecl &node) {
    declareFunc(node.id, node.return_type, node.formals);
    beginScope();
    funcDeclBeginScope = true;
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.return_type.get() != nullptr) node.return_type->accept(*this);
    if (node.formals.get() != nullptr) node.formals->accept(*this);
    if (node.body.get() != nullptr)  node.body->accept(*this);
    endScope();
}

void MyVisitor::visit(ast::Funcs &node) {
    for (auto& funcPtr: node.funcs) {
        if (funcPtr.get() != nullptr) funcPtr->accept(*this);
    }
    std::cout << scopePrinter;
}

Entry* MyVisitor::lookup(const std::string& name) {
    std::stack<std::shared_ptr<SymbolTable>> tmp = tables;
    while (!tmp.empty()) {
        auto& table = tmp.top();
        if (table.get()->contains(name))
            return table.get()->get(name);
        tmp.pop();
    }
    return nullptr;
}

void MyVisitor::checkNotDefinedAnywhere(const std::string& name, int line) {
    if (lookup(name))
        output::errorDef(line, name);
}


