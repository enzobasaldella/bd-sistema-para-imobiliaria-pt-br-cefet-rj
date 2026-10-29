#include "conexao.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *variavel_ou(const char *nome, const char *padrao) {
  const char *valor = getenv(nome);
  return valor != NULL && valor[0] != '\0' ? valor : padrao;
}

MYSQL *conectar(void) {
  MYSQL *conexao = mysql_init(NULL);
  if (conexao == NULL) {
    fputs("Nao foi possivel inicializar o cliente MySQL.\n", stderr);
    return NULL;
  }

  unsigned int porta =
      (unsigned int)strtoul(variavel_ou("IMOB_DB_PORT", "3306"), NULL, 10);
  const char *host = variavel_ou("IMOB_DB_HOST", "127.0.0.1");
  const char *usuario = variavel_ou("IMOB_DB_USER", "root");
  const char *senha = variavel_ou("IMOB_DB_PASSWORD", "");
  const char *banco = variavel_ou("IMOB_DB_NAME", "imobiliaria");

  if (mysql_real_connect(conexao, host, usuario, senha, banco, porta, NULL,
                         0) == NULL) {
    fprintf(stderr, "Falha ao conectar ao MySQL em %s:%u: %s\n", host, porta,
            mysql_error(conexao));
    mysql_close(conexao);
    return NULL;
  }
  if (mysql_set_character_set(conexao, "utf8mb4") != 0) {
    fprintf(stderr, "Falha ao configurar utf8mb4: %s\n", mysql_error(conexao));
    mysql_close(conexao);
    return NULL;
  }
  return conexao;
}

bool escapar_texto(MYSQL *conexao, const char *origem, char *destino,
                   size_t capacidade) {
  size_t tamanho = strlen(origem);
  if (capacidade < (tamanho * 2) + 1) {
    fputs("Buffer insuficiente para preparar o texto da consulta.\n", stderr);
    return false;
  }
  unsigned long gravados = mysql_real_escape_string(conexao, destino, origem,
                                                    (unsigned long)tamanho);
  destino[gravados] = '\0';
  return true;
}

bool executar_comando(MYSQL *conexao, const char *consulta,
                      const char *operacao) {
  if (mysql_query(conexao, consulta) != 0) {
    fprintf(stderr, "Erro ao %s: %s\n", operacao, mysql_error(conexao));
    return false;
  }
  return true;
}

MYSQL_RES *executar_consulta(MYSQL *conexao, const char *consulta,
                             const char *operacao) {
  if (!executar_comando(conexao, consulta, operacao)) {
    return NULL;
  }
  MYSQL_RES *resultado = mysql_store_result(conexao);
  if (resultado == NULL) {
    fprintf(stderr, "Erro ao obter o resultado de %s: %s\n", operacao,
            mysql_error(conexao));
  }
  return resultado;
}

const char *campo_ou(const char *campo, const char *alternativa) {
  return campo != NULL ? campo : alternativa;
}
