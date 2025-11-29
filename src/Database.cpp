// ============================================================================
// INCLUDES
// ============================================================================
#include "../include/Database.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>

// ============================================================================
// CONSTRUTOR E DESTRUTOR
// ============================================================================
Database::Database(const std::string& path) : dbPath(path), db(nullptr) {
    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

Database::~Database() {
    close();
}

// ============================================================================
// INICIALIZAÇÃO E CONFIGURAÇÃO DO BANCO
// ============================================================================
bool Database::initialize() {
    if (sqlite3_open(dbPath.c_str(), (sqlite3**)&db) != SQLITE_OK) {
        std::cerr << "Erro ao abrir banco de dados: " << sqlite3_errmsg((sqlite3*)db) << std::endl;
        return false;
    }
    
    // Habilitar foreign keys
    if (!executeQuery("PRAGMA foreign_keys = ON;")) {
        std::cerr << "Erro ao habilitar foreign keys" << std::endl;
        return false;
    }
    
    if (!createTable()) {
        return false;
    }
    
    return createTagsTables();
}

bool Database::createTable() {
    std::string query = R"(
        CREATE TABLE IF NOT EXISTS receitas (
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
        )
    )";
    
    if (!executeQuery(query)) {
        return false;
    }
    
    // Adicionar coluna feita se a tabela já existir sem ela (migration)
    if (!columnExists("receitas", "feita")) {
        std::string alterQueryFeita = R"(
            ALTER TABLE receitas ADD COLUMN feita INTEGER DEFAULT 0
        )";
        executeQuerySilent(alterQueryFeita);
    }
    
    // Adicionar coluna nota se a tabela já existir sem ela (migration)
    if (!columnExists("receitas", "nota")) {
        std::string alterQueryNota = R"(
            ALTER TABLE receitas ADD COLUMN nota INTEGER DEFAULT 0
        )";
        executeQuerySilent(alterQueryNota);
    }
    
    if (!columnExists("receitas", "imagem")) {
        std::string alterQueryImagem = R"(
            ALTER TABLE receitas ADD COLUMN imagem TEXT
        )";
        executeQuerySilent(alterQueryImagem);
    }
    
    return true;
}

bool Database::createTagsTables() {
    std::string queryTags = R"(
        CREATE TABLE IF NOT EXISTS tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nome TEXT UNIQUE NOT NULL
        )
    )";
    
    if (!executeQuery(queryTags)) {
        return false;
    }
    
    std::string queryReceitasTags = R"(
        CREATE TABLE IF NOT EXISTS receitas_tags (
            receita_id INTEGER,
            tag_id INTEGER,
            PRIMARY KEY (receita_id, tag_id),
            FOREIGN KEY (receita_id) REFERENCES receitas(id) ON DELETE CASCADE,
            FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
        )
    )";
    
    return executeQuery(queryReceitasTags);
}

// ============================================================================
// UTILITÁRIOS DE BANCO DE DADOS
// ============================================================================
bool Database::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    sqlite3* sqliteDb = (sqlite3*)db;
    
    if (sqlite3_exec(sqliteDb, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Erro SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool Database::executeQuerySilent(const std::string& query) {
    char* errMsg = nullptr;
    sqlite3* sqliteDb = (sqlite3*)db;
    
    int result = sqlite3_exec(sqliteDb, query.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    
    return (result == SQLITE_OK);
}

bool Database::columnExists(const std::string& tableName, const std::string& columnName) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "PRAGMA table_info(?);";
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    // SQLite PRAGMA não aceita bind parameters, então vamos usar uma query diferente
    std::string query = "PRAGMA table_info(" + tableName + ");";
    sqlite3_finalize(stmt);
    
    if (sqlite3_prepare_v2(sqliteDb, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name && std::string(name) == columnName) {
            sqlite3_finalize(stmt);
            return true;
        }
    }
    
    sqlite3_finalize(stmt);
    return false;
}

// ============================================================================
// CRUD DE RECEITAS
// ============================================================================
int Database::cadastrarReceita(const Receita& receita) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "INSERT INTO receitas (nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, receita.nome.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receita.ingredientes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, receita.preparo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, receita.tempo);
    sqlite3_bind_text(stmt, 5, receita.categoria.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, receita.porcoes);
    sqlite3_bind_int(stmt, 7, receita.feita ? 1 : 0);
    sqlite3_bind_int(stmt, 8, receita.nota);
    sqlite3_bind_text(stmt, 9, receita.imagem.empty() ? nullptr : receita.imagem.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Erro ao inserir receita: " << sqlite3_errmsg(sqliteDb) << std::endl;
        sqlite3_finalize(stmt);
        return 0;
    }
    
    int receitaId = static_cast<int>(sqlite3_last_insert_rowid(sqliteDb));
    sqlite3_finalize(stmt);
    
    return receitaId;
}

std::vector<Receita> Database::listarReceitas() {
    std::vector<Receita> receitas;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem FROM receitas ORDER BY id";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return receitas;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Receita r;
        r.id = sqlite3_column_int(stmt, 0);
        r.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.tempo = sqlite3_column_int(stmt, 4);
        r.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.porcoes = sqlite3_column_int(stmt, 6);
        r.feita = (sqlite3_column_int(stmt, 7) == 1);
        r.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        r.imagem = (img ? std::string(img) : "");
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

Receita Database::consultarPorId(int id) {
    Receita receita;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem FROM receitas WHERE id = ?";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return receita;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        receita.id = sqlite3_column_int(stmt, 0);
        receita.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        receita.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        receita.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        receita.tempo = sqlite3_column_int(stmt, 4);
        receita.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        receita.porcoes = sqlite3_column_int(stmt, 6);
        receita.feita = (sqlite3_column_int(stmt, 7) == 1);
        receita.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        receita.imagem = (img ? std::string(img) : "");
        receita.tags = getTagsFromReceita(receita.id);
    }
    
    sqlite3_finalize(stmt);
    return receita;
}

std::vector<Receita> Database::buscarPorNome(const std::string& nome) {
    std::vector<Receita> receitas;
    sqlite3* sqlite3Db = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem FROM receitas WHERE nome LIKE ? ORDER BY id";
    
    if (sqlite3_prepare_v2(sqlite3Db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqlite3Db) << std::endl;
        return receitas;
    }
    
    std::string pattern = "%" + nome + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Receita r;
        r.id = sqlite3_column_int(stmt, 0);
        r.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.tempo = sqlite3_column_int(stmt, 4);
        r.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.porcoes = sqlite3_column_int(stmt, 6);
        r.feita = (sqlite3_column_int(stmt, 7) == 1);
        r.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        r.imagem = (img ? std::string(img) : "");
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

bool Database::excluirReceita(int id) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "DELETE FROM receitas WHERE id = ?";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

// ============================================================================
// GERENCIAMENTO DE TAGS
// ============================================================================
int Database::createTag(const std::string& nome) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sqlCheck = "SELECT id FROM tags WHERE nome = ?";
    if (sqlite3_prepare_v2(sqliteDb, sqlCheck, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int existingId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return existingId;
    }
    
    sqlite3_finalize(stmt);
    
    const char* sqlInsert = "INSERT INTO tags (nome) VALUES (?)";
    if (sqlite3_prepare_v2(sqliteDb, sqlInsert, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Erro ao inserir tag: " << sqlite3_errmsg(sqliteDb) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int tagId = static_cast<int>(sqlite3_last_insert_rowid(sqliteDb));
    sqlite3_finalize(stmt);
    
    return tagId;
}

std::vector<std::string> Database::getTagsFromReceita(int receitaId) {
    std::vector<std::string> tags;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT t.nome FROM tags t "
                      "INNER JOIN receitas_tags rt ON t.id = rt.tag_id "
                      "WHERE rt.receita_id = ? ORDER BY t.nome";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return tags;
    }
    
    sqlite3_bind_int(stmt, 1, receitaId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* tagNome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (tagNome) {
            tags.push_back(std::string(tagNome));
        }
    }
    
    sqlite3_finalize(stmt);
    return tags;
}

void Database::addTagToReceita(int receitaId, int tagId) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "INSERT OR IGNORE INTO receitas_tags (receita_id, tag_id) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return;
    }
    
    sqlite3_bind_int(stmt, 1, receitaId);
    sqlite3_bind_int(stmt, 2, tagId);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::removeTagFromReceita(int receitaId, int tagId) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "DELETE FROM receitas_tags WHERE receita_id = ? AND tag_id = ?";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return;
    }
    
    sqlite3_bind_int(stmt, 1, receitaId);
    sqlite3_bind_int(stmt, 2, tagId);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Receita> Database::getReceitasByTag(const std::string& nomeTag) {
    std::vector<Receita> receitas;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT DISTINCT r.id, r.nome, r.ingredientes, r.preparo, r.tempo, r.categoria, r.porcoes, r.feita, r.nota, r.imagem "
                      "FROM receitas r "
                      "INNER JOIN receitas_tags rt ON r.id = rt.receita_id "
                      "INNER JOIN tags t ON rt.tag_id = t.id "
                      "WHERE t.nome = ? ORDER BY r.id";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return receitas;
    }
    
    sqlite3_bind_text(stmt, 1, nomeTag.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Receita r;
        r.id = sqlite3_column_int(stmt, 0);
        r.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.tempo = sqlite3_column_int(stmt, 4);
        r.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.porcoes = sqlite3_column_int(stmt, 6);
        r.feita = (sqlite3_column_int(stmt, 7) == 1);
        r.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        r.imagem = (img ? std::string(img) : "");
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

std::vector<std::pair<int, std::string>> Database::listAllTags() {
    std::vector<std::pair<int, std::string>> tags;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome FROM tags ORDER BY nome";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return tags;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (nome) {
            tags.push_back(std::make_pair(id, std::string(nome)));
        }
    }
    
    sqlite3_finalize(stmt);
    return tags;
}

std::vector<std::string> Database::getTagsByPrefix(const std::string& prefixo) {
    std::vector<std::string> tags;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT nome FROM tags WHERE nome LIKE ? ORDER BY nome LIMIT 10";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return tags;
    }
    
    std::string pattern = prefixo + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (nome) {
            tags.push_back(std::string(nome));
        }
    }
    
    sqlite3_finalize(stmt);
    return tags;
}

// ============================================================================
// STATUS "FEITA" DAS RECEITAS
// ============================================================================
bool Database::marcarReceitaComoFeita(int id, bool feita) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "UPDATE receitas SET feita = ? WHERE id = ?";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, feita ? 1 : 0);
    sqlite3_bind_int(stmt, 2, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

std::vector<Receita> Database::getReceitasFeitas() {
    std::vector<Receita> receitas;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem FROM receitas WHERE feita = 1 ORDER BY id";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return receitas;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Receita r;
        r.id = sqlite3_column_int(stmt, 0);
        r.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.tempo = sqlite3_column_int(stmt, 4);
        r.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.porcoes = sqlite3_column_int(stmt, 6);
        r.feita = (sqlite3_column_int(stmt, 7) == 1);
        r.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        r.imagem = (img ? std::string(img) : "");
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

// ============================================================================
// AVALIAÇÃO DE RECEITAS
// ============================================================================
bool Database::avaliarReceita(int id, int nota) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    Receita receita = consultarPorId(id);
    if (receita.id == 0) {
        std::cerr << "Receita nao encontrada." << std::endl;
        return false;
    }
    
    if (!receita.feita) {
        std::cerr << "A receita precisa ser marcada como feita antes de ser avaliada." << std::endl;
        return false;
    }
    
    if (nota < 1 || nota > 5) {
        std::cerr << "Nota deve estar entre 1 e 5." << std::endl;
        return false;
    }
    
    const char* sql = "UPDATE receitas SET nota = ? WHERE id = ?";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, nota);
    sqlite3_bind_int(stmt, 2, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

std::vector<Receita> Database::getReceitasPorNota(int nota) {
    std::vector<Receita> receitas;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita, nota, imagem FROM receitas WHERE nota = ? AND feita = 1 ORDER BY id";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return receitas;
    }
    
    sqlite3_bind_int(stmt, 1, nota);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Receita r;
        r.id = sqlite3_column_int(stmt, 0);
        r.nome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.preparo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.tempo = sqlite3_column_int(stmt, 4);
        r.categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.porcoes = sqlite3_column_int(stmt, 6);
        r.feita = (sqlite3_column_int(stmt, 7) == 1);
        r.nota = sqlite3_column_int(stmt, 8);
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        r.imagem = (img ? std::string(img) : "");
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

// ============================================================================
// BACKUP E RESTAURAÇÃO
// ============================================================================
bool Database::fazerBackup(const std::string& caminhoBackup) {
    if (!db) {
        std::cerr << "Banco de dados nao esta aberto." << std::endl;
        return false;
    }
    
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3* backupDb = nullptr;
    
    std::filesystem::path backupPath(caminhoBackup);
    std::filesystem::path backupDir = backupPath.parent_path();
    if (!backupDir.empty() && !std::filesystem::exists(backupDir)) {
        std::filesystem::create_directories(backupDir);
    }
    
    sqlite3_exec(sqliteDb, "COMMIT", nullptr, nullptr, nullptr);
    sqlite3_exec(sqliteDb, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr);
    sqlite3_exec(sqliteDb, "COMMIT", nullptr, nullptr, nullptr);
    
    sqlite3_wal_checkpoint_v2(sqliteDb, nullptr, SQLITE_CHECKPOINT_FULL, nullptr, nullptr);
    
    sqlite3_stmt* checkStmt;
    const char* countSql = "SELECT COUNT(*) FROM receitas";
    int countAntes = 0;
    if (sqlite3_prepare_v2(sqliteDb, countSql, -1, &checkStmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(checkStmt) == SQLITE_ROW) {
            countAntes = sqlite3_column_int(checkStmt, 0);
        }
        sqlite3_finalize(checkStmt);
    }
    
    if (std::filesystem::exists(caminhoBackup)) {
        std::filesystem::remove(caminhoBackup);
    }
    
    if (sqlite3_open(caminhoBackup.c_str(), &backupDb) != SQLITE_OK) {
        std::cerr << "Erro ao criar arquivo de backup: " << sqlite3_errmsg(backupDb) << std::endl;
        if (backupDb) {
            sqlite3_close(backupDb);
        }
        return false;
    }
    
    sqlite3_exec(backupDb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    
    sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", sqliteDb, "main");
    if (!backup) {
        std::cerr << "Erro ao inicializar backup: " << sqlite3_errmsg(backupDb) << std::endl;
        sqlite3_close(backupDb);
        return false;
    }
    
    int result = sqlite3_backup_step(backup, -1);
    if (result != SQLITE_DONE) {
        std::cerr << "Erro durante backup: " << sqlite3_errmsg(backupDb) << std::endl;
        sqlite3_backup_finish(backup);
        sqlite3_close(backupDb);
        return false;
    }
    
    result = sqlite3_backup_finish(backup);
    if (result != SQLITE_OK) {
        std::cerr << "Erro ao finalizar backup: " << sqlite3_errmsg(backupDb) << std::endl;
        sqlite3_close(backupDb);
        return false;
    }
    
    sqlite3_exec(backupDb, "PRAGMA synchronous = FULL;", nullptr, nullptr, nullptr);
    
    sqlite3_close(backupDb);
    
    if (!std::filesystem::exists(caminhoBackup)) {
        std::cerr << "Arquivo de backup nao foi criado." << std::endl;
        return false;
    }
    
    auto tamanho = std::filesystem::file_size(caminhoBackup);
    if (tamanho == 0) {
        std::cerr << "Arquivo de backup esta vazio." << std::endl;
        return false;
    }
    
    sqlite3* verifyDb = nullptr;
    if (sqlite3_open_v2(caminhoBackup.c_str(), &verifyDb, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK) {
        sqlite3_stmt* verifyStmt;
        const char* verifyCountSql = "SELECT COUNT(*) FROM receitas";
        int countBackup = 0;
        if (sqlite3_prepare_v2(verifyDb, verifyCountSql, -1, &verifyStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(verifyStmt) == SQLITE_ROW) {
                countBackup = sqlite3_column_int(verifyStmt, 0);
            }
            sqlite3_finalize(verifyStmt);
        }
        sqlite3_close(verifyDb);
        
        if (countBackup != countAntes) {
            std::cerr << "Aviso: Numero de receitas no backup (" << countBackup 
                      << ") difere do banco original (" << countAntes << ")." << std::endl;
        }
    }
    
    return true;
}

bool Database::restaurarBackup(const std::string& caminhoBackup) {
    if (!std::filesystem::exists(caminhoBackup)) {
        std::cerr << "Arquivo de backup nao encontrado: " << caminhoBackup << std::endl;
        return false;
    }
    
    auto tamanho = std::filesystem::file_size(caminhoBackup);
    if (tamanho == 0) {
        std::cerr << "Arquivo de backup esta vazio." << std::endl;
        return false;
    }
    
    sqlite3* backupDb = nullptr;
    if (sqlite3_open_v2(caminhoBackup.c_str(), &backupDb, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao abrir arquivo de backup: " << sqlite3_errmsg(backupDb) << std::endl;
        if (backupDb) {
            sqlite3_close(backupDb);
        }
        return false;
    }
    
    sqlite3_stmt* stmt;
    const char* checkSql = "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('receitas', 'tags', 'receitas_tags')";
    if (sqlite3_prepare_v2(backupDb, checkSql, -1, &stmt, nullptr) == SQLITE_OK) {
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            count++;
        }
        sqlite3_finalize(stmt);
        if (count < 3) {
            std::cerr << "Arquivo de backup incompleto. Faltam tabelas essenciais." << std::endl;
            sqlite3_close(backupDb);
            return false;
        }
    }
    sqlite3_close(backupDb);
    
    if (db) {
        sqlite3_close((sqlite3*)db);
        db = nullptr;
    }
    
    std::string backupSeguranca = dbPath + ".pre_restore";
    if (std::filesystem::exists(dbPath)) {
        try {
            std::filesystem::copy_file(dbPath, backupSeguranca, std::filesystem::copy_options::overwrite_existing);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Aviso: Nao foi possivel criar backup de seguranca: " << e.what() << std::endl;
        }
    }
    
    try {
        std::filesystem::copy_file(caminhoBackup, dbPath, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Erro ao copiar arquivo de backup: " << e.what() << std::endl;
        if (std::filesystem::exists(backupSeguranca)) {
            std::filesystem::copy_file(backupSeguranca, dbPath, std::filesystem::copy_options::overwrite_existing);
        }
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    if (sqlite3_open_v2(dbPath.c_str(), (sqlite3**)&db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao reabrir banco de dados: " << sqlite3_errmsg((sqlite3*)db) << std::endl;
        return false;
    }
    
    if (!executeQuery("PRAGMA foreign_keys = ON;")) {
        std::cerr << "Erro ao habilitar foreign keys" << std::endl;
        return false;
    }
    
    executeQuery("PRAGMA journal_mode = DELETE;");
    executeQuery("PRAGMA synchronous = FULL;");
    
    sqlite3* sqliteDb = (sqlite3*)db;
    
    const char* verifySql = "SELECT COUNT(*) FROM receitas";
    int countReceitas = 0;
    if (sqlite3_prepare_v2(sqliteDb, verifySql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            countReceitas = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    const char* countTagsSql = "SELECT COUNT(*) FROM tags";
    int countTags = 0;
    if (sqlite3_prepare_v2(sqliteDb, countTagsSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            countTags = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    const char* countRelSql = "SELECT COUNT(*) FROM receitas_tags";
    int countRel = 0;
    if (sqlite3_prepare_v2(sqliteDb, countRelSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            countRel = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_exec(sqliteDb, "PRAGMA synchronous = FULL;", nullptr, nullptr, nullptr);
    sqlite3_exec(sqliteDb, "PRAGMA journal_mode = DELETE;", nullptr, nullptr, nullptr);
    
    std::cout << "Restaurado: " << countReceitas << " receitas, " 
              << countTags << " tags, " << countRel << " relacionamentos." << std::endl;
    
    return true;
}

// ============================================================================
// FECHAMENTO E LIMPEZA
// ============================================================================
void Database::close() {
    if (db) {
        sqlite3_close((sqlite3*)db);
        db = nullptr;
    }
}
