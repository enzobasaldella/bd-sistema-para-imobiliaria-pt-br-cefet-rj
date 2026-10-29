#include "consulta.h"
#include "conexao.h"
#include "entrada.h"
#include <stdio.h>

static void exibir_consulta(const char *titulo, const char *cabecalho,
                            const char *sql, unsigned int colunas) {
  MYSQL *c = conectar();
  if (!c)
    return;
  MYSQL_RES *r = executar_consulta(c, sql, titulo);
  if (r) {
    MYSQL_ROW linha;
    printf("\n%s\n%s\n", titulo, cabecalho);
    while ((linha = mysql_fetch_row(r))) {
      for (unsigned int i = 0; i < colunas; i++)
        printf("%s%s", i ? " | " : "", campo_ou(linha[i], "-"));
      putchar('\n');
    }
    mysql_free_result(r);
  }
  mysql_close(c);
}

void clientes_com_multiplos_contratos(void) {
  exibir_consulta("Clientes com mais de um contrato",
                  "ID | Cliente | Contratos",
                  "SELECT c.id_cliente,c.nome,COUNT(*) FROM contrato co JOIN "
                  "proposta p ON p.id_proposta=co.id_proposta JOIN cliente c "
                  "ON c.id_cliente=p.id_inquilino GROUP BY c.id_cliente,c.nome "
                  "HAVING COUNT(*)>1 ORDER BY COUNT(*) DESC",
                  3);
}

void imoveis_sem_corretor(void) {
  exibir_consulta("Imoveis sem corretor",
                  "ID | Tipo | Cidade | Bairro | Aluguel",
                  "SELECT id_imovel,tipo,cidade,bairro,valor_aluguel FROM "
                  "imovel WHERE id_corretor IS NULL ORDER BY id_imovel",
                  5);
}

void alugueis_de_2022(void) {
  exibir_consulta(
      "Alugueis firmados em 2022",
      "Contrato | Inquilino | Imovel | Valor | Data",
      "SELECT "
      "co.id_contrato,c.nome,p.id_imovel,co.valor_inicial,co.data_contrato "
      "FROM contrato co JOIN proposta p ON p.id_proposta=co.id_proposta JOIN "
      "cliente c ON c.id_cliente=p.id_inquilino WHERE "
      "YEAR(co.data_contrato)=2022 ORDER BY co.data_contrato",
      5);
}

void corretor_com_maior_rendimento(void) {
  int ano;
  char sql[1000];
  if (!ler_inteiro("Ano: ", 1900, 9999, &ano))
    return;
  snprintf(sql, sizeof(sql),
           "SELECT "
           "c.id_corretor,c.nome,ROUND(SUM(co.valor_inicial*(c.comissao/"
           "100)*co.duracao),2) FROM contrato co JOIN proposta p ON "
           "p.id_proposta=co.id_proposta JOIN imovel i ON "
           "i.id_imovel=p.id_imovel JOIN corretor c ON "
           "c.id_corretor=i.id_corretor WHERE YEAR(co.data_contrato)=%d GROUP "
           "BY c.id_corretor,c.nome ORDER BY 3 DESC LIMIT 1",
           ano);
  exibir_consulta("Corretor com maior rendimento", "ID | Corretor | Rendimento",
                  sql, 3);
}

void tres_imoveis_mais_caros(void) {
  exibir_consulta("Tres imoveis mais caros",
                  "ID | Tipo | Aluguel | Proprietario",
                  "SELECT i.id_imovel,i.tipo,i.valor_aluguel,c.nome FROM "
                  "imovel i JOIN cliente c ON c.id_cliente=i.id_proprietario "
                  "ORDER BY i.valor_aluguel DESC LIMIT 3",
                  4);
}

void propostas_sem_contrato(void) {
  exibir_consulta(
      "Propostas sem contrato",
      "Proposta | Imovel | Inquilino | Valor | Data | Validade",
      "SELECT "
      "p.id_proposta,p.id_imovel,c.nome,p.valor_proposta,p.data_proposta,p."
      "validade FROM proposta p JOIN cliente c ON c.id_cliente=p.id_inquilino "
      "LEFT JOIN contrato co ON co.id_proposta=p.id_proposta WHERE "
      "co.id_contrato IS NULL ORDER BY p.id_proposta",
      6);
}

void imoveis_mais_visitados(void) {
  exibir_consulta(
      "Imoveis mais visitados", "Imovel | Proprietario | Visitas",
      "SELECT i.id_imovel,c.nome,COUNT(v.id_visita) FROM imovel i JOIN cliente "
      "c ON c.id_cliente=i.id_proprietario LEFT JOIN visita v ON "
      "v.id_imovel=i.id_imovel GROUP BY i.id_imovel,c.nome ORDER BY "
      "COUNT(v.id_visita) DESC,i.id_imovel",
      3);
}

void imoveis_ate_tres_mil(void) {
  exibir_consulta(
      "Imoveis de ate R$ 3.000",
      "ID | Tipo | Cidade | Bairro | Aluguel | Situacao",
      "SELECT id_imovel,tipo,cidade,bairro,valor_aluguel,situacao FROM imovel "
      "WHERE valor_aluguel<=3000 ORDER BY valor_aluguel",
      6);
}
