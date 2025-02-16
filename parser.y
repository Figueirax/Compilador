%code requires {
  #include "ast.h"
}


%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "parser.tab.h"

/* Ponteiro global para a raiz da AST */
ast* ast_root = NULL;

extern int yylex();
extern int line_number;
void yyerror(const char *s);
%}

%union {
    int intval;
    char* str;
    ast* node;
}

%token ERRO
%token IF ELSE WHILE INT RETURN VOID
%token <str> ID
%token <intval> NUM

%token IGUALDADE MENOR_IGL MAIOR_IGL MENOR MAIOR DIFERENTE IGUAL
%token SOMA SUB MULT DIV
%token ABRE_PAREN FECHA_PAREN ABRE_CHAVES FECHA_CHAVES ABRE_COL FECHA_COL
%token PEV VIRGULA

%left SOMA SUB
%left MULT DIV
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <node> programa lista_declaracoes declaracao declaracao_variavel declaracao_funcao especificador_tipo parametros lista_parametros parametro comando_composto declaracoes_locais lista_comandos comando comando_expressao comando_selecao comando_iteracao comando_retorno expressao var expressao_simples operador_relacional expressao_aditiva expressao_multiplicativa fator
%%

programa:
      lista_declaracoes { $$ = new_ast("programa", "", 1, $1); ast_root = $$; }
    ;

lista_declaracoes:
      lista_declaracoes declaracao { $$ = new_ast("lista_declaracoes", "", 2, $1, $2); }
    | declaracao { $$ = new_ast("lista_declaracoes", "", 1, $1); }
    ;

declaracao:
      declaracao_variavel { $$ = new_ast("declaracao", "", 1, $1); }
    | declaracao_funcao { $$ = new_ast("declaracao", "", 1, $1); }
    ;

declaracao_variavel:
      especificador_tipo ID PEV { $$ = new_ast("declaracao_variavel", "", 2, $1, new_ast("ID", $2, 0)); }
    | especificador_tipo ID IGUAL expressao PEV { $$ = new_ast("declaracao_variavel", "", 3, $1, new_ast("ID", $2, 0), $4); }
    ;

declaracao_funcao:
      especificador_tipo ID ABRE_PAREN parametros FECHA_PAREN comando_composto {
          $$ = new_ast("declaracao_funcao", "", 4, $1, new_ast("ID", $2, 0), $4, $6);
      }
    ;

especificador_tipo:
      INT { $$ = new_ast("especificador_tipo", "int", 0); }
    | VOID { $$ = new_ast("especificador_tipo", "void", 0); }
    ;

parametros:
      lista_parametros { $$ = new_ast("parametros", "", 1, $1); }
    | /* vazio */ { $$ = new_ast("parametros", "void", 0); }
    ;

lista_parametros:
      parametro { $$ = new_ast("lista_parametros", "", 1, $1); }
    | lista_parametros VIRGULA parametro { $$ = new_ast("lista_parametros", "", 2, $1, $3); }
    ;

parametro:
      especificador_tipo ID { $$ = new_ast("parametro", "", 2, $1, new_ast("ID", $2, 0)); }
    ;

comando_composto:
      ABRE_CHAVES declaracoes_locais lista_comandos FECHA_CHAVES { $$ = new_ast("comando_composto", "", 2, $2, $3); }
    ;

declaracoes_locais:
      /* vazio */ { $$ = new_ast("declaracoes_locais", "vazio", 0); }
    | declaracoes_locais declaracao_variavel { $$ = new_ast("declaracoes_locais", "", 2, $1, $2); }
    ;

lista_comandos:
      /* vazio */ { $$ = new_ast("lista_comandos", "vazio", 0); }
    | lista_comandos comando { $$ = new_ast("lista_comandos", "", 2, $1, $2); }
    ;

comando:
      comando_expressao { $$ = new_ast("comando", "", 1, $1); }
    | comando_composto { $$ = new_ast("comando", "", 1, $1); }
    | comando_selecao { $$ = new_ast("comando", "", 1, $1); }
    | comando_iteracao { $$ = new_ast("comando", "", 1, $1); }
    | comando_retorno { $$ = new_ast("comando", "", 1, $1); }
    ;

comando_expressao:
      expressao PEV { $$ = new_ast("comando_expressao", "", 1, $1); }
    | PEV { $$ = new_ast("comando_expressao", "vazio", 0); }
    ;

comando_selecao:
      IF ABRE_PAREN expressao FECHA_PAREN comando %prec LOWER_THAN_ELSE { $$ = new_ast("comando_selecao", "se", 2, $3, $5); }
    | IF ABRE_PAREN expressao FECHA_PAREN comando ELSE comando { $$ = new_ast("comando_selecao", "se-senao", 3, $3, $5, $7); }
    ;

comando_iteracao:
      WHILE ABRE_PAREN expressao FECHA_PAREN comando { $$ = new_ast("comando_iteracao", "enquanto", 2, $3, $5); }
    ;

comando_retorno:
      RETURN PEV { $$ = new_ast("comando_retorno", "retorno", 0); }
    | RETURN expressao PEV { $$ = new_ast("comando_retorno", "retorno", 1, $2); }
    ;

expressao:
      var IGUAL expressao { $$ = new_ast("expressao", "atribuicao", 2, $1, $3); }
    | expressao_simples { $$ = $1; }
    ;

var:
      ID { $$ = new_ast("var", $1, 0); }
    ;

expressao_simples:
      expressao_aditiva { $$ = new_ast("expressao_simples", "", 1, $1); }
    | expressao_aditiva operador_relacional expressao_aditiva { $$ = new_ast("expressao_simples", "operador_relacional", 3, $1, $2, $3); }
    ;

operador_relacional:
      IGUALDADE { $$ = new_ast("operador_relacional", "==", 0); }
    | MENOR_IGL { $$ = new_ast("operador_relacional", "<=", 0); }
    | MAIOR_IGL { $$ = new_ast("operador_relacional", ">=", 0); }
    | MENOR { $$ = new_ast("operador_relacional", "<", 0); }
    | MAIOR { $$ = new_ast("operador_relacional", ">", 0); }
    | DIFERENTE { $$ = new_ast("operador_relacional", "!=", 0); }
    ;

expressao_aditiva:
      expressao_aditiva SOMA expressao_multiplicativa { $$ = new_ast("expressao_aditiva", "+", 2, $1, $3); }
    | expressao_aditiva SUB expressao_multiplicativa { $$ = new_ast("expressao_aditiva", "-", 2, $1, $3); }
    | expressao_multiplicativa { $$ = new_ast("expressao_aditiva", "", 1, $1); }
    ;

expressao_multiplicativa:
      expressao_multiplicativa MULT fator { $$ = new_ast("expressao_multiplicativa", "*", 2, $1, $3); }
    | expressao_multiplicativa DIV fator { $$ = new_ast("expressao_multiplicativa", "/", 2, $1, $3); }
    | fator { $$ = new_ast("expressao_multiplicativa", "", 1, $1); }
    ;

fator:
      ABRE_PAREN expressao FECHA_PAREN { $$ = new_ast("fator", "()", 1, $2); }
    | var { $$ = new_ast("fator", "", 1, $1); }
    | NUM { char buf[32]; sprintf(buf, "%d", $1); $$ = new_ast("fator", buf, 0); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro na linha %d: %s\n", line_number, s);
    exit(1);
}

int main() {
    printf("Iniciando a análise sintática...\n");
    if(yyparse() == 0) {
        printf("Análise concluída com sucesso!\n");
        print_ast(ast_root, 0);
        free_ast(ast_root);
    } else {
        fprintf(stderr, "Erro durante a análise sintática.\n");
    }
    return 0;
}