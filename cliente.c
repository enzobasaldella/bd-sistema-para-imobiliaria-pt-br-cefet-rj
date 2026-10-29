#include "cliente.h"

#include "conexao.h"
#include "entrada.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

void cadastrar_cliente(void) {
  MYSQL *conexao = conectar();
  if (conexao == NULL)
    return;

  char nome[201], cpf[12], telefone_um[21], telefone_dois[21];
  char estado_civil[20], tipo[20], query[1400];
  char e_nome[401], e_cpf[23], e_tel1[41], e_tel2[41], e_estado[39];

  puts("\nCadastro de cliente");
  if (!ler_texto("Nome: ", nome, sizeof(nome), false) ||
      !ler_texto("CPF (11 digitos): ", cpf, sizeof(cpf), false) ||
      !ler_texto("Telefone principal: ", telefone_um, sizeof(telefone_um),
                 false) ||
      !ler_texto("Telefone adicional (opcional): ", telefone_dois,
                 sizeof(telefone_dois), true) ||
      !ler_texto("Estado civil (Solteiro/Casado/Divorciado/Viuvo): ",
                 estado_civil, sizeof(estado_civil), false) ||
      !ler_texto("Tipo (Proprietario/Inquilino): ", tipo, sizeof(tipo),
                 false)) {
    mysql_close(conexao);
    return;
  }
  if (strlen(cpf) != 11 ||
      (strcmp(tipo, "Proprietario") != 0 && strcmp(tipo, "Inquilino") != 0)) {
    puts("CPF ou tipo de cliente invalido.");
    mysql_close(conexao);
    return;
  }
  escapar_texto(conexao, nome, e_nome, sizeof(e_nome));
  escapar_texto(conexao, cpf, e_cpf, sizeof(e_cpf));
  escapar_texto(conexao, telefone_um, e_tel1, sizeof(e_tel1));
  escapar_texto(conexao, telefone_dois, e_tel2, sizeof(e_tel2));
  escapar_texto(conexao, estado_civil, e_estado, sizeof(e_estado));

  if (!executar_comando(conexao, "START TRANSACTION", "iniciar a transacao"))
    goto fim;
  if (telefone_dois[0]) {
    snprintf(query, sizeof(query),
             "INSERT INTO "
             "cliente(nome,cpf,telefone_um,telefone_dois,estado_civil,tipo_"
             "cliente) VALUES('%s','%s','%s','%s','%s','%s')",
             e_nome, e_cpf, e_tel1, e_tel2, e_estado, tipo);
  } else {
    snprintf(query, sizeof(query),
             "INSERT INTO "
             "cliente(nome,cpf,telefone_um,telefone_dois,estado_civil,tipo_"
             "cliente) VALUES('%s','%s','%s',NULL,'%s','%s')",
             e_nome, e_cpf, e_tel1, e_estado, tipo);
  }
  if (!executar_comando(conexao, query, "cadastrar o cliente"))
    goto rollback;

  unsigned long id = (unsigned long)mysql_insert_id(conexao);
  if (strcmp(tipo, "Proprietario") == 0) {
    char certidao[41], data[11], e_certidao[81], e_data[21];
    if (!ler_texto("Certidao de registro: ", certidao, sizeof(certidao),
                   false) ||
        !ler_texto("Data do registro (AAAA-MM-DD): ", data, sizeof(data),
                   false))
      goto rollback;
    escapar_texto(conexao, certidao, e_certidao, sizeof(e_certidao));
    escapar_texto(conexao, data, e_data, sizeof(e_data));
    snprintf(query, sizeof(query),
             "INSERT INTO proprietario VALUES(%lu,'%s','%s')", id, e_certidao,
             e_data);
  } else {
    char profissao[51], e_profissao[101];
    double renda;
    if (!ler_texto("Profissao: ", profissao, sizeof(profissao), false) ||
        !ler_decimal("Renda familiar: ", 0.01, 9999999999.99, &renda))
      goto rollback;
    escapar_texto(conexao, profissao, e_profissao, sizeof(e_profissao));
    snprintf(query, sizeof(query),
             "INSERT INTO inquilino VALUES(%lu,'%s',%.2f)", id, e_profissao,
             renda);
  }
  if (!executar_comando(conexao, query, "cadastrar os dados complementares"))
    goto rollback;
  if (executar_comando(conexao, "COMMIT", "confirmar a transacao"))
    puts("Cliente cadastrado com sucesso.");
  goto fim;

rollback:
  executar_comando(conexao, "ROLLBACK", "desfazer a transacao");
fim:
  mysql_close(conexao);
}

void listar_clientes(void) {
  MYSQL *conexao = conectar();
  if (!conexao)
    return;
  const char *sql =
      "SELECT "
      "c.id_cliente,c.nome,c.cpf,c.telefone_um,c.telefone_dois,c.estado_civil,"
      "c.tipo_cliente,"
      "p.certidao_registro,i.profissao,i.renda_familiar FROM cliente c LEFT "
      "JOIN proprietario p ON p.id_proprietario=c.id_cliente "
      "LEFT JOIN inquilino i ON i.id_inquilino=c.id_cliente ORDER BY "
      "c.id_cliente";
  MYSQL_RES *r = executar_consulta(conexao, sql, "listar os clientes");
  if (r) {
    MYSQL_ROW linha;
    puts("\nID | Nome | CPF | Telefone(s) | Estado civil | Tipo | Complemento");
    while ((linha = mysql_fetch_row(r))) {
      printf("%s | %s | %s | %s / %s | %s | %s | ", linha[0], linha[1],
             linha[2], linha[3], campo_ou(linha[4], "-"), linha[5], linha[6]);
      if (strcmp(linha[6], "Proprietario") == 0)
        printf("certidao %s\n", campo_ou(linha[7], "-"));
      else
        printf("%s; renda R$ %s\n", campo_ou(linha[8], "-"),
               campo_ou(linha[9], "-"));
    }
    mysql_free_result(r);
  }
  mysql_close(conexao);
}

void modificar_cliente(void) {
  int id, opcao;
  char valor[201], escapado[401], query[800];
  if (!ler_inteiro("ID do cliente: ", 1, INT_MAX, &id) ||
      !ler_inteiro("Campo (1-Nome, 2-Telefone principal, 3-Telefone adicional, "
                   "4-Estado civil): ",
                   1, 4, &opcao) ||
      !ler_texto("Novo valor: ", valor, sizeof(valor), opcao == 3))
    return;
  MYSQL *conexao = conectar();
  if (!conexao)
    return;
  escapar_texto(conexao, valor, escapado, sizeof(escapado));
  const char *campo[] = {"", "nome", "telefone_um", "telefone_dois",
                         "estado_civil"};
  if (opcao == 3 && valor[0] == '\0')
    snprintf(query, sizeof(query),
             "UPDATE cliente SET telefone_dois=NULL WHERE id_cliente=%d", id);
  else
    snprintf(query, sizeof(query),
             "UPDATE cliente SET %s='%s' WHERE id_cliente=%d", campo[opcao],
             escapado, id);
  if (executar_comando(conexao, query, "atualizar o cliente"))
    puts(mysql_affected_rows(conexao)
             ? "Cliente atualizado."
             : "Cliente nao encontrado ou sem alteracoes.");
  mysql_close(conexao);
}

void excluir_cliente(void) {
  int id;
  if (!ler_inteiro("ID do cliente: ", 1, INT_MAX, &id) ||
      !confirmar("Confirmar exclusao? (s/N): "))
    return;
  MYSQL *conexao = conectar();
  if (!conexao)
    return;
  char query[128];
  snprintf(query, sizeof(query), "DELETE FROM cliente WHERE id_cliente=%d", id);
  if (executar_comando(conexao, query, "excluir o cliente"))
    puts(mysql_affected_rows(conexao) ? "Cliente excluido."
                                      : "Cliente nao encontrado.");
  mysql_close(conexao);
}
