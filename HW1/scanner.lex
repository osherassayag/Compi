%{
#include "tokens.hpp"
#include "output.hpp"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
void resetBuffer(char buffer[BUFFER_SIZE]);
void appendChar(char buffer[BUFFER_SIZE], char c);
char hexToChar(char *s);
%}


%option yylineno
%option noyywrap

digit [0-9]
pos_digit [1-9]
letter [a-zA-z]
whitespace [\t\n]

%x COMMENT
%x STRING_COND 

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
hex \\x(([2-6][0-9A-Fa-f])|(7[0-9A-Ea-e]))
ID {letter}({letter}|{digit})*
NUM ({pos_digit}{digit}*) | 0
NUM_B {NUM}b


%%

{VOID}  {return VOID;}

\"      { resetBuffer(buffer); BEGIN(STRING_COND); }
<STRING_COND>\"      {BEGIN(INITIAL); return STRING; }
<STRING_COND>\\n     {appendChar(buffer, '\n');}
<STRING_COND>\\r     {appendChar(buffer, '\r');}
<STRING_COND>\\t     {appendChar(buffer, '\t');}
<STRING_COND>\\\\    {appendChar(buffer, '\\');}
<STRING_COND>\\\"     {appendChar(buffer, '\"');}
<STRING_COND>\\0     {appendChar(buffer, '\0');}
<STRING_COND>{hex}     {appendChar(buffer, hexToChar(yytext));}
<STRING_COND>{new_line}       {output::errorUnclosedString();}
<STRING_COND>\\.      {output::errorUndefinedEscape(yytext);}
<STRING_COND>.       {appendChar(buffer, yytext[0]);}

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

//Function to convert a hex character to an int
int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return c - 'a' + 10;
}

//Function to convert hex to a character
char hexToChar(char *s) {
    int ascii = hexCharToInt(s[2]) * 16 + hexCharToInt(s[3]);
    return ascii;
}



