# CookBookCLI

Sistema de gerenciamento de receitas em linha de comando (CLI) desenvolvido em C++17 com SQLite.

## Características

- CRUD completo de receitas
- Sistema de tags filtráveis (múltiplas tags por receita)
- Marcar receitas como feitas
- Sistema de avaliação (notas de 1 a 5)
- Filtros avançados (por tag, por nota, receitas feitas)
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
   - Permite adicionar tags durante o cadastro
   - Permite marcar se a receita já foi feita
2. **Listar receitas**: Exibe todas as receitas cadastradas (com tags, status e nota)
3. **Consultar detalhes por ID**: Mostra informações completas de uma receita específica
4. **Buscar por nome ou parte do nome**: Busca receitas que contenham o termo pesquisado
5. **Excluir receita**: Remove uma receita do banco de dados

### Gerenciamento de Tags
6. **Adicionar tag em uma receita**: Associa uma ou múltiplas tags a uma receita existente
7. **Remover tag de uma receita**: Remove uma tag específica de uma receita
8. **Listar tags disponíveis**: Exibe todas as tags cadastradas no sistema
9. **Filtrar receitas por tag**: Lista todas as receitas que possuem uma tag específica

### Status e Avaliação
10. **Marcar receita como feita/não feita**: Altera o status de conclusão da receita
11. **Listar receitas feitas**: Filtra e exibe apenas receitas que já foram preparadas
12. **Avaliar receita (1-5)**: Atribui uma nota de 1 a 5 para receitas que foram feitas
13. **Filtrar receitas por nota**: Lista receitas feitas com uma nota específica

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
    nota INTEGER DEFAULT 0
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

## Desenvolvimento

### Classes Principais

- **`Receita`**: Struct que representa uma receita com todos os seus campos:
  - Informações básicas (nome, ingredientes, preparo, tempo, categoria, porcoes)
  - Status (feita: bool)
  - Avaliação (nota: int, 0 = não avaliada, 1-5 = nota)
  - Tags (std::vector<std::string>)

- **`Database`**: Classe responsável por gerenciar conexão e operações no SQLite:
  - CRUD de receitas
  - Gerenciamento de tags (criar, listar, associar, remover)
  - Filtros (por tag, por nota, receitas feitas)
  - Avaliação de receitas
  - Marcação de status (feita/não feita)

### Regras de Negócio

- **Tags**: 
  - Múltiplas tags podem ser associadas a uma receita
  - Tags são criadas automaticamente se não existirem
  - Tags são únicas no sistema (não duplicadas)

- **Avaliação**:
  - Apenas receitas marcadas como "feitas" podem ser avaliadas
  - Nota deve estar entre 1 e 5
  - Nota 0 significa "não avaliada"

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
   Deseja adicionar tags? (s/n): s
   Digite as tags separadas por virgula: doce, chocolate, sobremesa
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

## Licença

Este projeto é fornecido como está, para fins educacionais e de demonstração.
