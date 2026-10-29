#!/usr/bin/env python3
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
fontes = "\n".join(path.read_text(encoding="utf-8") for path in RAIZ.glob("*.c"))
sql = (RAIZ / "banco-de-dados-imobiliaria.sql").read_text(encoding="utf-8")

assert "scanf(" not in fontes, "Use as funcoes centralizadas de entrada em vez de scanf."
assert "mysql_real_escape_string" in fontes, "Textos SQL precisam ser escapados."
assert "data_contrato" in sql, "Contratos precisam registrar a propria data."
assert "trg_contrato_validar" in sql, "As regras de contrato precisam existir no banco."
assert "DEFINER=" not in sql, "O esquema nao deve depender de um usuario MySQL local."
assert "cp850" not in sql.lower(), "O esquema deve permanecer em UTF-8."
print("Validacoes estaticas concluidas com sucesso.")
