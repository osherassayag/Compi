//
// Created by roro1 on 27/12/2025.
//

#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "SymbType.h"

struct Entry {
    std::string name;
    std::shared_ptr<SymbType> type;
    int offset;
};


class SymbolTable {
private:
    std::unordered_map<std::string, Entry> symbols;
public:
    void insert(const std::string& name,
                std::shared_ptr<SymbType> type,
                int offset);
    SymbolTable();
    bool contains(const std::string& name);
    Entry *get(const std::string& name);
    virtual ~SymbolTable();

};
