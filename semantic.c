#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ================================
   Gerenciamento de Escopos e Tabela de Símbolos
   ================================ */

/* Estrutura para um símbolo (variável ou função) */
/* (Supondo que a definição de Simbolo, Scope, FunctionSymbols e Tipo esteja em semantic.h) */

/* Ponteiro para o escopo corrente (inicia com NULL) */
Scope* current_scope = NULL;

/* Lista global onde vamos guardar os símbolos locais de cada função */
static FunctionSymbols* function_symbols_list = NULL;

/* Cria um novo escopo e o empilha */
void push_scope() {
    Scope* novo = malloc(sizeof(Scope));
    if (!novo) { perror("malloc"); exit(1); }
    novo->simbolos = NULL;
    novo->prox = current_scope;
    current_scope = novo;
}

/* Desempilha (libera) o escopo corrente */
void pop_scope() {
    if (current_scope) {
        Scope* temp = current_scope;
        current_scope = current_scope->prox;
        /* Se necessário, os símbolos podem ser liberados aqui ou mantidos para cópia */
        free(temp);
    }
}

void free_all_scopes() {
    while (current_scope) {
        pop_scope();
    }
}

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
    novo->isArray = isArray;  // Armazena corretamente se é vetor
    novo->prox = current_scope->simbolos;
    current_scope->simbolos = novo;
}

/* Procura um símbolo a partir do escopo corrente (percorrendo os pais) */
Simbolo* lookup_simbolo_scoped(const char* nome) {
    Scope* s = current_scope;
    while (s) {
        Simbolo* sim = s->simbolos;
        while (sim) {
            if (strcmp(sim->nome, nome) == 0)
                return sim;
            sim = sim->prox;
        }
        s = s->prox;
    }
    return NULL;
}

/* Imprime a tabela de símbolos do escopo corrente (para depuração) */
void print_tabela_simbolos() {
    if (!current_scope) {
        printf("Nenhum escopo ativo.\n");
        return;
    }
    Simbolo* sim = current_scope->simbolos;
    while (sim) {
        printf("Nome: %s, Tipo: %s%s\n", sim->nome,
               (sim->tipo == TIPO_INT ? "int" :
                (sim->tipo == TIPO_VOID ? "void" : "indefinido")),
               (sim->isArray ? " [vetor]" : ""));
        sim = sim->prox;
    }
}

/* Faz uma cópia dos símbolos ligados (escopo local) */
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

/* Adiciona na lista global function_symbols_list uma entrada
   (function_name, cópia dos símbolos do escopo local). */
static void store_local_symbols(const char* function_name, Simbolo* local_symbols) {
    FunctionSymbols* fs = malloc(sizeof(FunctionSymbols));
    if (!fs) { perror("malloc"); exit(1); }
    fs->func_name = strdup(function_name);
    fs->local_symbols = copy_symbols_list(local_symbols);
    fs->prox = function_symbols_list;
    function_symbols_list = fs;
}

/* Imprime todos os símbolos locais de todas as funções armazenados em function_symbols_list */
static void print_all_function_symbols() {
    FunctionSymbols* fs = function_symbols_list;
    while (fs) {
        printf("\nSímbolos locais da função '%s':\n", fs->func_name);
        Simbolo* sim = fs->local_symbols;
        while (sim) {
            printf("  Nome: %s, Tipo: %s%s\n", sim->nome,
                   (sim->tipo == TIPO_INT ? "int" :
                    (sim->tipo == TIPO_VOID ? "void" : "indefinido")),
                   (sim->isArray ? " [vetor]" : ""));
            sim = sim->prox;
        }
        fs = fs->prox;
    }
}

/* Libera os FunctionSymbols */
static void free_function_symbols_list() {
    FunctionSymbols* fs = function_symbols_list;
    while (fs) {
        FunctionSymbols* temp = fs;
        fs = fs->prox;
        free(temp->func_name);
        // Libera a lista local_symbols
        Simbolo* s = temp->local_symbols;
        while (s) {
            Simbolo* aux = s;
            s = s->prox;
            free(aux->nome);
            free(aux);
        }
        free(temp);
    }
    function_symbols_list = NULL;
}

/* ================================
   Função de Análise Semântica
   ================================ */

/* Converte uma string de tipo para o enum Tipo */
Tipo converter_tipo(const char* tipo_str) {
    if (strcmp(tipo_str, "int") == 0)
        return TIPO_INT;
    else if (strcmp(tipo_str, "void") == 0)
        return TIPO_VOID;
    return TIPO_INDEFINIDO;
}

/* Função recursiva para percorrer a AST e realizar a análise semântica.
   Trata declarações de variáveis (incluindo vetores), declarações de função,
   uso de variáveis e atribuições.
*/
Tipo verificar_semantica(ast* no) {
    if (no == NULL) return TIPO_INDEFINIDO;
    
    // Caso especial: literal numérico
    if (strcmp(no->name, "fator") == 0 && no->value != NULL) {
        if (isdigit(no->value[0]))
            return TIPO_INT;
    }
    
    // Declaração de variável (global ou local)
    if (strcmp(no->name, "declaracao_variavel") == 0) {
        Tipo t = TIPO_INDEFINIDO;
        int isArray = 0;  // Assume que não é vetor
        
        if (no->n_children > 0 && no->children[0] && no->children[0]->value) {
            t = converter_tipo(no->children[0]->value);
        }
        // Se o nó possui o rótulo "vetor", considere que é vetor
        if (no->value && strcmp(no->value, "vetor") == 0) {
            isArray = 1;
        }       

        if (no->n_children > 1 && no->children[1] && no->children[1]->value) {
            add_simbolo_scoped(no->children[1]->value, t, isArray);
        }
        return t;
    }
    
    // Declaração de função
    else if (strcmp(no->name, "declaracao_funcao") == 0) {
        Tipo t = TIPO_INDEFINIDO;
        // O primeiro filho é o especificador de tipo e o segundo é o nome da função.
        if (no->n_children > 0 && no->children[0] && no->children[0]->value) {
            t = converter_tipo(no->children[0]->value);
        }
        if (no->n_children > 1 && no->children[1] && no->children[1]->value) {
            add_simbolo_scoped(no->children[1]->value, t, 0);
        }
        push_scope();  // Novo escopo para os parâmetros e corpo da função

        // Processa os demais filhos (parâmetros e comando composto)
        for (int i = 2; i < no->n_children; i++) {
            verificar_semantica(no->children[i]);
        }

        // Imprime a tabela do escopo local desta função
        printf("Tabela de Símbolos (escopo local da função '%s'):\n", no->children[1]->value);
        print_tabela_simbolos();

        // Copia os símbolos locais para a lista global antes de liberar o escopo
        store_local_symbols(no->children[1]->value, current_scope->simbolos);

        pop_scope();  // Remove o escopo local da função
        return t;
    }
    
    // Tratamento de parâmetros (similar à declaração de variável)
    else if (strcmp(no->name, "parametro") == 0) {
    Tipo t = TIPO_INDEFINIDO;
    int isArray = 0;
    if (no->n_children > 0 && no->children[0] && no->children[0]->value) {
        t = converter_tipo(no->children[0]->value);
    }
    if (no->n_children > 1 && no->children[1] && no->children[1]->value) {
        // Aqui, como na AST o nó "parametro" mostra "vetor", usamos o campo value
        if (no->value && strcmp(no->value, "vetor") == 0) {
            isArray = 1;
        }
        add_simbolo_scoped(no->children[1]->value, t, isArray);
    }
    return t;
}


    
    // Uso de variável (escalares)
    else if (strcmp(no->name, "var") == 0) {
        if (no->value) {
            Simbolo* sim = lookup_simbolo_scoped(no->value);
            if (sim == NULL) {
                fprintf(stderr, "Erro semântico: variável '%s' não declarada.\n", no->value);
                return TIPO_INDEFINIDO;
            }
            return sim->tipo;
        }
    }
    
    // Acesso a vetor (array_access)
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
    
    // Atribuição
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
    
    // Comando composto (bloco)
    else if (strcmp(no->name, "comando_composto") == 0) {
        push_scope(); // Cria um novo escopo para o bloco
        for (int i = 0; i < no->n_children; i++) {
            verificar_semantica(no->children[i]);
        }
        // --- Alteração: mescla os símbolos do escopo do bloco ao escopo pai ---
        Scope* inner_scope = current_scope;
        Scope* parent_scope = current_scope->prox;
        if (parent_scope != NULL) {
            Simbolo* sim = inner_scope->simbolos;
            while (sim) {
                Simbolo* next = sim->prox;
                // Insere o símbolo na lista de símbolos do escopo pai
                sim->prox = parent_scope->simbolos;
                parent_scope->simbolos = sim;
                sim = next;
            }
        }
        pop_scope();  // Remove o escopo do bloco
        return TIPO_INDEFINIDO;
    }
    
    // Caso padrão: percorre recursivamente os filhos
    Tipo result = TIPO_INDEFINIDO;
    for (int i = 0; i < no->n_children; i++) {
        result = verificar_semantica(no->children[i]);
    }
    return result;
}

void analise_semantica(ast* raiz) {
    printf("Iniciando a análise semântica...\n");
    push_scope();  // Cria o escopo global
    verificar_semantica(raiz);

    /* Imprime escopo global */
    printf("\nTabela de Símbolos (escopo global):\n");
    print_tabela_simbolos();
    pop_scope();
    
    /* Agora imprime os símbolos locais de cada função */
    print_all_function_symbols();

    /* Libera a lista de function_symbols */
    free_function_symbols_list();

    /* E por fim libera todos os escopos (se ainda restar algo) */
    free_all_scopes();
}
