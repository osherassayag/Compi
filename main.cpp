#include "tokens.hpp"
#include "output.hpp"

int main() {
    enum tokentype token;

    // read tokens until the end of file is reached
    while ((token = static_cast<tokentype>(yylex()))) {
        // your code here
    }
    return 0;
}