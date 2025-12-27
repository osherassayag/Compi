//
// Created by roro1 on 27/12/2025.
//

#ifndef COMPI_SYMBOLTABLE_H
#define COMPI_SYMBOLTABLE_H
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
    void Insert(const std::string& name,
                std::shared_ptr<SymbType> type,
                int offset);
    SymbolTable();
    bool contains(const std::string name&);
    virtual ~SymbolTable();

};
#endif //COMPI_SYMBOLTABLE_H
