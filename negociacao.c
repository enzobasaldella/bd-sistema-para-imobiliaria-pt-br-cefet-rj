#include "negociacao.h"
#include "conexao.h"
#include "entrada.h"
#include <limits.h>
#include <stdio.h>

void marcar_visita(void) {
  int imovel, inquilino;
  char data_hora[20], e_data[39], q[700];
  if (!ler_inteiro("ID do imovel: ", 1, INT_MAX, &imovel) ||
      !ler_inteiro("ID do inquilino: ", 1, INT_MAX, &inquilino) ||
      !ler_texto("Data e hora (AAAA-MM-DD HH:MM:SS): ", data_hora,
                 sizeof(data_hora), false))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  escapar_texto(c, data_hora, e_data, sizeof(e_data));
  /* O corretor vem do imovel para impedir visitas associadas a outro
   * responsavel. */
  snprintf(q, sizeof(q),
           "INSERT INTO visita(id_inquilino,id_imovel,id_corretor,data_visita) "
           "SELECT %d,i.id_imovel,i.id_corretor,'%s' FROM imovel i WHERE "
           "i.id_imovel=%d AND i.id_corretor IS NOT NULL AND "
           "i.situacao='Disponivel'",
           inquilino, e_data, imovel);
  if (executar_comando(c, q, "marcar a visita"))
    puts(mysql_affected_rows(c)
             ? "Visita marcada."
             : "Imovel indisponivel, inexistente ou sem corretor.");
  mysql_close(c);
}

void registrar_proposta(void) {
  int inquilino, imovel;
  double valor;
  char data[11], validade[11], e_data[21], e_validade[21], q[700];
  if (!ler_inteiro("ID do inquilino: ", 1, INT_MAX, &inquilino) ||
      !ler_inteiro("ID do imovel: ", 1, INT_MAX, &imovel) ||
      !ler_decimal("Valor proposto: ", 0.01, 9999999999.99, &valor) ||
      !ler_texto("Data da proposta (AAAA-MM-DD): ", data, sizeof(data),
                 false) ||
      !ler_texto("Validade (AAAA-MM-DD): ", validade, sizeof(validade), false))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  escapar_texto(c, data, e_data, sizeof(e_data));
  escapar_texto(c, validade, e_validade, sizeof(e_validade));
  snprintf(q, sizeof(q),
           "INSERT INTO "
           "proposta(id_inquilino,id_imovel,valor_proposta,data_proposta,"
           "validade) SELECT %d,i.id_imovel,%.2f,'%s','%s' FROM imovel i WHERE "
           "i.id_imovel=%d AND i.situacao='Disponivel'",
           inquilino, valor, e_data, e_validade, imovel);
  if (executar_comando(c, q, "registrar a proposta"))
    puts(mysql_affected_rows(c) ? "Proposta registrada."
                                : "Imovel inexistente ou indisponivel.");
  mysql_close(c);
}

void firmar_contrato(void) {
  int proposta, duracao;
  char q[1000];
  if (!ler_inteiro("ID da proposta: ", 1, INT_MAX, &proposta) ||
      !ler_inteiro("Duracao do contrato (meses): ", 1, 600, &duracao))
    return;
  MYSQL *c = conectar();
  if (!c)
    return;
  if (!executar_comando(c, "START TRANSACTION", "iniciar a transacao"))
    goto fim;
  /* O SELECT ... FOR UPDATE evita dois contratos simultaneos para o mesmo
   * imovel. */
  snprintf(q, sizeof(q),
           "SELECT p.id_proposta FROM proposta p JOIN imovel i ON "
           "i.id_imovel=p.id_imovel WHERE p.id_proposta=%d AND "
           "p.validade>=CURRENT_DATE AND i.situacao='Disponivel' AND (SELECT "
           "COUNT(*) FROM fiador f WHERE f.id_inquilino=p.id_inquilino)>=2 FOR "
           "UPDATE",
           proposta);
  MYSQL_RES *r = executar_consulta(c, q, "validar a proposta");
  if (!r)
    goto rollback;
  bool proposta_valida = mysql_fetch_row(r) != NULL;
  mysql_free_result(r);
  if (!proposta_valida) {
    puts("A proposta nao esta valida, o imovel nao esta disponivel ou faltam "
         "fiadores.");
    goto rollback;
  }
  snprintf(
      q, sizeof(q),
      "INSERT INTO contrato(id_proposta,duracao,valor_inicial,data_contrato) "
      "SELECT id_proposta,%d,valor_proposta,CURRENT_DATE FROM proposta WHERE "
      "id_proposta=%d",
      duracao, proposta);
  if (!executar_comando(c, q, "firmar o contrato"))
    goto rollback;
  if (executar_comando(c, "COMMIT", "confirmar a transacao"))
    puts("Contrato firmado.");
  goto fim;
rollback:
  executar_comando(c, "ROLLBACK", "desfazer a transacao");
fim:
  mysql_close(c);
}

void listar_visitas(void) {
  MYSQL *c = conectar();
  if (!c)
    return;
  MYSQL_RES *r = executar_consulta(
      c,
      "SELECT v.id_visita,ci.nome,v.id_imovel,co.nome,v.data_visita FROM "
      "visita v JOIN cliente ci ON ci.id_cliente=v.id_inquilino JOIN corretor "
      "co ON co.id_corretor=v.id_corretor ORDER BY v.data_visita",
      "listar as visitas");
  if (r) {
    MYSQL_ROW x;
    puts("\nID | Inquilino | Imovel | Corretor | Data e hora");
    while ((x = mysql_fetch_row(r)))
      printf("%s | %s | %s | %s | %s\n", x[0], x[1], x[2], x[3], x[4]);
    mysql_free_result(r);
  }
  mysql_close(c);
}

void cancelar_visita(void) {
  int id;
  if (!ler_inteiro("ID da visita: ", 1, INT_MAX, &id) ||
      !confirmar("Confirmar cancelamento? (s/N): "))
    return;
  char q[128];
  snprintf(q, sizeof(q), "DELETE FROM visita WHERE id_visita=%d", id);
  MYSQL *c = conectar();
  if (!c)
    return;
  if (executar_comando(c, q, "cancelar a visita"))
    puts(mysql_affected_rows(c) ? "Visita cancelada."
                                : "Visita nao encontrada.");
  mysql_close(c);
}

void listar_propostas(void) {
  MYSQL *c = conectar();
  if (!c)
    return;
  MYSQL_RES *r = executar_consulta(
      c,
      "SELECT "
      "p.id_proposta,c.nome,p.id_imovel,p.valor_proposta,p.data_proposta,p."
      "validade,IF(co.id_contrato IS NULL,'Pendente','Contratada') FROM "
      "proposta p JOIN cliente c ON c.id_cliente=p.id_inquilino LEFT JOIN "
      "contrato co ON co.id_proposta=p.id_proposta ORDER BY p.id_proposta",
      "listar as propostas");
  if (r) {
    MYSQL_ROW x;
    puts("\nID | Inquilino | Imovel | Valor | Data | Validade | Estado");
    while ((x = mysql_fetch_row(r)))
      printf("%s | %s | %s | R$ %s | %s | %s | %s\n", x[0], x[1], x[2], x[3],
             x[4], x[5], x[6]);
    mysql_free_result(r);
  }
  mysql_close(c);
}
