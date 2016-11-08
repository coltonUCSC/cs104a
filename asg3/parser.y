%{
// Dummy parser for scanner project.
// Colton Willey
// cwwilley@ucsc.edu
//
// Jacob Janowski
// jnjanows@ucsc.edu

#include <cassert>

#include "lyutils.h"
#include "astree.h"


%}

%initial-action {
   parser::root = new astree(TOK_ROOT, location{0,0,0}, "");}

%debug
%defines
%error-verbose
%token-table
%verbose

%start start

%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING TOK_BOOL
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY TOK_FALSE TOK_TRUE
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL 
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT TOK_PARAM TOK_DECLID
%token TOK_VARDECL TOK_RETURNVOID TOK_NEWSTRING TOK_INDEX
%token TOK_PROTOTYPE TOK_FUNCTION

%right TOK_IFELSE TOK_IF TOK_ELSE
%right '=' 
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left '+' '-'
%left '*' '/' '%'
%right TOK_CHR TOK_POS TOK_NEG '!'
%left TOK_ARRAY TOK_FIELD TOK_FUNCTION '.' '['
%nonassoc TOK_PARENS

%%
start     : program             { parser::root = $1; }
          ;
program   : program structdef   { $$ = $1->adopt($2); }
          | program function    { $$ = $1->adopt($2); }
          | program statement   { $$ = $1->adopt($2); }
          | program error '}'   { $$ = $1; }
          | program error ';'   { $$ = $1; }
          |                     { $$ = new astree(TOK_ROOT,
                                           location{0,0,0},""); }
          ;
structdef :  strdefs '}'                  { $$ = $1; }
          ;
strdefs   : strdefs fielddecl ';'         { destroy($3); 
                                            $$ = $1->adopt($2); }
          | TOK_STRUCT TOK_IDENT '{'
                              { destroy($3); 
                                $$ = $1->adopt($2->sym(TOK_TYPEID)); }
          ;
fielddecl : basetype TOK_ARRAY TOK_IDENT   { $2->sym(TOK_FIELD);
                                             $$ = $2->adopt($1); }
          | basetype TOK_IDENT             { $1->sym(TOK_FIELD);
                                             $$ = $1->adopt($2); }
          ; 
function  : identdecl param ')' block { destroy($3);
                                        $$ = new_func($1,$2,$4); }
          ;
param     : param ',' identdecl  { destroy($2); $$ = $1->adopt($3); }
          | '(' identdecl        { $1->sym(TOK_PARAM);
                                   $$ = $1->adopt($2); }
          | '('                  { $1->sym(TOK_PARAM); $$ = $1; }
          ;
identdecl : basetype TOK_ARRAY TOK_IDENT 
                               { $$ = $2->adopt($1, $3); }
          | basetype TOK_IDENT { $$ = $1->adopt($2->sym(TOK_DECLID));}
          ;
basetype  : TOK_IDENT           { $$ = $1->sym(TOK_TYPEID); }      
          | TOK_STRING          { $$ = $1; }              
          | TOK_INT             { $$ = $1; }              
          | TOK_CHAR            { $$ = $1; }                
          | TOK_BOOL            { $$ = $1; }                
          | TOK_VOID            { $$ = $1; }                
          ;
block     : stmts '}'    { destroy($2); $1->sym(TOK_BLOCK); $$ = $1; }
          | '{' '}'      { destroy($2); $$ = $1->sym(TOK_BLOCK); }
          | ';'          { $$ = $1->sym(TOK_BLOCK);}
          ;
stmts     : stmts statement     { $$ = $1->adopt($2); }
          | '{' statement       { $$ = $1->adopt($2); }
          ;
statement : block               { $$ = $1; }        
          | vardecl             { $$ = $1; }                    
          | while               { $$ = $1; }                          
          | ifelse              { $$ = $1; }                          
          | return              { $$ = $1; }                            
          | expr ';'            { destroy($2); $$ = $1;}             
          ;
vardecl   : identdecl '=' expr ';'   { $2->sym(TOK_VARDECL);
                                     destroy($4); 
                                     $$ = $2->adopt($1,$3); }
          ;
while     : TOK_WHILE '(' expr ')' statement { destroy($2, $4); 
                                             $$ = $1->adopt($3, $5); }
          ;
ifelse    : TOK_IF '(' expr ')' statement TOK_ELSE statement            
                                      { destroy($2, $4);
                                        $1->sym(TOK_IFELSE); 
                                        $$ = $1->adopt($3, $5, $7); }
          | TOK_IF '(' expr ')' statement %prec TOK_ELSE 
                                          { destroy($2, $4); 
                                            $$ = $1->adopt($3, $5); }
          ;
return    : TOK_RETURN ';'        { destroy($2); 
                                    $$ = $1->sym(TOK_RETURNVOID); }
          | TOK_RETURN expr ';'   { destroy($3); 
                                    $$ = $1->adopt($2); }
          ;

expr      : BINOP                { $$ = $1; }         
          | UNOP                 { $$ = $1; }
          | allocator            { $$ = $1; }   
          | call                 { $$ = $1; } 
          | '(' expr ')'         { destroy($1, $3); $$ = $2; } 
          | variable             { $$ = $1; } 
          | constant             { $$ = $1; }     
          ;
BINOP     : expr TOK_EQ expr              { $$ = $2->adopt($1, $3); }
          | expr TOK_NE expr              { $$ = $2->adopt($1, $3); }
          | expr TOK_LT expr              { $$ = $2->adopt($1, $3); }
          | expr TOK_LE expr              { $$ = $2->adopt($1, $3); }
          | expr TOK_GT expr              { $$ = $2->adopt($1, $3); }
          | expr TOK_GE expr              { $$ = $2->adopt($1, $3); }
          | expr '+' expr                 { $$ = $2->adopt($1, $3); }
          | expr '-' expr                 { $$ = $2->adopt($1, $3); }
          | expr '*' expr                 { $$ = $2->adopt($1, $3); }
          | expr '/' expr                 { $$ = $2->adopt($1, $3); }
          | expr '=' expr                 { $$ = $2->adopt($1, $3); }
          ;
UNOP      : '!' expr                        { $$ = $1->adopt($2); }
          | TOK_NEG expr                    { $$ = $1->adopt($2); }
          | TOK_POS expr                    { $$ = $1->adopt($2); }
          ; 
allocator : TOK_NEW TOK_IDENT '(' ')'       { destroy($3, $4); 
                                              $2->sym(TOK_TYPEID);
                                              $$ = $1->adopt($2); }
          | TOK_NEW TOK_STRING '(' expr ')' { destroy($2, $3, $5);
                                              $1->sym(TOK_NEWSTRING);
                                              $$ = $1->adopt($4); }
          | TOK_NEW basetype '[' expr ']'  { destroy($3, $5); 
                                             $1->sym(TOK_NEWARRAY); 
                                             $$ = $1->adopt($2, $4); }
          ;
call      : TOK_IDENT '(' ')'                { destroy($3); 
                                               $2->sym(TOK_CALL);
                                               $$ = $2->adopt($1); }
          | TOK_IDENT rexpr ')'              { destroy($2); 
                                               $$ = $1; }
          ;
rexpr     : rexpr ',' expr       { destroy($2); $$ = $1->adopt($3); }
          | '(' expr             { $1->sym(TOK_CALL);
                                   $$ = $1->adopt($2); }
          ;
variable:   TOK_IDENT         { $$ = $1; }
          | expr '.' TOK_IDENT
                          { $$ = $2->adopt($1, $3->sym(TOK_FIELD)); }
          | expr '[' expr ']' 
                        { $2->sym(TOK_INDEX); $$ = $2->adopt($1, $3);
                                destroy($4); }
          ;
constant:   TOK_INTCON          { $$ = $1; }
          | TOK_CHARCON         { $$ = $1; }          
          | TOK_STRINGCON       { $$ = $1; }    
          | TOK_NULL            { $$ = $1; }
          ;
%%

const char *parser::get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

/*
static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != nullptr);
   return result;
}
*/

