#include tokens.hpp

%option yylineno
%option noyywrap

digit [0-9]
pos_digit [1-9]
letter [a-zA-z]
whitespace [\t\n]


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
COMMENT \/\/[^\n\r(\r\n)]*
ID letter+(letter | digit)*
NUM pos_digit+digit* | 0
NUM_B NUMb


%%




%%