//
// Created by roro1 on 27/12/2025.
//
#include "SymbolTable.h"
#include <string>
SymbolTable::SymbolTable() = default;

void SymbolTable::insert(const std::string& name,
                         std::shared_ptr<SymbType> type,
                         int offset) {
    Entry e;
    e.type = type;
    e.offset = offset;
    e.name = name;
    symbols[name] = e;
}

bool SymbolTable::contains(const std::string& name) {
    return symbols.find(name) != symbols.end();
}

Entry *SymbolTable::get(const std::string& name) {
    return &(symbols.find(name)->second);
}

SymbolTable::~SymbolTable() = default;