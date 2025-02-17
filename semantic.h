#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef enum { TIPO_INT, TIPO_VOID, TIPO_INDEFINIDO } Tipo;

typedef struct simbolo {
    char *nome;
    Tipo tipo;
    int isArray;
    struct simbolo *prox;
} Simbolo;

typedef struct scope {
    Simbolo* simbolos;
    struct scope* prox;
} Scope;

/* Estrutura para armazenar a lista de símbolos locais de cada função */
typedef struct function_symbols {
    char* func_name;     
    Simbolo* local_symbols; 
    struct function_symbols* prox;
} FunctionSymbols;

/* Demais funções de gerenciamento e análise */
void push_scope();
void pop_scope();
void add_simbolo_scoped(const char* nome, Tipo tipo, int isArray);
Simbolo* lookup_simbolo_scoped(const char* nome);
void free_all_scopes();
void print_tabela_simbolos();
Tipo converter_tipo(const char* tipo_str);
Tipo verificar_semantica(ast* no);
void analise_semantica(ast* raiz);

#endif
