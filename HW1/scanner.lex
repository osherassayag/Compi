#include tokens.hpp

%option yylineno
%option noyywrap

static char[1024] str;

digit [0-9]
pos_digit [1-9]
letter [a-zA-z]
whitespace [\t\n]

%x COMMENT 
%x STRING 

VOID void
INT int 
BYTE byte 
BOOL bool 
AND and 
OR or 
NOT not 
TRUE true 
FALSE false 
RETURN return 
IF if 
ELSE else 
WHILE while 
BREAK break 
CONTINUE continue 
SC ;
COMMA ,
LPAREN (
RPAREN )
LBRACE {
RBRACE }
LBRACK [
RBRACK ]
ASSIGN = 
RELOP == | != | < | > | <= | >=
BINOP + | - | * | \/
//COMMENT \/\/[^\n\r(\r\n)]*
ID letter+(letter | digit)*
NUM pos_digit+digit* | 0
NUM_B NUMb


%%
\"      BEGIN(STRING);
<STRING>\"      {}
<STRING>



%%
void reset_str() {
    for(int i = 0; i < 1023; i++) str[i] = 0;
    str[i] = '\0';
}
