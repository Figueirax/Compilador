#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Gerenciamento de Escopos e Tabela de Símbolos */

/* Ponteiro para o escopo corrente (inicia com NULL) */
Scope* current_scope = NULL;

/* Lista global para guardar os símbolos locais de cada função */
static FunctionSymbols* function_symbols_list = NULL;

/* Cria um novo escopo e o empilha */
void push_scope() {
    Scope* novo = malloc(sizeof(Scope));
    if (!novo) { perror("malloc"); exit(1); }
    novo->simbolos = NULL;
    novo->prox = current_scope;
    current_scope = novo;
}

/* Remove o escopo corrente */
void pop_scope() {
    if (current_scope) {
        Scope* temp = current_scope;
        current_scope = current_scope->prox;
        free(temp);
    }
}

void free_all_scopes() {
    while (current_scope) {
        pop_scope();
    }
}

/* Adiciona um símbolo no escopo corrente */
void add_simbolo_scoped(const char* nome, Tipo tipo, int isArray) {
    if (!current_scope) push_scope();
    Simbolo* sim = current_scope->simbolos;
    while (sim) {
        if (strcmp(sim->nome, nome) == 0) {
            fprintf(stderr, "Erro semântico: '%s' já declarada neste escopo.\n", nome);
            return;
        }
        sim = sim->prox;
    }
    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) { perror("malloc"); exit(1); }
    novo->nome = strdup(nome);
    novo->tipo = tipo;
    novo->isArray = isArray;
    novo->prox = current_scope->simbolos;
    current_scope->simbolos = novo;
}

/* Procura um símbolo a partir do escopo corrente (percorrendo os pais) */
Simbolo* lookup_simbolo_scoped(const char* nome) {
    for (Scope* s = current_scope; s; s = s->prox) {
        for (Simbolo* sim = s->simbolos; sim; sim = sim->prox) {
            if (strcmp(sim->nome, nome) == 0)
                return sim;
        }
    }
    return NULL;
}

/* Imprime a tabela de símbolos do escopo corrente */
void print_tabela_simbolos() {
    if (!current_scope) {
        printf("Nenhum escopo ativo.\n");
        return;
    }
    for (Simbolo* sim = current_scope->simbolos; sim != NULL; sim = sim->prox) {
        printf("Nome: %-10s | Tipo: %-8s%s\n", 
               sim->nome,
               (sim->tipo == TIPO_INT ? "int" :
                (sim->tipo == TIPO_VOID ? "void" : "indefinido")),
               (sim->isArray ? " [vetor]" : ""));
    }
    printf("-------------------------------------------\n");
}

/* Copia recursivamente a lista de símbolos */
static Simbolo* copy_symbols_list(Simbolo* original) {
    if (!original) return NULL;
    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) { perror("malloc"); exit(1); }
    novo->nome = strdup(original->nome);
    novo->tipo = original->tipo;
    novo->isArray = original->isArray;
    novo->prox = copy_symbols_list(original->prox);
    return novo;
}

/* Armazena a cópia dos símbolos locais de uma função na lista global */
static void store_local_symbols(const char* function_name, Simbolo* local_symbols) {
    FunctionSymbols* fs = malloc(sizeof(FunctionSymbols));
    if (!fs) { perror("malloc"); exit(1); }
    fs->func_name = strdup(function_name);
    fs->local_symbols = copy_symbols_list(local_symbols);
    fs->prox = function_symbols_list;
    function_symbols_list = fs;
}

/* Imprime os símbolos locais de todas as funções */
static void print_all_function_symbols() {
    for (FunctionSymbols* fs = function_symbols_list; fs != NULL; fs = fs->prox) {
        printf("\n--- Símbolos locais da função '%s' ---\n", fs->func_name);
        for (Simbolo* sim = fs->local_symbols; sim != NULL; sim = sim->prox) {
            printf("Nome: %-10s | Tipo: %-8s%s\n", 
                   sim->nome,
                   (sim->tipo == TIPO_INT ? "int" :
                    (sim->tipo == TIPO_VOID ? "void" : "indefinido")),
                   (sim->isArray ? " [vetor]" : ""));
        }
        printf("---------------------------------------\n");
    }
}

/* Libera a lista global de símbolos locais */
static void free_function_symbols_list() {
    FunctionSymbols* fs = function_symbols_list;
    while (fs) {
        FunctionSymbols* temp = fs;
        fs = fs->prox;
        free(temp->func_name);
        for (Simbolo* s = temp->local_symbols; s; ) {
            Simbolo* aux = s;
            s = s->prox;
            free(aux->nome);
            free(aux);
        }
        free(temp);
    }
    function_symbols_list = NULL;
}

/* Análise Semântica */

/* Converte uma string de tipo para o enum Tipo */
Tipo converter_tipo(const char* tipo_str) {
    if (strcmp(tipo_str, "int") == 0)
        return TIPO_INT;
    else if (strcmp(tipo_str, "void") == 0)
        return TIPO_VOID;
    return TIPO_INDEFINIDO;
}

/* Percorre a AST e realiza a análise semântica */
Tipo verificar_semantica(ast* no) {
    if (no == NULL) return TIPO_INDEFINIDO;
    
    /* Literal numérico */
    if (strcmp(no->name, "fator") == 0 && no->value != NULL) {
        if (isdigit(no->value[0]))
            return TIPO_INT;
    }
    
    /* Declaração de variável */
    if (strcmp(no->name, "declaracao_variavel") == 0) {
        Tipo t = TIPO_INDEFINIDO;
        int isArray = 0;
        if (no->n_children > 0 && no->children[0] && no->children[0]->value)
            t = converter_tipo(no->children[0]->value);
        if (no->value && strcmp(no->value, "vetor") == 0)
            isArray = 1;
        if (no->n_children > 1 && no->children[1] && no->children[1]->value)
            add_simbolo_scoped(no->children[1]->value, t, isArray);
        return t;
    }
    
    /* Declaração de função */
    else if (strcmp(no->name, "declaracao_funcao") == 0) {
        Tipo t = TIPO_INDEFINIDO;
        if (no->n_children > 0 && no->children[0] && no->children[0]->value)
            t = converter_tipo(no->children[0]->value);
        if (no->n_children > 1 && no->children[1] && no->children[1]->value)
            add_simbolo_scoped(no->children[1]->value, t, 0);
        push_scope();
        for (int i = 2; i < no->n_children; i++) {
            verificar_semantica(no->children[i]);
        }
        store_local_symbols(no->children[1]->value, current_scope->simbolos);
        pop_scope();
        return t;
    }
    
    /* Tratamento de parâmetros */
    else if (strcmp(no->name, "parametro") == 0) {
        Tipo t = TIPO_INDEFINIDO;
        int isArray = 0;
        if (no->n_children > 0 && no->children[0] && no->children[0]->value)
            t = converter_tipo(no->children[0]->value);
        if (no->n_children > 1 && no->children[1] && no->children[1]->value) {
            if (no->value && strcmp(no->value, "vetor") == 0)
                isArray = 1;
            add_simbolo_scoped(no->children[1]->value, t, isArray);
        }
        return t;
    }
    
    /* Uso de variável */
    else if (strcmp(no->name, "var") == 0) {
        if (no->value) {
            Simbolo* sim = lookup_simbolo_scoped(no->value);
            if (sim == NULL) {
                fprintf(stderr, "Erro semântico: variável '%s' não declarada.\n", no->value);
                exit(1);
                return TIPO_INDEFINIDO;
                
            }
            return sim->tipo;
        }
    }
    
    /* Acesso a vetor */
    else if (strcmp(no->name, "array_access") == 0) {
        if (no->n_children >= 2 && no->children[0] && no->children[0]->value) {
            Simbolo* sim = lookup_simbolo_scoped(no->children[0]->value);
            if (!sim) {
                fprintf(stderr, "Erro semântico: variável '%s' não declarada.\n", no->children[0]->value);
                return TIPO_INDEFINIDO;
            }
            if (!sim->isArray) {
                fprintf(stderr, "Erro semântico: variável '%s' não é um vetor.\n", no->children[0]->value);
                return TIPO_INDEFINIDO;
            }
            Tipo t_index = verificar_semantica(no->children[1]);
            if (t_index != TIPO_INT) {
                fprintf(stderr, "Erro semântico: índice de vetor '%s' deve ser do tipo int.\n", no->children[0]->value);
                return TIPO_INDEFINIDO;
            }
            return sim->tipo;
        }
    }
    
    /* Atribuição */
    else if (strcmp(no->name, "expressao") == 0) {
        if (no->n_children >= 2) {
            Tipo t_esq = verificar_semantica(no->children[0]);
            Tipo t_dir = verificar_semantica(no->children[1]);
            if (t_esq != t_dir) {
                fprintf(stderr, "Erro semântico: incompatibilidade de tipos na atribuição de '%s'.\n",
                        no->children[0]->value);
                return TIPO_INDEFINIDO;
            }
            return t_esq;
        }
    }

    /* Tratamento de chamada de função */
    else if (strcmp(no->name, "chamada_funcao") == 0) {
        if (no->n_children >= 1 && no->children[0] && no->children[0]->value) {
            if (strcmp(no->children[0]->value, "input") == 0)
                return TIPO_INT;
            else if (strcmp(no->children[0]->value, "output") == 0)
                return TIPO_VOID;
            else {
                Simbolo* sim = lookup_simbolo_scoped(no->children[0]->value);
                if (!sim) {
                    fprintf(stderr, "Erro semântico: função '%s' não declarada.\n", no->children[0]->value);
                    return TIPO_INDEFINIDO;
                }
                return sim->tipo;
            }
        }
        return TIPO_INDEFINIDO;
    }

    
    /* Comando composto (bloco) */
    else if (strcmp(no->name, "comando_composto") == 0) {
        push_scope();
        for (int i = 0; i < no->n_children; i++) {
            verificar_semantica(no->children[i]);
        }
        Scope* inner_scope = current_scope;
        Scope* parent_scope = current_scope->prox;
        if (parent_scope != NULL) {
            Simbolo* sim = inner_scope->simbolos;
            while (sim) {
                Simbolo* next = sim->prox;
                sim->prox = parent_scope->simbolos;
                parent_scope->simbolos = sim;
                sim = next;
            }
        }
        pop_scope();
        return TIPO_INDEFINIDO;
    }
    
    /* Percorre os filhos por padrão */
    Tipo result = TIPO_INDEFINIDO;
    for (int i = 0; i < no->n_children; i++) {
        result = verificar_semantica(no->children[i]);
    }
    return result;
}

void analise_semantica(ast* raiz) {
    printf("Iniciando a análise semântica...\n");
    push_scope();
    verificar_semantica(raiz);

    printf("\nTabela de Símbolos (escopo global):\n");
    print_tabela_simbolos();
    pop_scope();
    
    print_all_function_symbols();
    free_function_symbols_list();
    free_all_scopes();
}
