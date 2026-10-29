#ifndef ENTRADA_H
#define ENTRADA_H

#include <stdbool.h>
#include <stddef.h>

bool ler_texto(const char *mensagem, char *destino, size_t capacidade,
               bool permitir_vazio);
bool ler_inteiro(const char *mensagem, int minimo, int maximo, int *valor);
bool ler_decimal(const char *mensagem, double minimo, double maximo,
                 double *valor);
bool confirmar(const char *mensagem);

#endif
