%{
#include "tokens.hpp"
#include "output.hpp"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
void resetBuffer(char buffer[BUFFER_SIZE]);
void appendChar(char buffer[BUFFER_SIZE], char c);
%}

%option yylineno
%option noyywrap

digit [0-9]
pos_digit [1-9]
letter [a-zA-Z]
whitespace [ \t\r\n]

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
SC ";"
COMMA ","
LPAREN "("
RPAREN ")"
LBRACE "{"
RBRACE "}"
LBRACK "["
RBRACK "]"
ASSIGN "="
//RELOP "=="|"!="|"<"|">"|"<="|">="
//BINOP "+"|"-"|"*"|"\/"
//EQUALITY "==" | "!="
EQ "=="
NQ "!="
ADD "+"
DEC "-"
MULT "*"
DIV "/"
GT ">"
LT "<"
GE ">="
LE "<="
//RELATIONAL "<"|">"|"<="|">="
new_line  [\n\r]
ID {letter}({letter}|{digit})*
NUM ({pos_digit}{digit}*)|0
NUM_B {NUM}b

%%

{VOID}  { return VOID;}
{INT}   {return INT;}
{BYTE}  { return BYTE;}
{BOOL}	{ return BOOL; }
{AND}	{ return AND; }
{OR}	{ return OR; }
{NOT}	{ return NOT; }
{TRUE}	{ return TRUE; }
{FALSE}	{ return FALSE; }
{RETURN}	{ return RETURN; }
{IF}	{ return IF; }
{ELSE}	{ return ELSE; }
{WHILE}	{ return WHILE; }
{BREAK}	{ return BREAK; }
{CONTINUE}	{ return CONTINUE; }
{SC}	{ return SC; }
{COMMA}	{ return COMMA; }
{LPAREN}	{ return LPAREN; }
{RPAREN}	{ return RPAREN; }
{LBRACE}	{ return LBRACE; }
{RBRACE}	{ return RBRACE; }
{LBRACK}	{ return LBRACK; }
{RBRACK}	{ return RBRACK; }
{ASSIGN}	{ return ASSIGN; }
{EQ} {return EQ;}
{NQ} {return NQ;}
{DEC} {return DEC;}
{ADD} {return ADD;}
//{RELATIONAL} {return RELATIONAL;}
{MULT} {return MULT;}
{DIV} {return DIV;}
{GT} {return GT;}
{GE} {return GE;}
{LT} {return LT;}
{LE} {return LE;}
//{RELOP}	{ return RELOP; }
//{BINOP}	{ return BINOP; }
{ID}	{yylval = std::make_shared<ast::ID>(yytext); return ID; }
{NUM}	{yylval = std::make_shared<ast::Num>(yytext); return NUM; }
{NUM_B}	{yylval = std::make_shared<ast::NumB>(yytext); return NUM_B; }
"//"[^\r\n]* {}

\"      { resetBuffer(buffer); BEGIN(STRING_COND); }
<STRING_COND>\"      {yylval = std::make_shared<ast::String>(buffer); BEGIN(INITIAL);  return STRING; }
<STRING_COND>\\n     {appendChar(buffer, '\n');}
<STRING_COND>\\r     {appendChar(buffer, '\r');}
<STRING_COND>\\t     {appendChar(buffer, '\t');}
<STRING_COND>\\\\    {appendChar(buffer, '\\');}
<STRING_COND>\\\"     {appendChar(buffer, '\"');}
<STRING_COND>\\0     {appendChar(buffer, '\0');}
<STRING_COND>{new_line} {output::errorLex(yylineno);}
<STRING_COND>(\\x({letter}|{digit})?({letter}|{digit}))|(\\.)      {output::errorLex(yylineno);}
<STRING_COND>.       {appendChar(buffer, yytext[0]);}
<STRING_COND><<EOF>> {output::errorLex(yylineno);}

{whitespace} {}
. {output::errorLex(yylineno);}

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


