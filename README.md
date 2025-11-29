# CookBookCLI

Sistema de gerenciamento de receitas em linha de comando (CLI) desenvolvido em C++17 com SQLite.

## Características

- CRUD completo de receitas
- Banco de dados SQLite persistente
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

1. **Cadastrar receita**: Adiciona uma nova receita ao banco de dados
2. **Listar receitas**: Exibe todas as receitas cadastradas
3. **Consultar detalhes por ID**: Mostra informações completas de uma receita específica
4. **Buscar por nome ou parte do nome**: Busca receitas que contenham o termo pesquisado
5. **Excluir receita**: Remove uma receita do banco de dados
0. **Sair**: Encerra o programa

## Estrutura do Banco de Dados

O banco de dados SQLite é criado automaticamente em `./data/recipes.db` com a seguinte estrutura:

```sql
CREATE TABLE receitas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nome TEXT NOT NULL,
    ingredientes TEXT NOT NULL,
    preparo TEXT NOT NULL,
    tempo INTEGER,
    categoria TEXT,
    porcoes INTEGER
);
```

## Persistência de Dados

- **Build local**: O banco de dados é criado em `./data/recipes.db`
- **Docker**: O volume `./data` é mapeado para `/app/data` no container, garantindo persistência dos dados mesmo após remover o container

## Desenvolvimento

### Classes Principais

- **`Receita`**: Struct que representa uma receita com todos os seus campos
- **`Database`**: Classe responsável por gerenciar conexão e operações CRUD no SQLite

### Compilação

O projeto usa CMake para gerenciar o build. O executável gerado se chama `cookbook`.

## Licença

Este projeto é fornecido como está, para fins educacionais e de demonstração.
