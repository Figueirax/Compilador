%{
#include <stdio.h>
#include <string.h>

// número de linhas
int line_number = 1;
%}

%%
"if"     { printf("IF " ); }
"else"   { printf("ELSE " ); }
"while"  { printf("WHILE " ); }
"int"    { printf("INT " ); }
"return" { printf("RETURN " ); }
"void"   { printf("VOID " ); }

[a-zA-Z]+ {
    printf("ID " );
}

[0-9]+ {
    printf("NUM " );
}

"==" { printf("IGUALDADE " ); }
"<=" { printf("MENOR IGL " ); }
">=" { printf("MAIOR IGL " ); }
"<"  { printf("MENOR " ); }
">"  { printf("MAIOR " ); }
"!=" { printf("DIFERENTE " ); }
"="  { printf("IGUAL " ); }
"+"  { printf("SOMA " ); }
"-"  { printf("SUB " ); }
"*"  { printf("MULT " ); }
"/"  { printf("DIV " ); }

"(" { printf("ABRE PAREN " ); }
")" { printf("FECHA PAREN " ); }
"{" { printf("ABRE CHAVES " ); }
"}" { printf("FECHA CHAVES " ); }
"[" { printf("ABRE COL " ); }
"]" { printf("FECHA COL " ); }
";" { printf("PEV " ); }
"," { printf("VIRGULA " ); }

"//".* {
    printf("COMENTARIO LINHA");
}

"/*"([^*]|\*+[^*/])*\*+"/" {
    printf("COMENTARIO DE BLOCO");
    // Incrementar linhas dentro de comentários de bloco
    for (int i = 0; yytext[i] != '\0'; i++) {
        if (yytext[i] == '\n') line_number++;
    }
}

[ \t]+ ; 

\n { line_number++; printf("\n"); }

. { printf("ERRO LÉXICO: %s (linha %d)", yytext, line_number);}

%%

int main(int argc, char **argv) {
    yylex();
    return 0;
}
