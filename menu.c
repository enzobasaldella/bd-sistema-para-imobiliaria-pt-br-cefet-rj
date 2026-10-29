#include "menu.h"

#include "cliente.h"
#include "consulta.h"
#include "corretor.h"
#include "entrada.h"
#include "fiador.h"
#include "imovel.h"
#include "negociacao.h"

#include <stdio.h>

static int escolher(const char *titulo, const char *itens, int maximo) {
  int opcao;
  printf("\n=== %s ===\n%s", titulo, itens);
  return ler_inteiro("Opcao: ", 0, maximo, &opcao) ? opcao : 0;
}

static void menu_clientes(void) {
  for (;;) {
    switch (escolher(
        "Clientes",
        "1 - Cadastrar\n2 - Listar\n3 - Modificar\n4 - Excluir\n0 - Voltar\n",
        4)) {
    case 1:
      cadastrar_cliente();
      break;
    case 2:
      listar_clientes();
      break;
    case 3:
      modificar_cliente();
      break;
    case 4:
      excluir_cliente();
      break;
    default:
      return;
    }
  }
}

static void menu_imoveis(void) {
  for (;;) {
    switch (escolher(
        "Imoveis",
        "1 - Cadastrar\n2 - Listar\n3 - Modificar\n4 - Excluir\n0 - Voltar\n",
        4)) {
    case 1:
      cadastrar_imovel();
      break;
    case 2:
      listar_imoveis();
      break;
    case 3:
      modificar_imovel();
      break;
    case 4:
      excluir_imovel();
      break;
    default:
      return;
    }
  }
}

static void menu_corretores(void) {
  for (;;) {
    switch (escolher(
        "Corretores",
        "1 - Cadastrar\n2 - Listar\n3 - Modificar\n4 - Excluir\n0 - Voltar\n",
        4)) {
    case 1:
      cadastrar_corretor();
      break;
    case 2:
      listar_corretores();
      break;
    case 3:
      modificar_corretor();
      break;
    case 4:
      excluir_corretor();
      break;
    default:
      return;
    }
  }
}

static void menu_fiadores(void) {
  for (;;) {
    switch (escolher(
        "Fiadores",
        "1 - Cadastrar\n2 - Listar\n3 - Modificar\n4 - Excluir\n0 - Voltar\n",
        4)) {
    case 1:
      cadastrar_fiador();
      break;
    case 2:
      listar_fiadores();
      break;
    case 3:
      modificar_fiador();
      break;
    case 4:
      excluir_fiador();
      break;
    default:
      return;
    }
  }
}

static void menu_negociacoes(void) {
  for (;;) {
    switch (escolher("Negociacoes",
                     "1 - Marcar visita\n2 - Listar visitas\n3 - Cancelar "
                     "visita\n4 - Registrar proposta\n5 - Listar propostas\n6 "
                     "- Firmar contrato\n0 - Voltar\n",
                     6)) {
    case 1:
      marcar_visita();
      break;
    case 2:
      listar_visitas();
      break;
    case 3:
      cancelar_visita();
      break;
    case 4:
      registrar_proposta();
      break;
    case 5:
      listar_propostas();
      break;
    case 6:
      firmar_contrato();
      break;
    default:
      return;
    }
  }
}

static void menu_consultas(void) {
  for (;;) {
    switch (escolher(
        "Relatorios",
        "1 - Clientes com mais de um contrato\n2 - Imoveis sem corretor\n3 - "
        "Alugueis firmados em 2022\n4 - Corretor com maior rendimento por "
        "ano\n5 - Tres imoveis mais caros\n6 - Propostas sem contrato\n7 - "
        "Imoveis mais visitados\n8 - Imoveis de ate R$ 3.000\n0 - Voltar\n",
        8)) {
    case 1:
      clientes_com_multiplos_contratos();
      break;
    case 2:
      imoveis_sem_corretor();
      break;
    case 3:
      alugueis_de_2022();
      break;
    case 4:
      corretor_com_maior_rendimento();
      break;
    case 5:
      tres_imoveis_mais_caros();
      break;
    case 6:
      propostas_sem_contrato();
      break;
    case 7:
      imoveis_mais_visitados();
      break;
    case 8:
      imoveis_ate_tres_mil();
      break;
    default:
      return;
    }
  }
}

void menu_principal(void) {
  puts("Sistema de gestao imobiliaria");
  for (;;) {
    switch (escolher("Menu principal",
                     "1 - Clientes\n2 - Imoveis\n3 - Corretores\n4 - "
                     "Fiadores\n5 - Negociacoes\n6 - Relatorios\n0 - Sair\n",
                     6)) {
    case 1:
      menu_clientes();
      break;
    case 2:
      menu_imoveis();
      break;
    case 3:
      menu_corretores();
      break;
    case 4:
      menu_fiadores();
      break;
    case 5:
      menu_negociacoes();
      break;
    case 6:
      menu_consultas();
      break;
    default:
      puts("Programa encerrado.");
      return;
    }
  }
}
