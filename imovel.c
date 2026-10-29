#include "imovel.h"
#include "conexao.h"
#include "entrada.h"
#include <limits.h>
#include <stdio.h>

void cadastrar_imovel(void) {
  int proprietario, corretor, comodos, vagas;
  double area, aluguel;
  char tipo[20], cidade[51], bairro[51], rua[151], numero[11], data[11];
  if (!ler_inteiro("ID do proprietario: ", 1, INT_MAX, &proprietario) ||
      !ler_inteiro("ID do corretor (0 para nenhum): ", 0, INT_MAX, &corretor) ||
      !ler_texto("Tipo (Casa/Apartamento/Kitnet): ", tipo, sizeof(tipo),
                 false) ||
      !ler_inteiro("Numero de comodos: ", 1, 100, &comodos) ||
      !ler_inteiro("Vagas de garagem: ", 0, 100, &vagas) ||
      !ler_decimal("Area construida (m2): ", 0.01, 999999.99, &area) ||
      !ler_texto("Cidade: ", cidade, sizeof(cidade), false) ||
      !ler_texto("Bairro: ", bairro, sizeof(bairro), false) ||
      !ler_texto("Rua: ", rua, sizeof(rua), false) ||
      !ler_texto("Numero: ", numero, sizeof(numero), false) ||
      !ler_decimal("Valor do aluguel: ", 0.01, 9999999999.99, &aluguel) ||
      !ler_texto("Data da oferta (AAAA-MM-DD): ", data, sizeof(data), false))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  char etipo[39], ecidade[101], ebairro[101], erua[301], enumero[21], edata[21],
      q[1500];
  escapar_texto(c, tipo, etipo, sizeof(etipo));
  escapar_texto(c, cidade, ecidade, sizeof(ecidade));
  escapar_texto(c, bairro, ebairro, sizeof(ebairro));
  escapar_texto(c, rua, erua, sizeof(erua));
  escapar_texto(c, numero, enumero, sizeof(enumero));
  escapar_texto(c, data, edata, sizeof(edata));
  if (corretor)
    snprintf(q, sizeof(q),
             "INSERT INTO "
             "imovel(id_proprietario,id_corretor,tipo,num_comodos,vagas_"
             "garagem,area,cidade,bairro,rua,numero,valor_aluguel,data_oferta) "
             "VALUES(%d,%d,'%s',%d,%d,%.2f,'%s','%s','%s','%s',%.2f,'%s')",
             proprietario, corretor, etipo, comodos, vagas, area, ecidade,
             ebairro, erua, enumero, aluguel, edata);
  else
    snprintf(q, sizeof(q),
             "INSERT INTO "
             "imovel(id_proprietario,id_corretor,tipo,num_comodos,vagas_"
             "garagem,area,cidade,bairro,rua,numero,valor_aluguel,data_oferta) "
             "VALUES(%d,NULL,'%s',%d,%d,%.2f,'%s','%s','%s','%s',%.2f,'%s')",
             proprietario, etipo, comodos, vagas, area, ecidade, ebairro, erua,
             enumero, aluguel, edata);
  if (executar_comando(c, q, "cadastrar o imovel"))
    puts("Imovel cadastrado.");
  mysql_close(c);
}

void listar_imoveis(void) {
  MYSQL *c = conectar();
  if (!c)
    return;
  const char *q =
      "SELECT "
      "i.id_imovel,i.tipo,i.cidade,i.bairro,i.rua,i.numero,i.valor_aluguel,i."
      "situacao,p.nome,co.nome FROM imovel i JOIN proprietario pr ON "
      "pr.id_proprietario=i.id_proprietario JOIN cliente p ON "
      "p.id_cliente=pr.id_proprietario LEFT JOIN corretor co ON "
      "co.id_corretor=i.id_corretor ORDER BY i.id_imovel";
  MYSQL_RES *r = executar_consulta(c, q, "listar os imoveis");
  if (r) {
    MYSQL_ROW x;
    puts("\nID | Tipo | Endereco | Aluguel | Situacao | Proprietario | "
         "Corretor");
    while ((x = mysql_fetch_row(r)))
      printf("%s | %s | %s, %s, %s, %s | R$ %s | %s | %s | %s\n", x[0], x[1],
             x[2], x[3], x[4], x[5], x[6], x[7], x[8],
             campo_ou(x[9], "Nao atribuido"));
    mysql_free_result(r);
  }
  mysql_close(c);
}

void modificar_imovel(void) {
  int id, op, corretor;
  double aluguel;
  char q[256];
  if (!ler_inteiro("ID do imovel: ", 1, INT_MAX, &id) ||
      !ler_inteiro("Campo (1-Aluguel, 2-Corretor): ", 1, 2, &op))
    return;
  if (op == 1) {
    if (!ler_decimal("Novo aluguel: ", 0.01, 9999999999.99, &aluguel))
      return;
    snprintf(q, sizeof(q),
             "UPDATE imovel SET valor_aluguel=%.2f WHERE id_imovel=%d", aluguel,
             id);
  } else {
    if (!ler_inteiro("ID do corretor (0 para remover): ", 0, INT_MAX,
                     &corretor))
      return;
    if (corretor)
      snprintf(q, sizeof(q),
               "UPDATE imovel SET id_corretor=%d WHERE id_imovel=%d", corretor,
               id);
    else
      snprintf(q, sizeof(q),
               "UPDATE imovel SET id_corretor=NULL WHERE id_imovel=%d", id);
  }
  MYSQL *c = conectar();
  if (!c)
    return;
  if (executar_comando(c, q, "atualizar o imovel"))
    puts(mysql_affected_rows(c) ? "Imovel atualizado."
                                : "Imovel nao encontrado ou sem alteracoes.");
  mysql_close(c);
}

void excluir_imovel(void) {
  int id;
  if (!ler_inteiro("ID do imovel: ", 1, INT_MAX, &id) ||
      !confirmar("Confirmar exclusao? (s/N): "))
    return;
  char q[128];
  snprintf(q, sizeof(q), "DELETE FROM imovel WHERE id_imovel=%d", id);
  MYSQL *c = conectar();
  if (!c)
    return;
  if (executar_comando(c, q, "excluir o imovel"))
    puts(mysql_affected_rows(c) ? "Imovel excluido."
                                : "Imovel nao encontrado.");
  mysql_close(c);
}
