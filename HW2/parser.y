%{

#include "nodes.hpp"
#include "output.hpp"

// bison declarations
extern int yylineno;
extern int yylex();

void yyerror(const char*);

// root of the AST, set by the parser and used by other parts of the compiler
std::shared_ptr<ast::Node> program;

using namespace std;

// TODO: Place any additional declarations here
%}

// TODO: Define tokens here
%token INT BYTE BOOL TRUE FALSE RETURN WHILE BREAK CONTINUE ID NUM NUMB STRING SC COMMA
%left IF ELSE
%right ASSIGN
%left OR
%left AND
%left NOT
%left GT LT GE LE
%left ADD DEC
%left MULT DIV
%left LPAREN RPAREN LBRACK RBRACK
// TODO: Define precedence and associativity here

%%
// TODO: Define grammar here

// While reducing the start variable, set the root of the AST
Program:  Funcs { program = $1; }
;

Funcs:
    | FuncsDecl Funcs
;

FuncsDecl: RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE
;

RetType: Type
    | VOID
;

Formals:
    | FormalsList
;

FormalsList: FormalDecl
    | FormalDecl COMMA FormalsList
;

FormalDecl: Type ID {$$ = std::make_shared<ast::Formal>($2, $1); }
;

Statements: Statement {$$ = std::make_shared<ast::Statements>($1);}
    | Statements Statement {$$ = $1 $$.push_back($2);}
;


Statement: LBRACE Statements RBRACE {$$ = $2;}
    | Type ID SC {$$ = std::make_shared<ast::VarDecl>($2, $1); }
    | Type ID ASSIGN Exp SC {$$ = std::make_shared<ast::VarDecl>($2, $1, $4); }
    | ID ASSIGN Exp SC {$$ = std::make_shared<ast::Assign>($1, $3); }
    | Call SC
    | RETURN SC
    | RETURN Exp SC
    | IF LPAREN Exp RPAREN Statement
    | IF LPAREN Exp RPAREN Statement ELSE Statement
    | WHILE LPAREN Exp RPAREN Statement
    | BREAK SC
    | CONTINUE SC
;

Call: ID LPAREN ExpList RPAREN  {$$ = std::make_shared<ast::Call>($1, $3); }
    | ID LPAREN RPAREN {$$ = std::make_shared<ast::Call>($1); }
;

ExpList: Exp {$$ = std::make_shared<ast::ExpList>($1); }
    | Exp COMMA ExpList  {$$ = $3; $$.push_front($1); }
;

Type: INT {$$ = std::make_shared<ast::Type>(INT)}
    | BYTE {$$ = std::make_shared<ast::Type>(BYTE)}
    | BOOL {$$ = std::make_shared<ast::Type>(BOOL)}
;

Exp: LPAREN Exp RPAREN {$$ = std::make_shared<ast::Exp>() }
    | Exp ADD Exp {$$ = std::make_shared<ast::BinOp>($1, $3, ADD); }
    | Exp DEC Exp {$$ = std::make_shared<ast::BinOp>($1, $3, DEC);}
    | Exp MULT Exp {$$ = std::make_shared<ast::BinOp>($1, $3, MUL); }
    | Exp DIV Exp {$$ = std::make_shared<ast::BinOp>($1, $3, DIV); }
    | ID {$$ = $1;}
    | Call {$$ = $1;}
    | NUM {$$ = $1;}
    | NUMB {$$ = $1;}
    | STRING {$$ = $1;}
    | TRUE {$$ = std::make_shared<ast::Bool>(true); }
    | FALSE {$$ = std::make_shared<ast::Bool>(false);}
    | NOT Exp {$$ = std::make_shared<ast::Not>($1); }
    | Exp AND Exp {$$ = std::make_shared<ast::And>($1, $2); }
    | Exp OR Exp {$$ = std::make_shared<ast::Or>($1, $2); }
    | Exp EQ Exp {$$ = std::make_shared<ast::RelOp>($1, $2, EQ); }
    | Exp NQ Exp {$$ = std::make_shared<ast::RelOp>($1, $2, NE); }
    | Exp GT Exp {$$ = std::make_shared<ast::RelOp>($1, $2, GT); }
    | Exp GE Exp {$$ = std::make_shared<ast::RelOp>($1, $2, GE); }
    | Exp LT Exp {$$ = std::make_shared<ast::RelOp>($1, $2, LT); }
    | Exp LE Exp {$$ = std::make_shared<ast::RelOp>($1, $2, LE); }
    | LPAREN Type RPAREN Exp {$$ = std::make_shared<ast::Cast>($4, $2);}
;



%%

// TODO: Place any additional code here