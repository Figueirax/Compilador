# Nome do executável final
EXEC = compilador

# Arquivos fonte
LEXER = lexer.l
PARSER = parser.y
AST = ast.c
SEMANTIC = semantic.c

# Arquivos gerados pelo Bison e Flex
PARSER_C = parser.tab.c
PARSER_H = parser.tab.h
LEXER_C = lex.yy.c

# Compilador
CC = gcc

# Flags para depuração (opcional)
CFLAGS = -Wall -Wextra -g

# Regras para compilação
all: $(EXEC)

# Geração do parser com Bison
$(PARSER_C) $(PARSER_H): $(PARSER)
	bison -d $(PARSER)

# Geração do lexer com Flex
$(LEXER_C): $(LEXER)
	flex $(LEXER)

# Compilação do programa principal
$(EXEC): $(PARSER_C) $(LEXER_C) $(AST) $(SEMANTIC)
	$(CC) $(CFLAGS) -o $(EXEC) $(PARSER_C) $(LEXER_C) $(AST) $(SEMANTIC) -lm

# Comando para rodar o compilador com um arquivo de entrada
run: $(EXEC)
	./$(EXEC) < teste.txt

# Limpeza dos arquivos gerados
clean:
	rm -f $(PARSER_C) $(PARSER_H) $(LEXER_C) $(EXEC)
