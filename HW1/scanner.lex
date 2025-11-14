#include tokens.hpp

#include <stdio.h>
#include <string.h>

%option yylineno
%option noyywrap


#define BUFFER_SIZE 1024
static char buffer[BUFFER_SIZE];


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


new_line  [\n\r]
hex \\x[2-7][0-9A-E]
ID letter+(letter | digit)*
NUM pos_digit+digit* | 0
NUM_B NUMb


%%





%%

\"      BEGIN(STRING);
<STRING>\"      {return buffer; resetBuffer(buffer);}
<STRING>\\n     {appendChar(buffer, '\n');}
<STRING>\\r     {appendChar(buffer, '\r');}
<STRING>\\t     {appendChar(buffer, '\t');}
<STRING>\\\\    {appendChar(buffer, '\\');}
<STRING>\\\"     {appendChar(buffer, '\"');}
<STRING>\\0     {appendChar(buffer, '\0');}
<STRING>hex     {appendChar(buffer, yytext);}
<STRING>new_line       {errorUnclosedString();}
<STRING>\\.      {errorUndefinedEscape(yytext);}
<STRING>.       {appendChar(buffer, yytext);}




%%




// Function to reset the array
void resetBuffer(char buffer[BUFFER_SIZE]) {
    memset(buffer, 0, BUFFER_SIZE);
}

// Function to append a character to the array
void appendChar(char buffer[BUFFER_SIZE], char c) {
    size_t len = strlen(buffer);
    buffer[len] = c;
    buffer[len + 1] = '\0';
}



