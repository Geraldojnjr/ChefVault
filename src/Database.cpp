#include "../include/Database.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <filesystem>

Database::Database(const std::string& path) : dbPath(path), db(nullptr) {
    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

Database::~Database() {
    close();
}

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
            feita INTEGER DEFAULT 0
        )
    )";
    
    if (!executeQuery(query)) {
        return false;
    }
    
    // Adicionar coluna feita se a tabela já existir sem ela (migration)
    std::string alterQuery = R"(
        ALTER TABLE receitas ADD COLUMN feita INTEGER DEFAULT 0
    )";
    
    // Ignorar erro se a coluna já existir
    executeQuery(alterQuery);
    
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

int Database::cadastrarReceita(const Receita& receita) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "INSERT INTO receitas (nome, ingredientes, preparo, tempo, categoria, porcoes, feita) VALUES (?, ?, ?, ?, ?, ?, ?)";
    
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
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita FROM receitas ORDER BY id";
    
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
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita FROM receitas WHERE id = ?";
    
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
        receita.tags = getTagsFromReceita(receita.id);
    }
    
    sqlite3_finalize(stmt);
    return receita;
}

std::vector<Receita> Database::buscarPorNome(const std::string& nome) {
    std::vector<Receita> receitas;
    sqlite3* sqlite3Db = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita FROM receitas WHERE nome LIKE ? ORDER BY id";
    
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
    
    const char* sql = "SELECT DISTINCT r.id, r.nome, r.ingredientes, r.preparo, r.tempo, r.categoria, r.porcoes, r.feita "
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
        r.tags = getTagsFromReceita(r.id);
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

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
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes, feita FROM receitas WHERE feita = 1 ORDER BY id";
    
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

void Database::close() {
    if (db) {
        sqlite3_close((sqlite3*)db);
        db = nullptr;
    }
}

