//
// Created by eyalk on 26/12/2025.
//

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <stack>
#include "visitor.hpp"
#include "output.hpp"
#include "SymbolTable.h"
class MyVisitor : public Visitor {
    output::ScopePrinter scopePrinter;
    std::stack<int> scopeOffsets;
    std::stack<std::shared_ptr<SymbolTable>> tables;
    bool funcDeclBeginScope;
    void beginScope();
    void endScope();
    void declareFunc(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> return_type,
                     const std::shared_ptr<ast::Formals> &formals);
    void declareBuiltInFunc(std::string id, ast::BuiltInType return_type,
                           const std::vector<ast::BuiltInType> &paramTypes);
    void declareVar(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> type, int offset);
    void checkNotDefinedAnywhere(const std::string& name, int line);
    Entry* lookup(const std::string& name);
public:
    MyVisitor();

    void visit(ast::Num &node);

    void visit(ast::NumB &node);

    void visit(ast::String &node);

    void visit(ast::Bool &node);

    void visit(ast::ID &node);

    void visit(ast::BinOp &node);

    void visit(ast::RelOp &node);

    void visit(ast::Not &node);

    void visit(ast::And &node);

    void visit(ast::Or &node);

    void visit(ast::Type &node);

    void visit(ast::Cast &node);

    void visit(ast::ExpList &node);

    void visit(ast::Call &node);

    void visit(ast::Statements &node);

    void visit(ast::Break &node);

    void visit(ast::Continue &node);

    void visit(ast::Return &node);

    void visit(ast::If &node);

    void visit(ast::While &node);

    void visit(ast::VarDecl &node);

    void visit(ast::Assign &node);

    void visit(ast::Formal &node);

    void visit(ast::Formals &node);

    void visit(ast::FuncDecl &node);

    void visit(ast::Funcs &node);
};
