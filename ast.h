#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct ast {
    char *name;
    char *value;
    struct ast **children;
    int n_children;
} ast;

ast* new_ast(char* name, char* value, int n, ...);
void print_ast(ast* root, int indent);
void free_ast(ast* root);



#endif 
