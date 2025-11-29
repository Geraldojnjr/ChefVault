# CookBookCLI

Sistema de gerenciamento de receitas em linha de comando (CLI) desenvolvido em C++17 com SQLite.

## Características

- CRUD completo de receitas
- Sistema de tags filtráveis (múltiplas tags por receita)
- **Autocompletar de tags** - Sugestões inteligentes ao digitar tags
- Marcar receitas como feitas
- Sistema de avaliação (notas de 1 a 5)
- Campo de imagem para receitas
- Filtros avançados (por tag, por nota, receitas feitas)
- **Backup e restauração** do banco de dados
- Banco de dados SQLite persistente com foreign keys
- Interface CLI interativa
- Containerização com Docker
- Build com CMake

## Estrutura do Projeto

```
CookBookCLI/
├── src/              # Código fonte
│   ├── main.cpp      # Menu principal e interação
│   └── Database.cpp  # Implementação do banco de dados
├── include/          # Headers
│   ├── Receita.h     # Estrutura de dados Receita
│   └── Database.h    # Classe Database
├── data/             # Diretório do banco de dados (recipes.db)
├── CMakeLists.txt    # Configuração CMake
├── Dockerfile        # Multi-stage build Docker
├── docker-compose.yml # Configuração Docker Compose
└── README.md         # Este arquivo
```

## Requisitos

### Build Local

- C++17 ou superior
- CMake 3.10 ou superior
- SQLite3 (biblioteca e headers)
- Compilador C++ (g++, clang++, etc.)

### Docker

- Docker
- Docker Compose (opcional)

## Build Local

### Linux

1. Instalar dependências:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libsqlite3-dev
```

2. Compilar o projeto:
```bash
mkdir build
cd build
cmake ..
make
```

3. Executar:
```bash
./cookbook
```

### Windows

1. Instalar dependências:
   - CMake: https://cmake.org/download/
   - SQLite3: https://www.sqlite.org/download.html

2. Compilar:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

3. Executar:
```bash
cookbook.exe
```

## Executar com Docker

### Build e execução manual

1. Build da imagem:
```bash
docker build -t cookbook-cli .
```

2. Executar container:
```bash
docker run -it --rm -v ./data:/app/data cookbook-cli
```

### Executar com Docker Compose

1. Build e execução:
```bash
docker-compose up --build
```

2. Para executar em background:
```bash
docker-compose up -d
```

3. Para ver logs:
```bash
docker-compose logs -f
```

4. Para parar:
```bash
docker-compose down
```

## Funcionalidades

O menu oferece as seguintes opções:

### Gerenciamento Básico
1. **Cadastrar receita**: Adiciona uma nova receita ao banco de dados
   - Permite adicionar tags durante o cadastro (com autocompletar)
   - Permite marcar se a receita já foi feita
   - Permite adicionar caminho de imagem da receita
2. **Listar receitas**: Exibe todas as receitas cadastradas (com tags, status e nota)
3. **Consultar detalhes por ID**: Mostra informações completas de uma receita específica
   - Exibe imagem se disponível
4. **Buscar por nome ou parte do nome**: Busca receitas que contenham o termo pesquisado
5. **Excluir receita**: Remove uma receita do banco de dados

### Gerenciamento de Tags
6. **Adicionar tag em uma receita**: Associa uma ou múltiplas tags a uma receita existente
   - **Autocompletar**: Digite `?` para ver todas as tags disponíveis
   - **Sugestões automáticas**: Mostra sugestões quando a tag não é encontrada exatamente
7. **Remover tag de uma receita**: Remove uma tag específica de uma receita
   - **Autocompletar**: Digite `?` para ver as tags da receita
   - **Busca por prefixo**: Se não encontrar exato, busca por prefixo automaticamente
8. **Listar tags disponíveis**: Exibe todas as tags cadastradas no sistema
9. **Filtrar receitas por tag**: Lista todas as receitas que possuem uma tag específica
   - **Autocompletar**: Digite `?` para ver todas as tags disponíveis
   - **Sugestões automáticas**: Mostra sugestões quando a tag não é encontrada

### Status e Avaliação
10. **Marcar receita como feita/não feita**: Altera o status de conclusão da receita
11. **Listar receitas feitas**: Filtra e exibe apenas receitas que já foram preparadas
12. **Avaliar receita (1-5)**: Atribui uma nota de 1 a 5 para receitas que foram feitas
13. **Filtrar receitas por nota**: Lista receitas feitas com uma nota específica

### Backup e Restauração
98. **Fazer backup do banco de dados**: Cria um backup completo do banco de dados com timestamp
99. **Restaurar backup do banco de dados**: Restaura o banco de dados a partir de um backup
   - Lista backups disponíveis automaticamente
   - Permite selecionar por número (1, 2, 3...) ou caminho completo
   - Cria backup de segurança antes de restaurar

0. **Sair**: Encerra o programa

## Estrutura do Banco de Dados

O banco de dados SQLite é criado automaticamente em `./data/recipes.db` com a seguinte estrutura:

### Tabela `receitas`
```sql
CREATE TABLE receitas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nome TEXT NOT NULL,
    ingredientes TEXT NOT NULL,
    preparo TEXT NOT NULL,
    tempo INTEGER,
    categoria TEXT,
    porcoes INTEGER,
    feita INTEGER DEFAULT 0,
    nota INTEGER DEFAULT 0,
    imagem TEXT
);
```

### Tabela `tags`
```sql
CREATE TABLE tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nome TEXT UNIQUE NOT NULL
);
```

### Tabela `receitas_tags` (relacionamento muitos-para-muitos)
```sql
CREATE TABLE receitas_tags (
    receita_id INTEGER,
    tag_id INTEGER,
    PRIMARY KEY (receita_id, tag_id),
    FOREIGN KEY (receita_id) REFERENCES receitas(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
);
```

### Características
- **Foreign keys habilitadas**: Integridade referencial garantida
- **CASCADE**: Exclusão automática de relacionamentos ao deletar receitas ou tags
- **Migration automática**: Colunas novas são adicionadas automaticamente em bancos existentes

## Persistência de Dados

- **Build local**: O banco de dados é criado em `./data/recipes.db`
- **Docker**: O volume `./data` é mapeado para `/app/data` no container, garantindo persistência dos dados mesmo após remover o container

## Testes

O projeto inclui um sistema completo de testes automatizados que verifica todas as funcionalidades.

### Executar Testes

Após compilar o projeto, você tem várias opções:

#### Opção 1: Testes com saída detalhada (recomendado)
Mostra cada um dos 17 testes individuais e se passou ou falhou:

```bash
cd build
make test-verbose
```

#### Opção 2: Testes resumidos
Mostra apenas o resultado geral:

```bash
cd build
make test
```

#### Opção 3: Executar diretamente
Executa o executável de teste diretamente:

```bash
cd build
./test_chefvault
```

### Cobertura de Testes

Os testes cobrem:

#### Funcionalidades Antigas (CRUD Básico)
-  Criar receita
-  Listar receitas
-  Consultar receita por ID
-  Buscar receita por nome
-  Excluir receita

#### Funcionalidades Novas - Tags
-  Criar tag
-  Listar tags
-  Adicionar tag a receita
-  Remover tag de receita
-  Filtrar receitas por tag

#### Funcionalidades Novas - Status Feita
-  Marcar receita como feita
-  Listar receitas feitas

#### Funcionalidades Novas - Avaliação
-  Avaliar receita (1-5)
-  Validar que apenas receitas feitas podem ser avaliadas
-  Filtrar receitas por nota
-  Validar nota inválida (fora do range 1-5)

**Total: 17 testes automatizados**

Os testes usam um banco de dados temporário (`test_recipes.db`) que é criado e removido automaticamente durante a execução.

## Desenvolvimento

### Classes Principais

- **`Receita`**: Struct que representa uma receita com todos os seus campos:
  - Informações básicas (nome, ingredientes, preparo, tempo, categoria, porcoes)
  - Status (feita: bool)
  - Avaliação (nota: int, 0 = não avaliada, 1-5 = nota)
  - Imagem (std::string) - caminho para imagem da receita
  - Tags (std::vector<std::string>)

- **`Database`**: Classe responsável por gerenciar conexão e operações no SQLite:
  - CRUD de receitas
  - Gerenciamento de tags (criar, listar, associar, remover)
  - **Busca de tags por prefixo** (para autocompletar)
  - Filtros (por tag, por nota, receitas feitas)
  - Avaliação de receitas
  - Marcação de status (feita/não feita)
  - **Backup e restauração** do banco de dados

### Regras de Negócio

- **Tags**: 
  - Múltiplas tags podem ser associadas a uma receita
  - Tags são criadas automaticamente se não existirem
  - Tags são únicas no sistema (não duplicadas)
  - **Autocompletar**: Sistema inteligente de sugestões
    - Digite `?` para ver todas as tags disponíveis
    - Digite parcialmente uma tag para ver sugestões (até 10 sugestões)
    - Se houver apenas 1 sugestão, ela é selecionada automaticamente
    - Funciona em todas as operações que envolvem tags

- **Avaliação**:
  - Apenas receitas marcadas como "feitas" podem ser avaliadas
  - Nota deve estar entre 1 e 5
  - Nota 0 significa "não avaliada"

- **Backup e Restauração**:
  - Backups são criados com timestamp automático
  - Backup inclui todas as tabelas (receitas, tags, relacionamentos)
  - Restauração valida integridade do backup antes de aplicar
  - Cria backup de segurança antes de restaurar
  - Permite seleção por número ou caminho completo

- **Segurança**:
  - Todas as queries SQL usam prepared statements (proteção contra SQL injection)
  - Foreign keys garantem integridade referencial

### Compilação

O projeto usa CMake para gerenciar o build. O executável gerado se chama `cookbook`.

## Exemplos de Uso

### Cadastrar uma receita com tags
```
1. Cadastrar receita
   Nome: Bolo de Chocolate
   Ingredientes: ...
   Modo de preparo: ...
   Tempo: 60
   Categoria: Sobremesa
   Porcoes: 8
   Ja foi feita? (s/n): n
   Caminho da imagem (ou Enter para pular): ./imagens/bolo_chocolate.jpg
   Deseja adicionar tags? (s/n): s
   Digite as tags separadas por virgula.
   Dica: Digite ? para ver todas as tags disponiveis.
   Tags: doce, chocolate, sobremesa
```

### Usar autocompletar de tags
```
Ao adicionar tags, você pode:
- Digitar ? para ver todas as tags disponíveis
- Digitar parcialmente uma tag para ver sugestões
- O sistema mostra automaticamente sugestões quando a tag não é encontrada exatamente

Exemplo:
Tags: choc?
Sugestoes para "choc": chocolate, chocolate_branco, chocolate_amargo
```

### Filtrar receitas por tag
```
9. Filtrar receitas por tag
   Digite o nome da tag: chocolate
```

### Avaliar uma receita
```
10. Marcar receita como feita/nao feita
    (Primeiro marque como feita)
    
12. Avaliar receita (1-5)
    Digite o ID da receita: 1
    Digite a nota (1 a 5): 5
```

### Filtrar receitas por nota
```
13. Filtrar receitas por nota
    Digite a nota (1 a 5): 5
```

### Fazer backup do banco de dados
```
98. Fazer backup do banco de dados
    Caminho do backup (Enter para usar padrão): 
    Backup concluido com sucesso!
    Tamanho do arquivo: 28672 bytes
    Receitas no backup: 5
```

### Restaurar backup do banco de dados
```
99. Restaurar backup do banco de dados
    ATENCAO: Esta operacao substituira o banco de dados atual!
    
    --- Backups Disponiveis ---
    #    Arquivo                                           Tamanho        
    ----------------------------------------------------------------------
    1    recipes_backup_20251129_175515.db                 28 KB          
    2    recipes_backup_20251129_172321.db                 28 KB          
    
    Digite o numero do backup (1-2) ou o caminho completo: 1
    Backup selecionado: recipes_backup_20251129_175515.db
    Tem certeza que deseja restaurar o backup? (s/n): s
    Restaurado: 5 receitas, 10 tags, 15 relacionamentos.
```

## Licença

Este projeto é fornecido como está, para fins educacionais e de demonstração.
