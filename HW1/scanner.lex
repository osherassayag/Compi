%{
#include "tokens.hpp"
#include "output.hpp"
#define BUFFER_SIZE 1024
#include <stdio.h>
#include <string.h>

static char buffer[BUFFER_SIZE];

void resetBuffer(char buffer[BUFFER_SIZE]);

void appendChar(char buffer[BUFFER_SIZE], char* lexeme);
%}
%option yylineno
%option noyywrap





digit ([0-9])
pos_digit ([1-9])
letter ([a-zA-Z])
whitespace ([\t\n])


%x COMMENT 
%x STRING 


relop (== | != | < | > | <= | >=)
binop (+ | - | * | \/)


new_line  ([\n\r])
hex ([\x20-\x7E])
id (letter+(letter | digit)*)
num (pos_digit+digit* | 0)



%%
void    {printToken(yylineno, VOID, "void"); return VOID;}
int     {printToken(yylineno, INT, "int"); return INT;}
byte    {printToken(yylineno, BYTE, "byte"); return BYTE;}
bool    {printToken(yylineno, BOOL, "bool"); return BOOL;}
and     {printToken(yylineno, AND, "and"); return AND;}
or      {printToken(yylineno, OR, "or"); return OR;}
not     {printToken(yylineno, NOT, "not"); return NOT;}
true    {printToken(yylineno, TRUE, "true"); return TRUE;}
false   {printToken(yylineno, FALSE, "false"); return FALSE;}
return  {printToken(yylineno, RETURN, "return"); return RETURN;}
if      {printToken(yylineno, IF, "if"); return IF;}
else    {printToken(yylineno, ELSE, "else"); return ELSE;}
break   {printToken(yylineno, BREAK, "break"); return BREAK;}
while   {printToken(yylineno, WHILE, "while"); return WHILE;}
continue    {printToken(yylineno, CONTINUE, "continue");return CONTINUE;}

;   {printToken(yylineno, SC, ";"); return SC;}
,   {printToken(yylineno, COMMA, ","); return COMMA;}
\(   {printToken(yylineno, LPAREN, "("); return LPAREN;}
\)   {printToken(yylineno, RPAREN, ")"); return RPAREN;}
{   {printToken(yylineno, LBRACE, "{"); return LBRACE;}
}   {printToken(yylineno, RBRACE, "}"); return RBRACE;}
\[   {printToken(yylineno, LBRACK, "["); return LBRACK;}
]   {printToken(yylineno, RBRACK, "]"); return RBRACK;}
=   {printToken(yylineno, ASSIGN, "="); return ASSIGN;}
(relop)   {return RELOP;}
(binop)   {return BINOP;}
(num)b    {return NUM_B;}
(num)     {return NUM;}
(id)        {return ID;}
(whitespace)    {}

\"      BEGIN(STRING);
<STRING>\"      {yylval.str = strdup(buffer); resetBuffer(buffer); BEGIN(INITIAL); return STRING;}
<STRING>\\n     {appendChar(buffer, '\n');}
<STRING>\\r     {appendChar(buffer, '\r');}
<STRING>\\t     {appendChar(buffer, '\t');}
<STRING>\\\\    {appendChar(buffer, '\\');}
<STRING>\\\"     {appendChar(buffer, '\"');}
<STRING>\\0     {appendChar(buffer, '\0');}
<STRING>hex     {int hex_val = strtol(yytext +2 , NULL, 16); appendChar(buffer, (char)hex_val);}
<STRING>new_line       {errorUnclosedString();}
<STRING>\\.      {errorUndefinedEscape(yytext);}
<STRING>.       {appendChar(buffer, yytext);}

\/\/    {BEGIN(COMMENT);}
<COMMENT>{new_line}     {BEGIN(INITIAL);}
<COMMENT>.          {};
.       {errorUnknownChar(yytext[1]);}



%%




// Function to reset the array
void resetBuffer(char buffer[BUFFER_SIZE]) {
    memset(buffer, 0, BUFFER_SIZE);
}

// Function to append a character to the array
void appendChar(char buffer[BUFFER_SIZE], char* lexeme) {
    size_t buffer_len = strlen(buffer);
    size_t lexeme_len = strlen(lexeme);
    for (int i = 0; i < lexeme_len; ++i; len++)
        buffer[len] = curr;
    buffer[len] = '\0';
}

