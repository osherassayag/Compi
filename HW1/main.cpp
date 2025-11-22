#include "tokens.hpp"
#include "output.hpp"
#include <iostream>

using namespace output;

extern char buffer[];

int main() {
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
}