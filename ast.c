#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

ast* new_ast(char* name, char* value, int n, ...) {
    ast *node = malloc(sizeof(ast));
    if (!node) { perror("malloc"); exit(1); }
    node->name = strdup(name);
    node->value = (value != NULL) ? strdup(value) : NULL;
    node->n_children = n;
    if (n > 0) {
        node->children = malloc(sizeof(ast*) * n);
        if (!node->children) { perror("malloc"); exit(1); }
        va_list ap;
        va_start(ap, n);
        for (int i = 0; i < n; i++) {
            node->children[i] = va_arg(ap, ast*);
        }
        va_end(ap);
    } else {
        node->children = NULL;
    }
    return node;
}

ast* criar_programa(ast* declaracoes_globais, ast* declaracoes_funcoes) {
    return new_ast("programa", "", 2, declaracoes_globais, declaracoes_funcoes);
}

/* Cria uma lista de comandos */
ast* criar_lista_comandos(ast* novo_comando, ast* comandos_existentes) {
    if (!comandos_existentes) {
        return new_ast("lista_comandos", "", 1, novo_comando);
    }
    return new_ast("lista_comandos", "", 2, comandos_existentes, novo_comando);
}

void print_ast(ast* root, int indent) {
    if (root == NULL) return;
    for (int i = 0; i < indent; i++)
        printf("  ");
    printf("%s", root->name);
    if (root->value)
        printf(": %s", root->value);
    printf("\n");
    for (int i = 0; i < root->n_children; i++) {
        print_ast(root->children[i], indent + 1);
    }
}

void free_ast(ast* root) {
    if (root == NULL) return;
    if (root->name) free(root->name);
    if (root->value) free(root->value);
    for (int i = 0; i < root->n_children; i++) {
        free_ast(root->children[i]);
    }
    if (root->children) free(root->children);
    free(root);
}
