/* $Id: lexer.l,v 1.6 2016-10-20 13:48:57-07 - - $ */

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
NUMBER          {DIGIT}+
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
CHAR ('([^\\'\n]|\\[\\'"0nt])')
STRING (\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"char"         { return yylval_token (TOK_CHAR);   }
"else"         { return yylval_token (TOK_ELSE);   }
"if"           { return yylval_token (TOK_IF);     }
"int"          { return yylval_token (TOK_INT);    }
"new"          { return yylval_token (TOK_NEW);    }
"null"         { return yylval_token (TOK_NULL);   }
"return"       { return yylval_token (TOK_RETURN); }
"string"       { return yylval_token (TOK_STRING); }
"struct"       { return yylval_token (TOK_STRUCT); }
"void"         { return yylval_token (TOK_VOID);   }
"while"        { return yylval_token (TOK_WHILE);  }

"!="            { return yylval_token (TOK_NE); }
"!"             { return yylval_token ('!');    }
"%"             { return yylval_token ('%');    } 
"("             { return yylval_token ('(');    }
")"             { return yylval_token (')');    } 
"*"             { return yylval_token ('*');    } 
"+"             { return yylval_token ('+');    } 
","             { return yylval_token (',');    } 
"-"             { return yylval_token ('-');    } 
"."             { return yylval_token ('.');    } 
"/"             { return yylval_token ('/');    } 
";"             { return yylval_token (';');    } 
"<="            { return yylval_token (TOK_LE); }
"<"             { return yylval_token (TOK_LT); } 
"="             { return yylval_token ('=');    } 
"=="            { return yylval_token (TOK_EQ); }
">="            { return yylval_token (TOK_GE); }
">"             { return yylval_token (TOK_GT); } 
"["             { return yylval_token ('[');    } 
"[]"            { return yylval_token (TOK_ARRAY);}
"]"             { return yylval_token (']');    } 
"{"             { return yylval_token ('{');    } 
"}"             { return yylval_token ('}');    }
"^"             { return yylval_token ('^'); }

{NUMBER}        { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
{CHAR}          { return yylval_token (TOK_CHARCON); }
{STRING}        { return yylval_token (TOK_STRINGCON); }

.               { lexer::badchar (*yytext); }

%%

