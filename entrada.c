#include "entrada.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void descartar_restante_da_linha(void) {
  int caractere;
  while ((caractere = getchar()) != '\n' && caractere != EOF) {
  }
}

bool ler_texto(const char *mensagem, char *destino, size_t capacidade,
               bool permitir_vazio) {
  if (destino == NULL || capacidade < 2) {
    return false;
  }

  for (;;) {
    fputs(mensagem, stdout);
    if (fgets(destino, (int)capacidade, stdin) == NULL) {
      putchar('\n');
      return false;
    }

    size_t tamanho = strlen(destino);
    if (tamanho > 0 && destino[tamanho - 1] == '\n') {
      destino[tamanho - 1] = '\0';
    } else {
      descartar_restante_da_linha();
      printf("Entrada muito longa (maximo: %zu caracteres).\n", capacidade - 1);
      continue;
    }

    if (permitir_vazio || destino[0] != '\0') {
      return true;
    }
    puts("O valor nao pode ficar vazio.");
  }
}

bool ler_inteiro(const char *mensagem, int minimo, int maximo, int *valor) {
  char entrada[64];
  char *fim;
  long convertido;

  for (;;) {
    if (!ler_texto(mensagem, entrada, sizeof(entrada), false)) {
      return false;
    }
    errno = 0;
    convertido = strtol(entrada, &fim, 10);
    while (isspace((unsigned char)*fim)) {
      fim++;
    }
    if (errno == 0 && *fim == '\0' && convertido >= minimo &&
        convertido <= maximo) {
      *valor = (int)convertido;
      return true;
    }
    printf("Informe um numero inteiro entre %d e %d.\n", minimo, maximo);
  }
}

bool ler_decimal(const char *mensagem, double minimo, double maximo,
                 double *valor) {
  char entrada[64];
  char *fim;
  double convertido;

  for (;;) {
    if (!ler_texto(mensagem, entrada, sizeof(entrada), false)) {
      return false;
    }
    errno = 0;
    convertido = strtod(entrada, &fim);
    while (isspace((unsigned char)*fim)) {
      fim++;
    }
    if (errno == 0 && *fim == '\0' && convertido >= minimo &&
        convertido <= maximo) {
      *valor = convertido;
      return true;
    }
    printf("Informe um valor entre %.2f e %.2f (use ponto como separador).\n",
           minimo, maximo);
  }
}

bool confirmar(const char *mensagem) {
  char resposta[8];
  return ler_texto(mensagem, resposta, sizeof(resposta), false) &&
         (resposta[0] == 's' || resposta[0] == 'S');
}
