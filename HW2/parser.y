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


%}


%token INT BYTE BOOL VOID TRUE FALSE RETURN WHILE BREAK CONTINUE ID NUM NUM_B STRING SC COMMA
%token IF
%nonassoc ELSE
%right ASSIGN
%left OR
%left AND
%right NOT
%left GT LT GE LE
%left EQ NQ
%left ADD DEC
%left MULT DIV
%left LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE

%%


// While reducing the start variable, set the root of the AST
Program:  Funcs { program = $1; }
;

Funcs: {$$ = std::make_shared<ast::Funcs>(); }
    | FuncsDecl Funcs { std::dynamic_pointer_cast<ast::Funcs>($2)->push_front($1); $$ = $2;}
;

FuncsDecl: RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE {$$ = std::make_shared<ast::FuncDecl>($2, $1, $4, $7); }
;

RetType: Type {$$ = $1}
    | VOID {$$ = std::make_shared<ast::Type>(ast::BuiltInType::VOID); }
;

Formals: {{$$ = std::make_shared<ast::Formals>();}}
    | FormalsList {$$ = $1;}
;

FormalsList: FormalDecl {$$ = std::make_shared<ast::Formals>($1);}
    | FormalDecl COMMA FormalsList { std::dynamic_pointer_cast<ast::Formals>($3)->push_front($1); $$ = std::dynamic_pointer_cast<ast::Formals>($3);}
;

FormalDecl: Type ID {$$ = std::make_shared<ast::Formal>($2, $1); }
;

Statements: Statement {$$ = std::make_shared<ast::Statements>($1);}
    | Statements Statement {std::dynamic_pointer_cast<ast::Statements>($1)->push_back($2); $$ = std::dynamic_pointer_cast<ast::Statements>($1); }
;


Statement:
      MatchedStatement {$$ = $1;}
    | UnmatchedStatement {$$ = $1;}
    ;

MatchedStatement:
      RegularStatement {$$ = $1;}
    | IF LPAREN Exp RPAREN MatchedStatement ELSE MatchedStatement  {$$ = std::make_shared<ast::If>($3, $5, $7); }
    | WHILE LPAREN Exp RPAREN MatchedStatement  {$$ = std::make_shared<ast::While>($3, $5); }
    ;

UnmatchedStatement:
      IF LPAREN Exp RPAREN Statement {$$ = std::make_shared<ast::If>($3, $5); }
    | IF LPAREN Exp RPAREN MatchedStatement ELSE UnmatchedStatement {$$ = std::make_shared<ast::If>($3, $5, $7); }
    | WHILE LPAREN Exp RPAREN UnmatchedStatement  {$$ = std::make_shared<ast::While>($3, $5); }
    ;



RegularStatement: LBRACE Statements RBRACE {$$ = $2;}
     | Type ID SC {$$ = std::make_shared<ast::VarDecl>($2, $1); }
     | Type ID ASSIGN Exp SC {$$ = std::make_shared<ast::VarDecl>($2, $1, $4); }
     | ID ASSIGN Exp SC {$$ = std::make_shared<ast::Assign>($1, $3); }
     | Call SC {$$ = $1;}
     | RETURN SC {$$ = std::make_shared<ast::Return>(); }
     | RETURN Exp SC {$$ = std::make_shared<ast::Return>($2); }
     | BREAK SC {$$ = std::make_shared<ast::Break>(); }
     | CONTINUE SC {$$ = std::make_shared<ast::Continue>(); }
;

Call: ID LPAREN ExpList RPAREN  {$$ = std::make_shared<ast::Call>($1, $3); }
    | ID LPAREN RPAREN {$$ = std::make_shared<ast::Call>($1); }
;

ExpList: Exp {$$ = std::make_shared<ast::ExpList>($1); }
    | Exp COMMA ExpList  {std::dynamic_pointer_cast<ast::ExpList>($3)->push_front($1); $$ = std::dynamic_pointer_cast<ast::ExpList>($3); }
;

Type: INT {$$ = std::make_shared<ast::Type>(ast::BuiltInType::INT);}
    | BYTE {$$ = std::make_shared<ast::Type>(ast::BuiltInType::BYTE);}
    | BOOL {$$ = std::make_shared<ast::Type>(ast::BuiltInType::BOOL);}
;

Exp: LPAREN Exp RPAREN {$$ = $2; }
    | Exp ADD Exp {$$ = std::make_shared<ast::BinOp>($1, $3, ast::BinOpType::ADD); }
    | Exp DEC Exp {$$ = std::make_shared<ast::BinOp>($1, $3, ast::BinOpType::SUB);}
    | Exp MULT Exp {$$ = std::make_shared<ast::BinOp>($1, $3, ast::BinOpType::MUL); }
    | Exp DIV Exp {$$ = std::make_shared<ast::BinOp>($1, $3, ast::BinOpType::DIV); }
    | ID {$$ = $1;}
    | Call {$$ = $1;}
    | NUM {$$ = $1;}
    | NUM_B {$$ = $1;}
    | STRING {$$ = $1;}
    | TRUE {$$ = std::make_shared<ast::Bool>(true); }
    | FALSE {$$ = std::make_shared<ast::Bool>(false);}
    | NOT Exp {$$ = std::make_shared<ast::Not>($2); }
    | Exp AND Exp {$$ = std::make_shared<ast::And>($1, $3); }
    | Exp OR Exp {$$ = std::make_shared<ast::Or>($1, $3); }
    | Exp EQ Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::EQ); }
    | Exp NQ Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::NE); }
    | Exp GT Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::GT); }
    | Exp GE Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::GE); }
    | Exp LT Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::LT); }
    | Exp LE Exp {$$ = std::make_shared<ast::RelOp>($1, $3, ast::RelOpType::LE); }
    | LPAREN Type RPAREN Exp {$$ = std::make_shared<ast::Cast>($4, $2);}
;



%%

void yyerror(const char*) {
    output::errorSyn(yylineno);
    exit(0);
}
