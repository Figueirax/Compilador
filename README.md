# C-Minus Lexer

Este projeto é um **analisador léxico (lexer)** desenvolvido com **Flex** para processar código escrito na linguagem **C-**, uma linguagem de programação fictícia baseada em C. Ele reconhece tokens como identificadores, palavras-chave, números, operadores e delimitadores, e ignora comentários e espaços em branco.

## Funcionalidades

- **Palavras-chave:** Reconhece `if`, `else`, `while`, `int`, `return`, `void`.
- **Identificadores:** Nomes de variáveis e funções seguindo o padrão `[a-zA-Z_][a-zA-Z0-9_]*`.
- **Números inteiros:** Sequências de dígitos (ex.: `123`).
- **Operadores:** Inclui `==`, `<`, `>`, `<=`, `>=`, `!=`, `=`, `+`, `-`, `*`, `/`.
- **Delimitadores:** Parênteses, colchetes, chaves, ponto e vírgula, vírgulas.
- **Comentários:**
  - Linha única com `//`
  - Bloco com `/* ... */`
- Ignora espaços, tabulações e quebras de linha.

## Como usar

### Requisitos

- **Flex**: Ferramenta para criar analisadores léxicos.
- **GCC** (ou outro compilador C): Para compilar o código gerado pelo Flex.

### Compilar e Executar

1. Compile o arquivo `lexer.l` com o Flex:
   ```bash
   flex lexer.l

2. Compile o código gerado (lex.yy.c) usando o GCC:
    gcc lex.yy.c -o lexer -lfl

3. Execute o lexer com um arquivo de entrada:
    ./lexer < arquivo_teste.c

