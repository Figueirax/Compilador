%{
#include <stdio.h>
#include <string.h>

// número de linhas
int line_number = 1;
%}

%%
"if"|"else"|"while"|"int"|"return"|"void" {
    printf("PALAVRA-CHAVE: %s (linha %d)\n", yytext, line_number);
}

[a-zA-Z_][a-zA-Z0-9_]* {
    printf("IDENTIFICADOR: %s (linha %d)\n", yytext, line_number);
}

[0-9]+ {
    printf("NUMERO: %s (linha %d)\n", yytext, line_number);
}

"=="|"<="|">="|"<"|">"|"!="|"="|"\+"|"-"|"*"|"/" {
    printf("OPERADOR: %s (linha %d)\n", yytext, line_number);
}

"("|")"|"{"|"}"|"["|"]"|";"|"," {
    printf("DELIMITADOR: %s (linha %d)\n", yytext, line_number);
}

"//".* {
    printf("COMENTARIO DE LINHA (linha %d): %s\n", line_number, yytext);
}

"/*"([^*]|\*+[^*/])*\*+"/" {
    printf("COMENTARIO DE BLOCO (linha %d)\n", line_number);
    // Incrementar linhas dentro de comentários de bloco
    for (int i = 0; yytext[i] != '\0'; i++) {
        if (yytext[i] == '\n') line_number++;
    }
}

[ \t]+ ; 

\n { line_number++; }

. {
    printf("CARACTERE INVALIDO: %s (linha %d)\n", yytext, line_number);
}

%%

int main(int argc, char **argv) {
    printf("Analisador Léxico para C- Iniciado\n");
    yylex(); // Executa o analisador léxico
    printf("Analisador Léxico Finalizado\n");
    return 0;
}
