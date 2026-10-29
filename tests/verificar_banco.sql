USE imobiliaria;

SELECT IF(
  (SELECT COUNT(*) FROM cliente) = 6
  AND (SELECT COUNT(*) FROM contrato) = 2
  AND (SELECT COUNT(*) FROM contrato WHERE YEAR(data_contrato) = 2022) = 1
  AND (SELECT COUNT(*) FROM imovel WHERE situacao = 'Indisponivel') = 2,
  'OK', 'FALHA'
) AS resultado;
