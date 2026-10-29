CC ?= gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror
MYSQL_CFLAGS := $(shell mysql_config --cflags 2>/dev/null || pkg-config --cflags mysqlclient 2>/dev/null)
MYSQL_LIBS := $(shell mysql_config --libs 2>/dev/null || pkg-config --libs mysqlclient 2>/dev/null)
FONTES := main.c menu.c entrada.c conexao.c cliente.c imovel.c corretor.c fiador.c negociacao.c consulta.c
OBJETOS := $(FONTES:.c=.o)
EXECUTAVEL := imobiliaria

.PHONY: all clean run static-check

all: $(EXECUTAVEL)

$(EXECUTAVEL): $(OBJETOS)
	$(CC) $(OBJETOS) $(MYSQL_LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(MYSQL_CFLAGS) -c $< -o $@

run: $(EXECUTAVEL)
	./$(EXECUTAVEL)

static-check:
	$(CC) $(CFLAGS) -Itests/stubs -fsyntax-only $(FONTES)
	python3 tests/validar_repositorio.py

clean:
	$(RM) $(OBJETOS) $(EXECUTAVEL)
