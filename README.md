# Sistema para Imobiliária — Banco de Dados (CEFET/RJ)

Aplicação de terminal em C integrada ao MySQL para administrar clientes, proprietários, inquilinos, fiadores, corretores, imóveis, visitas, propostas e contratos. O projeto foi desenvolvido para a disciplina de Banco de Dados do CEFET/RJ.

> [!NOTE]
> O enunciado original está preservado em [`Projeto - Enunciado.pdf`](./Projeto%20-%20Enunciado.pdf). Esta versão reorganiza a implementação, mantém a interface em português e reforça as regras de integridade pedidas no trabalho.

## Funcionalidades

- cadastros e manutenção das entidades da imobiliária;
- agendamento de visitas com o corretor responsável pelo imóvel;
- registro de propostas para imóveis disponíveis;
- criação transacional de contratos a partir de propostas válidas;
- atualização automática da disponibilidade do imóvel por gatilhos;
- oito relatórios gerenciais, incluindo contratos de 2022 e rendimento anual de corretores;
- dados de demonstração prontos para uso.

## Tecnologias

- C11;
- MySQL 8.4;
- MySQL C API (`libmysqlclient`);
- Make e Docker Compose;
- GitHub Actions para compilação e validação do esquema.

## Execução rápida com Docker

Suba o MySQL já inicializado com o esquema e os dados de exemplo:

```bash
docker compose up -d
```

Configure a aplicação:

```bash
cp .env.example .env
set -a
source .env
set +a
```

Instale a biblioteca de desenvolvimento do MySQL, compile e execute. Em Debian ou Ubuntu:

```bash
sudo apt install build-essential default-libmysqlclient-dev
make
./imobiliaria
```

> [!TIP]
> Para recriar o banco desde o início, execute `docker compose down -v` e depois `docker compose up -d`.

## Execução com MySQL local

Importe o esquema em uma instância MySQL 8:

```bash
mysql -u root -p < banco-de-dados-imobiliaria.sql
```

A conexão é configurada exclusivamente por variáveis de ambiente:

| Variável | Padrão | Descrição |
| --- | --- | --- |
| `IMOB_DB_HOST` | `127.0.0.1` | Servidor MySQL |
| `IMOB_DB_PORT` | `3306` | Porta do servidor |
| `IMOB_DB_USER` | `root` | Usuário do banco |
| `IMOB_DB_PASSWORD` | `null` | Senha do usuário |
| `IMOB_DB_NAME` | `imobiliaria` | Banco utilizado |

O arquivo `.env` é ignorado pelo Git e não deve ser publicado.

## Estrutura

```text
.
├── banco-de-dados-imobiliaria.sql  # esquema, gatilhos e dados de exemplo
├── cliente.c / .h                  # proprietários e inquilinos
├── corretor.c / .h                 # corretores
├── fiador.c / .h                   # fiadores
├── imovel.c / .h                   # imóveis
├── negociacao.c / .h               # visitas, propostas e contratos
├── consulta.c / .h                 # relatórios gerenciais
├── conexao.c / .h                  # acesso seguro ao MySQL
├── entrada.c / .h                  # leitura e validação da entrada
├── docs/MODELO_DADOS.md            # diagrama e regras do modelo
└── tests/                          # verificações estáticas e do banco
```

## Modelo e integridade

O diagrama entidade-relacionamento e as principais decisões estão em [`docs/MODELO_DADOS.md`](docs/MODELO_DADOS.md). As operações compostas usam transações, os textos são escapados antes de entrar em consultas SQL e o banco também valida as regras críticas por gatilhos e restrições.

## Validação

```bash
make static-check
```

Esse comando compila sintaticamente todos os módulos com avisos tratados como erros e executa verificações de padronização sem exigir uma instalação local do MySQL. A integração contínua também compila contra a biblioteca real, importa o esquema em MySQL 8.4 e confere os dados esperados.

## Limitações acadêmicas

- O projeto usa a API procedural do MySQL; em produção, consultas preparadas seriam preferíveis.
- Contratos ainda não possuem encerramento, reajustes ou pagamentos mensais.
- As datas são validadas definitivamente pelo MySQL, depois da leitura textual no terminal.
- Não há autenticação nem níveis de acesso, pois o foco do trabalho é a modelagem relacional.
