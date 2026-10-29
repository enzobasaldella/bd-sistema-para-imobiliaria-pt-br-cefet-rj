#ifndef CONEXAO_H
#define CONEXAO_H

#include <mysql.h>
#include <stdbool.h>
#include <stddef.h>

MYSQL *conectar(void);
bool escapar_texto(MYSQL *conexao, const char *origem, char *destino,
                   size_t capacidade);
bool executar_comando(MYSQL *conexao, const char *consulta,
                      const char *operacao);
MYSQL_RES *executar_consulta(MYSQL *conexao, const char *consulta,
                             const char *operacao);
const char *campo_ou(const char *campo, const char *alternativa);

#endif
