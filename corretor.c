#include "corretor.h"
#include "conexao.h"
#include "entrada.h"
#include <limits.h>
#include <stdio.h>

void cadastrar_corretor(void) {
  char nome[201], creci[51], tel[21], data[11];
  double comissao;
  if (!ler_texto("Nome: ", nome, sizeof(nome), false) ||
      !ler_texto("CRECI: ", creci, sizeof(creci), false) ||
      !ler_texto("Telefone: ", tel, sizeof(tel), false) ||
      !ler_texto("Inicio da prestacao (AAAA-MM-DD): ", data, sizeof(data),
                 false) ||
      !ler_decimal("Comissao (%): ", 0, 100, &comissao))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  char en[401], ec[101], et[41], ed[21], q[900];
  escapar_texto(c, nome, en, sizeof(en));
  escapar_texto(c, creci, ec, sizeof(ec));
  escapar_texto(c, tel, et, sizeof(et));
  escapar_texto(c, data, ed, sizeof(ed));
  snprintf(q, sizeof(q),
           "INSERT INTO corretor(nome,creci,telefone,data_prestacao,comissao) "
           "VALUES('%s','%s','%s','%s',%.2f)",
           en, ec, et, ed, comissao);
  if (executar_comando(c, q, "cadastrar o corretor"))
    puts("Corretor cadastrado.");
  mysql_close(c);
}

void listar_corretores(void) {
  MYSQL *c = conectar();
  if (!c)
    return;
  MYSQL_RES *r = executar_consulta(
      c,
      "SELECT id_corretor,nome,creci,telefone,data_prestacao,comissao FROM "
      "corretor ORDER BY id_corretor",
      "listar os corretores");
  if (r) {
    MYSQL_ROW x;
    puts("\nID | Nome | CRECI | Telefone | Inicio | Comissao");
    while ((x = mysql_fetch_row(r)))
      printf("%s | %s | %s | %s | %s | %s%%\n", x[0], x[1], x[2], x[3], x[4],
             x[5]);
    mysql_free_result(r);
  }
  mysql_close(c);
}

void modificar_corretor(void) {
  int id, op;
  double comissao;
  char tel[21], et[41], q[300];
  if (!ler_inteiro("ID do corretor: ", 1, INT_MAX, &id) ||
      !ler_inteiro("Campo (1-Telefone, 2-Comissao): ", 1, 2, &op))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  if (op == 1) {
    if (!ler_texto("Novo telefone: ", tel, sizeof(tel), false)) {
      mysql_close(c);
      return;
    }
    escapar_texto(c, tel, et, sizeof(et));
    snprintf(q, sizeof(q),
             "UPDATE corretor SET telefone='%s' WHERE id_corretor=%d", et, id);
  } else {
    if (!ler_decimal("Nova comissao (%): ", 0, 100, &comissao)) {
      mysql_close(c);
      return;
    }
    snprintf(q, sizeof(q),
             "UPDATE corretor SET comissao=%.2f WHERE id_corretor=%d", comissao,
             id);
  }
  if (executar_comando(c, q, "atualizar o corretor"))
    puts(mysql_affected_rows(c) ? "Corretor atualizado."
                                : "Corretor nao encontrado ou sem alteracoes.");
  mysql_close(c);
}

void excluir_corretor(void) {
  int id;
  if (!ler_inteiro("ID do corretor: ", 1, INT_MAX, &id) ||
      !confirmar("Confirmar exclusao? (s/N): "))
    return;
  char q[128];
  snprintf(q, sizeof(q), "DELETE FROM corretor WHERE id_corretor=%d", id);
  MYSQL *c = conectar();
  if (!c)
    return;
  if (executar_comando(c, q, "excluir o corretor"))
    puts(mysql_affected_rows(c) ? "Corretor excluido."
                                : "Corretor nao encontrado.");
  mysql_close(c);
}
