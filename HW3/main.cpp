#include <iostream>
#include "output.hpp"
#include "nodes.hpp"
#include "tokens.hpp"

using namespace std;

// Extern from the bison-generated parser
extern int yyparse();

extern char buffer[];
extern std::shared_ptr<ast::Node> program;

/*
static const std::string token_names[] = {
        "__FILLER_FOR_ZERO",
        "VOID",
        "INT",
        "BYTE",
        "BOOL",
        "AND",
        "OR",
        "NOT",
        "TRUE",
        "FALSE",
        "RETURN",
        "IF",
        "ELSE",
        "WHILE",
        "BREAK",
        "CONTINUE",
        "SC",
        "COMMA",
        "LPAREN",
        "RPAREN",
        "LBRACE",
        "RBRACE",
        "LBRACK",
        "RBRACK",
        "ASSIGN",
        "RELOP",
        "BINOP",
        "COMMENT",
        "ID",
        "NUM",
        "NUM_B",
        "STRING"
};

void printToken(int lineno, enum tokentype token, const char *value) {
    if (token == COMMENT) {
        std::cout << lineno << " COMMENT //" << std::endl;
    } else {
        std::cout << lineno << " " << token_names[token] << " " << value << std::endl;
    }
}
*/

int main() {
/*
    enum tokentype token;

    // read tokens until the end of file is reached
    while ((token = static_cast<tokentype>(yylex()))) {
        //std::cout << yytext << " " << token << std::endl;
        // your code here
        switch (token) {
            case STRING:
                printToken(yylineno, token, buffer);
                break;
            default:
                printToken(yylineno, token, yytext);
                break;
        }

    }
    return 0;
*/


    // Parse the input. The result is stored in the global variable `program`
    yyparse();

    // Print the AST using the PrintVisitor
    output::PrintVisitor printVisitor;
    program->accept(printVisitor);
}
