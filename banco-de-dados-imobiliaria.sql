-- Esquema e dados de demonstracao do sistema de gestao imobiliaria.
CREATE DATABASE IF NOT EXISTS imobiliaria
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE imobiliaria;

SET FOREIGN_KEY_CHECKS = 0;
DROP TRIGGER IF EXISTS trg_contrato_validar;
DROP TRIGGER IF EXISTS trg_contrato_indisponibilizar_imovel;
DROP TRIGGER IF EXISTS trg_contrato_restaurar_imovel;
DROP TABLE IF EXISTS contrato;
DROP TABLE IF EXISTS proposta;
DROP TABLE IF EXISTS visita;
DROP TABLE IF EXISTS imovel;
DROP TABLE IF EXISTS fiador;
DROP TABLE IF EXISTS corretor;
DROP TABLE IF EXISTS inquilino;
DROP TABLE IF EXISTS proprietario;
DROP TABLE IF EXISTS cliente;
SET FOREIGN_KEY_CHECKS = 1;

CREATE TABLE cliente (
  id_cliente INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  nome VARCHAR(200) NOT NULL,
  cpf CHAR(11) NOT NULL UNIQUE,
  telefone_um VARCHAR(20) NOT NULL,
  telefone_dois VARCHAR(20),
  estado_civil ENUM('Solteiro','Casado','Divorciado','Viuvo') NOT NULL,
  tipo_cliente ENUM('Proprietario','Inquilino') NOT NULL
) ENGINE=InnoDB;

CREATE TABLE proprietario (
  id_proprietario INT UNSIGNED PRIMARY KEY,
  certidao_registro VARCHAR(40) NOT NULL UNIQUE,
  data_registro DATE NOT NULL,
  CONSTRAINT fk_proprietario_cliente FOREIGN KEY (id_proprietario)
    REFERENCES cliente(id_cliente) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE inquilino (
  id_inquilino INT UNSIGNED PRIMARY KEY,
  profissao VARCHAR(50) NOT NULL,
  renda_familiar DECIMAL(12,2) UNSIGNED NOT NULL,
  CONSTRAINT fk_inquilino_cliente FOREIGN KEY (id_inquilino)
    REFERENCES cliente(id_cliente) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE corretor (
  id_corretor INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  nome VARCHAR(200) NOT NULL,
  creci VARCHAR(50) NOT NULL UNIQUE,
  telefone VARCHAR(20) NOT NULL UNIQUE,
  data_prestacao DATE NOT NULL,
  comissao DECIMAL(5,2) UNSIGNED NOT NULL,
  CONSTRAINT chk_corretor_comissao CHECK (comissao <= 100)
) ENGINE=InnoDB;

CREATE TABLE fiador (
  id_fiador INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  id_inquilino INT UNSIGNED NOT NULL,
  nome VARCHAR(100) NOT NULL,
  telefone VARCHAR(20) NOT NULL,
  saldo_bancario DECIMAL(12,2) UNSIGNED NOT NULL,
  INDEX idx_fiador_inquilino (id_inquilino),
  CONSTRAINT fk_fiador_inquilino FOREIGN KEY (id_inquilino)
    REFERENCES inquilino(id_inquilino) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE imovel (
  id_imovel INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  id_proprietario INT UNSIGNED NOT NULL,
  id_corretor INT UNSIGNED,
  tipo ENUM('Casa','Apartamento','Kitnet') NOT NULL,
  num_comodos SMALLINT UNSIGNED NOT NULL,
  vagas_garagem SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  area DECIMAL(8,2) UNSIGNED NOT NULL,
  cidade VARCHAR(50) NOT NULL,
  bairro VARCHAR(50) NOT NULL,
  rua VARCHAR(150) NOT NULL,
  numero VARCHAR(10) NOT NULL,
  valor_aluguel DECIMAL(12,2) UNSIGNED NOT NULL,
  data_oferta DATE NOT NULL,
  situacao ENUM('Disponivel','Indisponivel') NOT NULL DEFAULT 'Disponivel',
  INDEX idx_imovel_proprietario (id_proprietario),
  INDEX idx_imovel_corretor (id_corretor),
  INDEX idx_imovel_valor (valor_aluguel),
  CONSTRAINT fk_imovel_proprietario FOREIGN KEY (id_proprietario)
    REFERENCES proprietario(id_proprietario) ON DELETE RESTRICT,
  CONSTRAINT fk_imovel_corretor FOREIGN KEY (id_corretor)
    REFERENCES corretor(id_corretor) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE visita (
  id_visita INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  id_inquilino INT UNSIGNED NOT NULL,
  id_imovel INT UNSIGNED NOT NULL,
  id_corretor INT UNSIGNED NOT NULL,
  data_visita DATETIME NOT NULL,
  UNIQUE KEY uk_visita_horario_corretor (id_corretor, data_visita),
  INDEX idx_visita_imovel (id_imovel),
  CONSTRAINT fk_visita_inquilino FOREIGN KEY (id_inquilino)
    REFERENCES inquilino(id_inquilino) ON DELETE CASCADE,
  CONSTRAINT fk_visita_imovel FOREIGN KEY (id_imovel)
    REFERENCES imovel(id_imovel) ON DELETE CASCADE,
  CONSTRAINT fk_visita_corretor FOREIGN KEY (id_corretor)
    REFERENCES corretor(id_corretor) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE proposta (
  id_proposta INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  id_inquilino INT UNSIGNED NOT NULL,
  id_imovel INT UNSIGNED NOT NULL,
  valor_proposta DECIMAL(12,2) UNSIGNED NOT NULL,
  data_proposta DATE NOT NULL,
  validade DATE NOT NULL,
  INDEX idx_proposta_inquilino (id_inquilino),
  INDEX idx_proposta_imovel (id_imovel),
  CONSTRAINT chk_proposta_validade CHECK (validade >= data_proposta),
  CONSTRAINT fk_proposta_inquilino FOREIGN KEY (id_inquilino)
    REFERENCES inquilino(id_inquilino) ON DELETE RESTRICT,
  CONSTRAINT fk_proposta_imovel FOREIGN KEY (id_imovel)
    REFERENCES imovel(id_imovel) ON DELETE RESTRICT
) ENGINE=InnoDB;

CREATE TABLE contrato (
  id_contrato INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  id_proposta INT UNSIGNED NOT NULL UNIQUE,
  duracao SMALLINT UNSIGNED NOT NULL COMMENT 'Duracao em meses',
  valor_inicial DECIMAL(12,2) UNSIGNED NOT NULL,
  data_contrato DATE NOT NULL,
  CONSTRAINT chk_contrato_duracao CHECK (duracao > 0),
  CONSTRAINT fk_contrato_proposta FOREIGN KEY (id_proposta)
    REFERENCES proposta(id_proposta) ON DELETE RESTRICT
) ENGINE=InnoDB;

DELIMITER $$
CREATE TRIGGER trg_contrato_validar
BEFORE INSERT ON contrato FOR EACH ROW
BEGIN
  IF (SELECT COUNT(*) FROM proposta WHERE id_proposta = NEW.id_proposta) = 0 THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Proposta inexistente';
  END IF;
  IF NEW.data_contrato > (SELECT validade FROM proposta WHERE id_proposta = NEW.id_proposta) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Proposta vencida na data do contrato';
  END IF;
  IF (SELECT COUNT(*) FROM fiador f JOIN proposta p ON p.id_inquilino = f.id_inquilino
      WHERE p.id_proposta = NEW.id_proposta) < 2 THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'O inquilino precisa de pelo menos dois fiadores';
  END IF;
  IF EXISTS (
    SELECT 1 FROM contrato c
    JOIN proposta existente ON existente.id_proposta = c.id_proposta
    JOIN proposta nova ON nova.id_proposta = NEW.id_proposta
    WHERE existente.id_imovel = nova.id_imovel
  ) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'O imovel ja possui contrato';
  END IF;
END$$

CREATE TRIGGER trg_contrato_indisponibilizar_imovel
AFTER INSERT ON contrato FOR EACH ROW
BEGIN
  UPDATE imovel SET situacao = 'Indisponivel'
  WHERE id_imovel = (SELECT id_imovel FROM proposta WHERE id_proposta = NEW.id_proposta);
END$$

CREATE TRIGGER trg_contrato_restaurar_imovel
AFTER DELETE ON contrato FOR EACH ROW
BEGIN
  UPDATE imovel SET situacao = 'Disponivel'
  WHERE id_imovel = (SELECT id_imovel FROM proposta WHERE id_proposta = OLD.id_proposta)
    AND NOT EXISTS (
      SELECT 1 FROM contrato c JOIN proposta p ON p.id_proposta = c.id_proposta
      WHERE p.id_imovel = imovel.id_imovel
    );
END$$
DELIMITER ;

INSERT INTO cliente (id_cliente,nome,cpf,telefone_um,telefone_dois,estado_civil,tipo_cliente) VALUES
  (1,'Enzo Basaldella','09642517037','2196166370',NULL,'Divorciado','Proprietario'),
  (2,'Jurair Rosa','05650928088','2299509835','2493608190','Solteiro','Inquilino'),
  (3,'Laura Assis','06277128361','2399852540','2291235943','Solteiro','Proprietario'),
  (4,'Gustavo Kanji','05416297828','2297351912',NULL,'Casado','Inquilino'),
  (5,'Nicolas Nogueira','09330297894','2395972415','2297992826','Solteiro','Proprietario'),
  (6,'Luis Tarrataca','04783358772','2394054341','2194445825','Casado','Inquilino');

INSERT INTO proprietario VALUES
  (1,'REG-001','2016-01-10'),(3,'REG-003','2018-06-15'),(5,'REG-005','2020-03-20');
INSERT INTO inquilino VALUES
  (2,'Medica',16800.00),(4,'Engenheiro',9500.75),(6,'Professor',4500.00);
INSERT INTO corretor VALUES
  (1,'Rodrigo Lima','CRECI-RJ 2486 F','2196575901','2020-04-07',6.57),
  (2,'Carolina Braga','CRECI-RJ 5891 F','2392706242','2020-08-15',5.15),
  (3,'Aline Silverio','CRECI-RJ 3742 F','2193890256','2017-01-15',4.07);
INSERT INTO fiador (id_inquilino,nome,telefone,saldo_bancario) VALUES
  (2,'Adailson Cardoso','2497391686',420000.00),(2,'Camila Figo','2391436264',32000.00),
  (4,'Athos Fernandes','2397698934',15000.00),(4,'Renato Campos','2191494026',18000.00),
  (6,'Carla Albuquerque','2294747338',22000.00),(6,'Felipe Siqueira','2193591106',28000.00);
INSERT INTO imovel (id_imovel,id_proprietario,id_corretor,tipo,num_comodos,vagas_garagem,area,cidade,bairro,rua,numero,valor_aluguel,data_oferta) VALUES
  (1,1,3,'Casa',5,3,170.63,'Sao Pedro','Centro','Rua das Palmeiras','332',3300.00,'2017-08-05'),
  (2,3,2,'Apartamento',5,2,91.69,'Petropolis','Centro','Rua do Imperador','593',1677.61,'2019-08-24'),
  (3,1,3,'Casa',2,3,141.92,'Petropolis','Centro','13 de Marco','290',3674.87,'2016-01-04'),
  (4,5,NULL,'Kitnet',1,0,37.98,'Petropolis','Centro','Paulo Barbosa','464',1434.65,'2020-08-04'),
  (5,5,1,'Apartamento',3,1,78.30,'Petropolis','Centro','Rua do Imperador','125',2100.00,'2022-11-30');
INSERT INTO visita (id_inquilino,id_imovel,id_corretor,data_visita) VALUES
  (2,1,3,'2022-01-20 10:00:00'),(4,2,2,'2024-05-10 14:30:00'),(6,1,3,'2024-06-12 09:00:00');
INSERT INTO proposta (id_proposta,id_inquilino,id_imovel,valor_proposta,data_proposta,validade) VALUES
  (1,2,1,3200.00,'2022-01-21','2022-02-21'),
  (2,4,2,1600.00,'2024-05-11','2024-06-11'),
  (3,6,5,2000.00,'2026-10-20','2026-12-20');
INSERT INTO contrato (id_proposta,duracao,valor_inicial,data_contrato) VALUES
  (1,12,3200.00,'2022-01-25'),(2,18,1600.00,'2024-05-20');
