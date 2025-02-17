%code requires {
  #include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "parser.tab.h"
#include "semantic.h"

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

%type <node> programa lista_declaracoes_globais lista_declaracoes_funcoes nome_funcao lista_argumentos declaracao_variavel declaracao_funcao especificador_tipo parametros lista_parametros parametro comando_composto declaracoes_locais lista_comandos comando comando_expressao comando_selecao comando_iteracao comando_retorno expressao var expressao_simples operador_relacional expressao_aditiva expressao_multiplicativa fator

%%

/* Programa: declarações globais e de funções */
programa:
      lista_declaracoes_globais lista_declaracoes_funcoes {
          $$ = new_ast("programa", "", 2, $1, $2); 
          ast_root = $$;
      }
    | lista_declaracoes_globais {
          $$ = new_ast("programa", "", 1, $1); 
          ast_root = $$;
      }
    | lista_declaracoes_funcoes {
          $$ = new_ast("programa", "", 1, $1); 
          ast_root = $$;
      }
    ;

/* Declarações globais */
lista_declaracoes_globais:
      lista_declaracoes_globais declaracao_variavel {
          $$ = new_ast("lista_declaracoes_globais", "", 2, $1, $2);
      }
    | declaracao_variavel {
          $$ = new_ast("lista_declaracoes_globais", "", 1, $1);
      }
    ;

/* Declarações de funções */
lista_declaracoes_funcoes:
      lista_declaracoes_funcoes declaracao_funcao {
          $$ = new_ast("lista_declaracoes_funcoes", "", 2, $1, $2);
      }
    | declaracao_funcao {
          $$ = new_ast("lista_declaracoes_funcoes", "", 1, $1);
      }
    ;

/* Nome da função */
nome_funcao:
      ID { $$ = new_ast("nome_funcao", $1, 0); }
    ;

/* Declaração de variável */
declaracao_variavel:
      especificador_tipo ID PEV {
          $$ = new_ast("declaracao_variavel", "", 2, $1, new_ast("ID", $2, 0));
      }
    | especificador_tipo ID IGUAL expressao PEV {
          $$ = new_ast("declaracao_variavel", "atribuicao", 3, $1, new_ast("ID", $2, 0), $4);
      }
    | especificador_tipo ID ABRE_COL NUM FECHA_COL PEV {
          char tamanho[16];
          sprintf(tamanho, "%d", $4);
          $$ = new_ast("declaracao_variavel", "vetor", 3, $1, new_ast("ID", $2, 0), new_ast("NUM", tamanho, 0));
      }
    | especificador_tipo ID ABRE_COL FECHA_COL PEV {
          $$ = new_ast("declaracao_variavel", "vetor", 2, $1, new_ast("ID", $2, 0));
      }
    ;

/* Declaração de função */
declaracao_funcao:
      especificador_tipo nome_funcao ABRE_PAREN parametros FECHA_PAREN comando_composto {
          $$ = new_ast("declaracao_funcao", "", 4, $1, $2, $4, $6);
      }
    ;

/* Especificador de tipo */
especificador_tipo:
      INT { $$ = new_ast("especificador_tipo", "int", 0); }
    | VOID { $$ = new_ast("especificador_tipo", "void", 0); }
    ;

/* Parâmetros da função */
parametros:
      lista_parametros { $$ = new_ast("parametros", "", 1, $1); }
    | VOID { $$ = new_ast("parametros", "void", 0); }
    | /* vazio */ { $$ = new_ast("parametros", "vazio", 0); }
    ;

/* Lista de parâmetros */
lista_parametros:
      parametro { $$ = new_ast("lista_parametros", "", 1, $1); }
    | lista_parametros VIRGULA parametro {
          $$ = new_ast("lista_parametros", "", 2, $1, $3);
      }
    ;

/* Parâmetro: variável ou vetor */
parametro:
      especificador_tipo ID {
          $$ = new_ast("parametro", "", 2, $1, new_ast("ID", $2, 0));
      }
    | especificador_tipo ID ABRE_COL FECHA_COL {
          $$ = new_ast("parametro", "vetor", 2, $1, new_ast("ID", $2, 0));
      }
    ;

/* Comando composto (bloco) */
comando_composto:
      ABRE_CHAVES declaracoes_locais lista_comandos FECHA_CHAVES {
          $$ = new_ast("comando_composto", "", 2, $2, $3);
      }
    ;

/* Declarações locais */
declaracoes_locais:
      /* vazio */ { $$ = new_ast("declaracoes_locais", "vazio", 0); }
    | declaracoes_locais declaracao_variavel {
          $$ = new_ast("declaracoes_locais", "", 2, $1, $2);
      }
    ;

/* Lista de comandos */
lista_comandos:
      /* vazio */ { $$ = new_ast("lista_comandos", "vazio", 0); }
    | lista_comandos comando {
          $$ = new_ast("lista_comandos", "", 2, $1, $2);
      }
    ;

/* Comando */
comando:
      comando_expressao { $$ = new_ast("comando", "", 1, $1); }
    | comando_composto { $$ = new_ast("comando", "", 1, $1); }
    | comando_selecao { $$ = new_ast("comando", "", 1, $1); }
    | comando_iteracao { $$ = new_ast("comando", "", 1, $1); }
    | comando_retorno { $$ = new_ast("comando", "", 1, $1); }
    ;

/* Comando de expressão */
comando_expressao:
      expressao PEV { $$ = new_ast("comando_expressao", "", 1, $1); }
    | PEV { $$ = new_ast("comando_expressao", "vazio", 0); }
    ;

/* Comando de seleção */
comando_selecao:
      IF ABRE_PAREN expressao FECHA_PAREN comando %prec LOWER_THAN_ELSE {
          $$ = new_ast("comando_selecao", "se", 2, $3, $5);
      }
    | IF ABRE_PAREN expressao FECHA_PAREN comando ELSE comando {
          $$ = new_ast("comando_selecao", "se-senao", 3, $3, $5, $7);
      }
    ;

/* Comando de iteração */
comando_iteracao:
      WHILE ABRE_PAREN expressao FECHA_PAREN comando {
          $$ = new_ast("comando_iteracao", "enquanto", 2, $3, $5);
      }
    ;

/* Comando de retorno */
comando_retorno:
      RETURN PEV { $$ = new_ast("comando_retorno", "retorno", 0); }
    | RETURN expressao PEV { $$ = new_ast("comando_retorno", "retorno", 1, $2); }
    ;

/* Lista de argumentos para chamada de função */
lista_argumentos:
      expressao { $$ = new_ast("lista_argumentos", "", 1, $1); }
    | lista_argumentos VIRGULA expressao {
          $$ = new_ast("lista_argumentos", "", 2, $1, $3);
      }
    | /* vazio */ { $$ = new_ast("lista_argumentos", "vazio", 0); }
    ;

/* Expressão */
expressao:
      var IGUAL expressao {
          $$ = new_ast("expressao", "atribuicao", 2, $1, $3);
      }
    | expressao_simples { $$ = $1; }
    | ID ABRE_PAREN lista_argumentos FECHA_PAREN {
          $$ = new_ast("chamada_funcao", "", 2, new_ast("ID", $1, 0), $3);
      }
    ;

/* Variável: acesso a vetor ou variável simples */
var:
    ID ABRE_COL expressao FECHA_COL { 
         /* Cria nó para acesso a vetor, com valor igual ao identificador */
         $$ = new_ast("array_access", $1, 2, new_ast("ID", $1, 0), $3); 
    }
  | ID { 
         $$ = new_ast("var", $1, 0); 
    }
  ;


/* Expressão simples */
expressao_simples:
      expressao_aditiva { $$ = new_ast("expressao_simples", "", 1, $1); }
    | expressao_aditiva operador_relacional expressao_aditiva {
          $$ = new_ast("expressao_simples", "operador_relacional", 3, $1, $2, $3);
      }
    ;

/* Operador relacional */
operador_relacional:
      IGUALDADE { $$ = new_ast("operador_relacional", "==", 0); }
    | MENOR_IGL { $$ = new_ast("operador_relacional", "<=", 0); }
    | MAIOR_IGL { $$ = new_ast("operador_relacional", ">=", 0); }
    | MENOR { $$ = new_ast("operador_relacional", "<", 0); }
    | MAIOR { $$ = new_ast("operador_relacional", ">", 0); }
    | DIFERENTE { $$ = new_ast("operador_relacional", "!=", 0); }
    ;

/* Expressão aditiva */
expressao_aditiva:
      expressao_aditiva SOMA expressao_multiplicativa {
          $$ = new_ast("expressao_aditiva", "+", 2, $1, $3);
      }
    | expressao_aditiva SUB expressao_multiplicativa {
          $$ = new_ast("expressao_aditiva", "-", 2, $1, $3);
      }
    | expressao_multiplicativa { $$ = new_ast("expressao_aditiva", "", 1, $1); }
    ;

/* Expressão multiplicativa */
expressao_multiplicativa:
      expressao_multiplicativa MULT fator {
          $$ = new_ast("expressao_multiplicativa", "*", 2, $1, $3);
      }
    | expressao_multiplicativa DIV fator {
          $$ = new_ast("expressao_multiplicativa", "/", 2, $1, $3);
      }
    | fator { $$ = new_ast("expressao_multiplicativa", "", 1, $1); }
    ;

/* Fator */
fator:
      ABRE_PAREN expressao FECHA_PAREN {
          $$ = new_ast("fator", "()", 1, $2);
      }
    | var { $$ = new_ast("fator", "", 1, $1); }
    | NUM {
          char buf[32];
          sprintf(buf, "%d", $1);
          $$ = new_ast("fator", buf, 0);
      }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro na linha %d: %s\n", line_number, s);
    exit(1);
}

int main() {
    printf("Iniciando a análise sintática...\n");
    if (yyparse() == 0) {
        printf("Análise sintática concluída com sucesso!\n");
        print_ast(ast_root, 0);
        analise_semantica(ast_root);
        free_ast(ast_root);
    } else {
        fprintf(stderr, "Erro durante a análise sintática.\n");
    }
    return 0;
}
