%option noinput nounput


%{
#include "parser.tab.h"  // Para os tokens gerados pelo Bison
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern YYSTYPE yylval;
int line_number = 1;
%}

%%

"if"           { return IF; }
"else"         { return ELSE; }
"while"        { return WHILE; }
"int"          { return INT; }
"return"       { return RETURN; }
"void"         { return VOID; }
"main"          { return MAIN; }


[a-zA-Z_][a-zA-Z0-9_]*  { 
    yylval.str = strdup(yytext); 
    return ID; 
}

[0-9]+      { 
    yylval.intval = atoi(yytext); 
    return NUM; 
}

"=="        { return IGUALDADE; }
"<="        { return MENOR_IGL; }
">="        { return MAIOR_IGL; }
"<"         { return MENOR; }
">"         { return MAIOR; }
"!="        { return DIFERENTE; }
"="         { return IGUAL; }


"+"         { return SOMA; }
"-"         { return SUB; }
"*"         { return MULT; }
"/"         { return DIV; }

"("         { return ABRE_PAREN; }
")"         { return FECHA_PAREN; }
"{"         { return ABRE_CHAVES; }
"}"         { return FECHA_CHAVES; }
"["         { return ABRE_COL; }
"]"         { return FECHA_COL; }
";"         { return PEV; }
","         { return VIRGULA; }

"//".*      { /* Ignorar comentários de linha */ }

"/*"([^*]|\*+[^*/])*\*+"/"  { 
    for (int i = 0; yytext[i] != '\0'; i++) {
        if (yytext[i] == '\n') line_number++;
    }
}

[ \t]+      { /* Ignorar */ }

\n          { line_number++; }

.           { return ERRO; }

%%

int yywrap(void) {
    return 1;
}
