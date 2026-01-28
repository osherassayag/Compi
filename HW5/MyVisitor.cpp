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
    
    // Initialize LLVM globals
    initializeGlobals();
    
    // Declare built-in functions
    declareBuiltInFunc("print", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::STRING));
    declareBuiltInFunc("printi", ast::BuiltInType::VOID,
                       std::vector<ast::BuiltInType>(1, ast::BuiltInType::INT));
}

void MyVisitor::initializeGlobals() {
    // Emit required declarations (from print_functions.llvm)
    buffer.emitGlobal("declare i32 @scanf(i8*, ...)");
    buffer.emitGlobal("declare i32 @printf(i8*, ...)");
    buffer.emitGlobal("declare void @exit(i32)");
    buffer.emitGlobal("@.int_specifier_scan = constant [3 x i8] c\"%d\\00\"");
    buffer.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    buffer.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    buffer.emitGlobal("@.div_zero_error = constant [23 x i8] c\"Error division by zero\\00\"");
}

void MyVisitor::emitPrintFunctions() {
    // Emit readi function (from print_functions.llvm)
    buffer.emitGlobal("\ndefine i32 @readi(i32) {");
    buffer.emitGlobal("    %ret_val = alloca i32");
    buffer.emitGlobal("    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0");
    buffer.emitGlobal("    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)");
    buffer.emitGlobal("    %val = load i32, i32* %ret_val");
    buffer.emitGlobal("    ret i32 %val");
    buffer.emitGlobal("}");
    
    // Emit printi function (from print_functions.llvm)
    buffer.emitGlobal("\ndefine void @printi(i32) {");
    buffer.emitGlobal("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    buffer.emitGlobal("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    buffer.emitGlobal("    ret void");
    buffer.emitGlobal("}");
    
    // Emit print function (from print_functions.llvm)
    buffer.emitGlobal("\ndefine void @print(i8*) {");
    buffer.emitGlobal("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    buffer.emitGlobal("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    buffer.emitGlobal("    ret void");
    buffer.emitGlobal("}");
}

std::string MyVisitor::getLLVMType(ast::BuiltInType type) {
    switch (type) {
        case ast::BuiltInType::INT:
        case ast::BuiltInType::BYTE:
        case ast::BuiltInType::BOOL:
            return "i32";
        case ast::BuiltInType::VOID:
            return "void";
        case ast::BuiltInType::STRING:
            return "i8*";
        default:
            return "i32";
    }
}

std::string MyVisitor::getLLVMFuncType(ast::BuiltInType retType, const std::vector<ast::BuiltInType>& argTypes) {
    std::string result = getLLVMType(retType) + " (";
    for (size_t i = 0; i < argTypes.size(); ++i) {
        if (i > 0) result += ", ";
        result += getLLVMType(argTypes[i]);
    }
    result += ")";
    return result;
}

void MyVisitor::declareBuiltInFunc(std::string id, ast::BuiltInType return_type,
                                   const std::vector<ast::BuiltInType> &paramTypes) {
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
}

void MyVisitor::declareVar(std::shared_ptr<ast::ID> id, std::shared_ptr<ast::Type> type,
                           int offset) {
    Entry* e = lookup(id->value);
    if (e) {
        output::errorDef(id->line, id->value);
    }
    tables.top()->insert(id->value, std::make_shared<BasicType>(type->type), offset);
}

void MyVisitor::beginScope() {
    scopeOffsets.push(scopeOffsets.top());
    tables.push(std::make_shared<SymbolTable>(SymbolTable()));
}

void MyVisitor::endScope() {
    scopeOffsets.pop();
    tables.pop();
}

void MyVisitor::emitDivisionByZeroCheck(const std::string& divisor) {
    std::string zeroLabel = buffer.freshLabel();
    std::string okLabel   = buffer.freshLabel();

    std::string cmp = buffer.freshVar();
    buffer.emit("    " + cmp + " = icmp eq i32 " + divisor + ", 0");
    buffer.emit("    br i1 " + cmp + ", label " + zeroLabel + ", label " + okLabel);

    buffer.emitLabel(zeroLabel);
    std::string msgPtr = buffer.freshVar();
    buffer.emit("    " + msgPtr +
                " = getelementptr [23 x i8], [23 x i8]* @.div_zero_error, i32 0, i32 0");
    buffer.emit("    call void @print(i8* " + msgPtr + ")");
    buffer.emit("    call void @exit(i32 0)");
    buffer.emit("    unreachable");

    buffer.emitLabel(okLabel);
}


void MyVisitor::visit(ast::Num& node) {
    node.type = ast::BuiltInType::INT;
    node.place = std::to_string(node.value);
}

void MyVisitor::visit(ast::NumB& node) {
    if (node.value > 255)
        output::errorByteTooLarge(node.line, node.value);
    node.type = ast::BuiltInType::BYTE;
    node.place = std::to_string(node.value);
}

void MyVisitor::visit(ast::String& node) {
    node.type = ast::BuiltInType::STRING;
    std::string strVar = buffer.emitString(node.value);
    std::string temp = buffer.freshVar();
    int len = node.value.length() + 1;
    buffer.emit("    " + temp + " = getelementptr [" + std::to_string(len) + " x i8], [" + 
                std::to_string(len) + " x i8]* " + strVar + ", i32 0, i32 0");
    node.place = temp;
}

void MyVisitor::visit(ast::Bool& node) {
    node.type = ast::BuiltInType::BOOL;
    node.place = node.value ? "1" : "0";
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
                    
                    // Load the variable value from stack
                    if (!node.isLvalue) {
                        std::string temp = buffer.freshVar();
                        std::string varPtr = varToRegister[node.value];
                        buffer.emit("    " + temp + " = load i32, i32* " + varPtr);
                        node.place = temp;
                    } else {
                        node.place = varToRegister[node.value];
                    }
                } else {
                    output::errorDefAsFunc(node.line, node.value);
                }
            }
        }
    }
}

std::string MyVisitor::mapBinOp(ast::BinOpType binOpType, ast::BuiltInType operandType) {
    switch (binOpType) {
        case ast::ADD:
            return "add";
        case ast::MUL:
            return "mul";
        case ast::SUB:
            return "sub";
        case ast::DIV:
            if(operandType == ast::INT) return "sdiv";
            return "udiv";
    }
    return "add";
}

std::string MyVisitor::mapRelOp(ast::RelOpType relOpType) {
    switch (relOpType) {
        case ast::EQ:
            return "eq";
        case ast::NE:
            return "ne";
        case ast::LT:
            return "slt";
        case ast::GT:
            return "sgt";
        case ast::LE:
            return "sle";
        case ast::GE:
            return "sge";
    }
    return "eq";
}

void MyVisitor::visit(ast::BinOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    
    if (!isNumeric(node.left->type) || !isNumeric(node.right->type))
        output::errorMismatch(node.line);
    
    std::string leftPlace = node.left->place;
    std::string rightPlace = node.right->place;
    
    // Convert byte to int if needed
    if (node.left->type == ast::BYTE && node.right->type == ast::INT) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + leftPlace + ", 255");
        leftPlace = temp;
    }
    if (node.right->type == ast::BYTE && node.left->type == ast::INT) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + rightPlace + ", 255");
        rightPlace = temp;
    }
    
    // Check for division by zero
    if (node.op == ast::DIV) {
        emitDivisionByZeroCheck(rightPlace);
    }
    
    std::string temp = buffer.freshVar();
    ast::BuiltInType resultType = (node.left->type == ast::INT || node.right->type == ast::INT) ? 
                                   ast::INT : ast::BYTE;
    
    std::string op = mapBinOp(node.op, resultType);
    buffer.emit("    " + temp + " = " + op + " i32 " + leftPlace + ", " + rightPlace);
    
    // Truncate if result should be byte
    if (resultType == ast::BYTE) {
        std::string truncated = buffer.freshVar();
        buffer.emit("    " + truncated + " = and i32 " + temp + ", 255");
        node.place = truncated;
    } else {
        node.place = temp;
    }
    
    node.type = resultType;
}

bool MyVisitor::isNumeric(ast::BuiltInType type) {
    return (type == ast::BuiltInType::INT || type == ast::BuiltInType::BYTE);
}

void MyVisitor::visit(ast::RelOp &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.right.get() != nullptr) node.right->accept(*this);
    
    if (!isNumeric(node.left->type) || !isNumeric(node.right->type))
        output::errorMismatch(node.line);
    
    std::string leftPlace = node.left->place;
    std::string rightPlace = node.right->place;
    
    // Convert byte to int if needed for comparison
    if (node.left->type == ast::BYTE) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + leftPlace + ", 255");
        leftPlace = temp;
    }
    if (node.right->type == ast::BYTE) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + rightPlace + ", 255");
        rightPlace = temp;
    }
    
    std::string cmpResult = buffer.freshVar();
    std::string op = mapRelOp(node.op);
    buffer.emit("    " + cmpResult + " = icmp " + op + " i32 " + leftPlace + ", " + rightPlace);
    
    std::string temp = buffer.freshVar();
    buffer.emit("    " + temp + " = zext i1 " + cmpResult + " to i32");
    
    node.place = temp;
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Not &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.exp->type != ast::BuiltInType::BOOL)
        output::errorMismatch(node.exp->line);
    
    std::string cmpResult = buffer.freshVar();
    buffer.emit("    " + cmpResult + " = icmp eq i32 " + node.exp->place + ", 0");
    
    std::string temp = buffer.freshVar();
    buffer.emit("    " + temp + " = zext i1 " + cmpResult + " to i32");
    
    node.place = temp;
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::And &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.left->type != ast::BuiltInType::BOOL)
        output::errorMismatch(node.line);
    
    // Short-circuit evaluation
    std::string checkLabel = buffer.freshLabel();
    std::string trueLabel = buffer.freshLabel();
    std::string falseLabel = buffer.freshLabel();
    std::string endLabel = buffer.freshLabel();
    
    std::string leftCmp = buffer.freshVar();
    buffer.emit("    " + leftCmp + " = icmp ne i32 " + node.left->place + ", 0");
    buffer.emit("    br i1 " + leftCmp + ", label " + checkLabel + ", label " + falseLabel);
    
    buffer.emitLabel(checkLabel);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.right->type != ast::BOOL)
        output::errorMismatch(node.line);
    
    std::string rightCmp = buffer.freshVar();
    buffer.emit("    " + rightCmp + " = icmp ne i32 " + node.right->place + ", 0");
    buffer.emit("    br i1 " + rightCmp + ", label " + trueLabel + ", label " + falseLabel);
    
    buffer.emitLabel(trueLabel);
    buffer.emit("    br label " + endLabel);
    
    buffer.emitLabel(falseLabel);
    buffer.emit("    br label " + endLabel);
    
    buffer.emitLabel(endLabel);
    std::string result = buffer.freshVar();
    buffer.emit("    " + result + " = phi i32 [1, " + trueLabel + "], [0, " + falseLabel + "]");
    
    node.place = result;
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Or &node) {
    if (node.left.get() != nullptr) node.left->accept(*this);
    if (node.left->type != ast::BuiltInType::BOOL)
        output::errorMismatch(node.line);
    
    // Short-circuit evaluation
    std::string checkLabel = buffer.freshLabel();
    std::string trueLabel = buffer.freshLabel();
    std::string falseLabel = buffer.freshLabel();
    std::string endLabel = buffer.freshLabel();
    
    std::string leftCmp = buffer.freshVar();
    buffer.emit("    " + leftCmp + " = icmp ne i32 " + node.left->place + ", 0");
    buffer.emit("    br i1 " + leftCmp + ", label " + trueLabel + ", label " + checkLabel);
    
    buffer.emitLabel(checkLabel);
    if (node.right.get() != nullptr) node.right->accept(*this);
    if (node.right->type != ast::BOOL)
        output::errorMismatch(node.line);
    
    std::string rightCmp = buffer.freshVar();
    buffer.emit("    " + rightCmp + " = icmp ne i32 " + node.right->place + ", 0");
    buffer.emit("    br i1 " + rightCmp + ", label " + trueLabel + ", label " + falseLabel);
    
    buffer.emitLabel(trueLabel);
    buffer.emit("    br label " + endLabel);
    
    buffer.emitLabel(falseLabel);
    buffer.emit("    br label " + endLabel);
    
    buffer.emitLabel(endLabel);
    std::string result = buffer.freshVar();
    buffer.emit("    " + result + " = phi i32 [1, " + trueLabel + "], [0, " + falseLabel + "]");
    
    node.place = result;
    node.type = ast::BuiltInType::BOOL;
}

void MyVisitor::visit(ast::Type &node) {
    // Nothing to do here
}

void MyVisitor::visit(ast::Cast &node) {
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    if (node.target_type.get() != nullptr) node.target_type->accept(*this);
    
    if((node.exp->type != ast::INT && node.exp->type != ast::BYTE)
        || (node.target_type->type != ast::INT && node.target_type->type != ast::BYTE))
            output::errorMismatch(node.exp->line);
    
    // Cast from byte to int or int to byte
    if (node.exp->type == ast::BYTE && node.target_type->type == ast::INT) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + node.exp->place + ", 255");
        node.place = temp;
    } else if (node.exp->type == ast::INT && node.target_type->type == ast::BYTE) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + node.exp->place + ", 255");
        node.place = temp;
    } else {
        node.place = node.exp->place;
    }
    
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

std::vector<std::string> MyVisitor::getFuncParamTypeStrings(const std::vector<ast::BuiltInType>& argTypes) {
    std::vector<std::string> types;
    for (auto type : argTypes) {
        std::string typeStr = toString(type);
        for (char& c : typeStr) {
            c = std::toupper(c);
        }
        types.push_back(typeStr);
    }
    return types;
}

void MyVisitor::visit(ast::Call &node) {
    Entry* e = lookup(node.func_id->value);
    if (!e) {
        output::errorUndefFunc(node.func_id->line, node.func_id->value);
    }

    auto ftype = dynamic_cast<FuncType*>(e->type.get());
    if (!ftype) {
        output::errorDefAsVar(node.line, node.func_id->value);
    }

    if (node.args.get() != nullptr) node.args->accept(*this);

    auto types = getFuncParamTypeStrings(ftype->getArgTypes());
    if (node.args->exps.size() != ftype->getArgTypes().size()) {
        output::errorPrototypeMismatch(node.line, node.func_id->value, types);
    }

    for (size_t i = 0; i < node.args->exps.size(); ++i) {
        if (!isAssignable(ftype->getArgTypes()[i], node.args->exps[i]->type))
            output::errorPrototypeMismatch(node.line, node.func_id->value, types);
    }

    // Generate call
    std::string callStr = "    ";
    if (ftype->getReturnType() != ast::VOID) {
        std::string temp = buffer.freshVar();
        callStr += temp + " = ";
        node.place = temp;
    }
    
    callStr += "call " + getLLVMType(ftype->getReturnType()) + " @" + node.func_id->value + "(";
    
    for (size_t i = 0; i < node.args->exps.size(); ++i) {
        if (i > 0) callStr += ", ";
        
        std::string argPlace = node.args->exps[i]->place;
        
        // Convert byte to int if necessary
        if (node.args->exps[i]->type == ast::BYTE && ftype->getArgTypes()[i] == ast::INT) {
            std::string temp = buffer.freshVar();
            buffer.emit("    " + temp + " = and i32 " + argPlace + ", 255");
            argPlace = temp;
        }
        
        callStr += getLLVMType(ftype->getArgTypes()[i]) + " " + argPlace;
    }
    
    callStr += ")";
    buffer.emit(callStr);
    
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
    buffer.emit("    br label " + breakLabels.top());
}

void MyVisitor::visit(ast::Continue &node) {
    if(loop_depth == 0) output::errorUnexpectedContinue(node.line);
    buffer.emit("    br label " + continueLabels.top());
}

void MyVisitor::visit(ast::Return &node) {
    ast::BuiltInType returnType = ast::BuiltInType::VOID;

    if (node.exp.get() != nullptr) {
        node.exp->accept(*this);
        returnType = node.exp->type;
        
        if (!isAssignable(currentFuncType, returnType)) {
            output::errorMismatch(node.line);
        }
        
        std::string retPlace = node.exp->place;
        
        // Convert byte to int if necessary
        if (returnType == ast::BYTE && currentFuncType == ast::INT) {
            std::string temp = buffer.freshVar();
            buffer.emit("    " + temp + " = and i32 " + retPlace + ", 255");
            retPlace = temp;
        }
        
        buffer.emit("    ret i32 " + retPlace);
    } else {
        if (currentFuncType != ast::VOID) {
            output::errorMismatch(node.line);
        }
        buffer.emit("    ret void");
    }
}

void MyVisitor::visit(ast::If &node) {
    beginScope();
    if (node.condition.get() != nullptr) node.condition->accept(*this);
    if(node.condition->type != ast::BOOL)
        output::errorMismatch(node.condition->line);
    
    std::string thenLabel = buffer.freshLabel();
    std::string elseLabel = buffer.freshLabel();
    std::string endLabel = buffer.freshLabel();
    
    std::string cmpResult = buffer.freshVar();
    buffer.emit("    " + cmpResult + " = icmp ne i32 " + node.condition->place + ", 0");
    
    if (node.otherwise.get() != nullptr) {
        buffer.emit("    br i1 " + cmpResult + ", label " + thenLabel + ", label " + elseLabel);
    } else {
        buffer.emit("    br i1 " + cmpResult + ", label " + thenLabel + ", label " + endLabel);
    }
    
    buffer.emitLabel(thenLabel);
    if (node.then.get() != nullptr) node.then->accept(*this);
    buffer.emit("    br label " + endLabel);
    
    if (node.otherwise.get() != nullptr) {
        buffer.emitLabel(elseLabel);
        beginScope();
        node.otherwise->accept(*this);
        endScope();
        buffer.emit("    br label " + endLabel);
    }
    
    buffer.emitLabel(endLabel);
    endScope();
}

void MyVisitor::visit(ast::While &node) {
    beginScope();
    
    std::string condLabel = buffer.freshLabel();
    std::string bodyLabel = buffer.freshLabel();
    std::string endLabel = buffer.freshLabel();
    
    breakLabels.push(endLabel);
    continueLabels.push(condLabel);
    
    buffer.emit("    br label " + condLabel);
    buffer.emitLabel(condLabel);
    
    if (node.condition.get() != nullptr) node.condition->accept(*this);
    if(node.condition->type != ast::BOOL)
        output::errorMismatch(node.condition->line);
    
    std::string cmpResult = buffer.freshVar();
    buffer.emit("    " + cmpResult + " = icmp ne i32 " + node.condition->place + ", 0");
    buffer.emit("    br i1 " + cmpResult + ", label " + bodyLabel + ", label " + endLabel);
    
    buffer.emitLabel(bodyLabel);
    loop_depth++;
    if (node.body.get() != nullptr) node.body->accept(*this);
    loop_depth--;
    buffer.emit("    br label " + condLabel);
    
    buffer.emitLabel(endLabel);
    
    breakLabels.pop();
    continueLabels.pop();
    
    endScope();
}

void MyVisitor::visit(ast::VarDecl &node) {
    if (node.id.get() != nullptr) {
        node.id->isDeclaration = true;
    }
    if (node.type.get() != nullptr) node.type->accept(*this);
    
    // Allocate space on stack
    std::string varPtr = buffer.freshVar();
    buffer.emit("    " + varPtr + " = alloca i32");
    varToRegister[node.id->value] = varPtr;
    
    std::string initValue;
    if (node.init_exp.get() != nullptr) {
        node.init_exp->accept(*this);

        if (!isAssignable(node.type->type, node.init_exp->type))
            output::errorMismatch(node.line);
        
        initValue = node.init_exp->place;
        
        // Convert byte to int if necessary
        if (node.init_exp->type == ast::BYTE && node.type->type == ast::INT) {
            std::string temp = buffer.freshVar();
            buffer.emit("    " + temp + " = and i32 " + initValue + ", 255");
            initValue = temp;
        }
    } else {
        // Default initialization
        if (node.type->type == ast::INT || node.type->type == ast::BYTE) {
            initValue = "0";
        } else if (node.type->type == ast::BOOL) {
            initValue = "0";
        }
    }
    
    buffer.emit("    store i32 " + initValue + ", i32* " + varPtr);
    
    declareVar(node.id, node.type, scopeOffsets.top());
    scopeOffsets.top()++;
}

bool MyVisitor::isAssignable(ast::BuiltInType target, ast::BuiltInType source) {
    return target == source ||
           (target == ast::BuiltInType::INT && source == ast::BuiltInType::BYTE);
}

void MyVisitor::visit(ast::Assign &node) {
    node.id->isLvalue = true;
    if (node.id.get() != nullptr) node.id->accept(*this);
    if (node.exp.get() != nullptr) node.exp->accept(*this);
    
    if (!isAssignable(node.id->type, node.exp->type))
        output::errorMismatch(node.line);
    
    std::string expPlace = node.exp->place;
    
    // Convert byte to int if necessary
    if (node.exp->type == ast::BYTE && node.id->type == ast::INT) {
        std::string temp = buffer.freshVar();
        buffer.emit("    " + temp + " = and i32 " + expPlace + ", 255");
        expPlace = temp;
    }
    
    buffer.emit("    store i32 " + expPlace + ", i32* " + varToRegister[node.id->value]);
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
    // Emit function declaration
    std::string funcDecl = "\ndefine " + getLLVMType(node.return_type->type) + " @" + node.id->value + "(";
    
    for (size_t i = 0; i < node.formals->formals.size(); ++i) {
        if (i > 0) funcDecl += ", ";
        funcDecl += getLLVMType(node.formals->formals[i]->type->type);
    }
    funcDecl += ") {";

    buffer.emit(funcDecl);
    
    /* Switch to function body
    std::stringstream oldBuffer;
    oldBuffer << buffer;
    */

    beginScope();
    funcDeclBeginScope = true;
    
    if (node.return_type.get() != nullptr) {
        node.return_type->accept(*this);
        currentFuncType = node.return_type->type;
    }
    
    // Allocate and store parameters
    if (node.formals.get() != nullptr) {
        for (size_t i = 0; i < node.formals->formals.size(); ++i) {
            auto& formal = node.formals->formals[i];
            
            std::string varPtr = buffer.freshVar();
            buffer.emit("    " + varPtr + " = alloca i32");
            varToRegister[formal->id->value] = varPtr;
            buffer.emit("    store i32 %" + std::to_string(i) + ", i32* " + varPtr);
        }
        
        node.formals->accept(*this);
    }
    
    if (node.body.get() != nullptr) node.body->accept(*this);
    
    // Add default return if needed
    if (currentFuncType == ast::VOID) {
        buffer.emit("    ret void");
    } else {
        // Default return value
        std::string defaultVal = "0";
        buffer.emit("    ret i32 " + defaultVal);
    }
    
    buffer.emit("}");
    
    endScope();
    currentFuncType = ast::BuiltInType::VOID;
}

void MyVisitor::visit(ast::Funcs &node) {
    // First pass: declare all functions
    for (auto &funcPtr: node.funcs) {
        if (funcPtr.get() != nullptr) {
            declareFunc(funcPtr->id, funcPtr->return_type, funcPtr->formals);

            if (funcPtr->id->value == "main" &&
                funcPtr->return_type->type == ast::BuiltInType::VOID &&
                funcPtr->formals->formals.empty()) {
                hasMain = true;
            }
        }
    }

    if (!hasMain)
        output::errorMainMissing();
    
    // Emit print functions
    emitPrintFunctions();
    
    // Second pass: generate code for all functions
    for (auto &funcPtr: node.funcs) {
        if (funcPtr.get() != nullptr) funcPtr->accept(*this);
    }
    
    std::cout << buffer;
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
