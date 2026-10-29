#include "fiador.h"
#include "conexao.h"
#include "entrada.h"
#include <limits.h>
#include <stdio.h>

void cadastrar_fiador(void) {
  int inquilino;
  double saldo;
  char nome[101], tel[21];
  if (!ler_inteiro("ID do inquilino: ", 1, INT_MAX, &inquilino) ||
      !ler_texto("Nome: ", nome, sizeof(nome), false) ||
      !ler_texto("Telefone: ", tel, sizeof(tel), false) ||
      !ler_decimal("Saldo bancario: ", 0, 9999999999.99, &saldo))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  char en[201], et[41], q[600];
  escapar_texto(c, nome, en, sizeof(en));
  escapar_texto(c, tel, et, sizeof(et));
  snprintf(q, sizeof(q),
           "INSERT INTO fiador(id_inquilino,nome,telefone,saldo_bancario) "
           "VALUES(%d,'%s','%s',%.2f)",
           inquilino, en, et, saldo);
  if (executar_comando(c, q, "cadastrar o fiador"))
    puts("Fiador cadastrado.");
  mysql_close(c);
}

void listar_fiadores(void) {
  MYSQL *c = conectar();
  if (!c)
    return;
  MYSQL_RES *r = executar_consulta(
      c,
      "SELECT f.id_fiador,f.nome,f.telefone,f.saldo_bancario,c.nome FROM "
      "fiador f JOIN cliente c ON c.id_cliente=f.id_inquilino ORDER BY "
      "f.id_fiador",
      "listar os fiadores");
  if (r) {
    MYSQL_ROW x;
    puts("\nID | Nome | Telefone | Saldo | Inquilino");
    while ((x = mysql_fetch_row(r)))
      printf("%s | %s | %s | R$ %s | %s\n", x[0], x[1], x[2], x[3], x[4]);
    mysql_free_result(r);
  }
  mysql_close(c);
}

void modificar_fiador(void) {
  int id, op;
  double saldo;
  char tel[21], et[41], q[300];
  if (!ler_inteiro("ID do fiador: ", 1, INT_MAX, &id) ||
      !ler_inteiro("Campo (1-Telefone, 2-Saldo): ", 1, 2, &op))
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
    snprintf(q, sizeof(q), "UPDATE fiador SET telefone='%s' WHERE id_fiador=%d",
             et, id);
  } else {
    if (!ler_decimal("Novo saldo: ", 0, 9999999999.99, &saldo)) {
      mysql_close(c);
      return;
    }
    snprintf(q, sizeof(q),
             "UPDATE fiador SET saldo_bancario=%.2f WHERE id_fiador=%d", saldo,
             id);
  }
  if (executar_comando(c, q, "atualizar o fiador"))
    puts(mysql_affected_rows(c) ? "Fiador atualizado."
                                : "Fiador nao encontrado ou sem alteracoes.");
  mysql_close(c);
}

void excluir_fiador(void) {
  int id;
  if (!ler_inteiro("ID do fiador: ", 1, INT_MAX, &id) ||
      !confirmar("Confirmar exclusao? (s/N): "))
    return;
  char q[128];
  snprintf(q, sizeof(q), "DELETE FROM fiador WHERE id_fiador=%d", id);
  MYSQL *c = conectar();
  if (!c)
    return;
  if (executar_comando(c, q, "excluir o fiador"))
    puts(mysql_affected_rows(c) ? "Fiador excluido."
                                : "Fiador nao encontrado.");
  mysql_close(c);
}
