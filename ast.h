#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Estrutura da Árvore Sintática Abstrata (AST) */
typedef struct ast {
    char *name;
    char *value;
    struct ast **children;
    int n_children;
} ast;

/* Funções para manipulação da AST */
ast* new_ast(char* name, char* value, int n, ...);
void print_ast(ast* root, int indent);
void free_ast(ast* root);

/* 🔥 Correção: Adicionando funções ausentes */
ast* criar_programa(ast* declaracoes_globais, ast* declaracoes_funcoes);
ast* criar_lista_comandos(ast* novo_comando, ast* comandos_existentes);

#endif // AST_H
