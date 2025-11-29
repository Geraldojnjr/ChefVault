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
    
    return createTable();
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
            porcoes INTEGER
        )
    )";
    
    return executeQuery(query);
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

bool Database::cadastrarReceita(const Receita& receita) {
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "INSERT INTO receitas (nome, ingredientes, preparo, tempo, categoria, porcoes) VALUES (?, ?, ?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(sqliteDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao preparar statement: " << sqlite3_errmsg(sqliteDb) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, receita.nome.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receita.ingredientes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, receita.preparo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, receita.tempo);
    sqlite3_bind_text(stmt, 5, receita.categoria.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, receita.porcoes);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

std::vector<Receita> Database::listarReceitas() {
    std::vector<Receita> receitas;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes FROM receitas ORDER BY id";
    
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
        receitas.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return receitas;
}

Receita Database::consultarPorId(int id) {
    Receita receita;
    sqlite3* sqliteDb = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes FROM receitas WHERE id = ?";
    
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
    }
    
    sqlite3_finalize(stmt);
    return receita;
}

std::vector<Receita> Database::buscarPorNome(const std::string& nome) {
    std::vector<Receita> receitas;
    sqlite3* sqlite3Db = (sqlite3*)db;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT id, nome, ingredientes, preparo, tempo, categoria, porcoes FROM receitas WHERE nome LIKE ? ORDER BY id";
    
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

void Database::close() {
    if (db) {
        sqlite3_close((sqlite3*)db);
        db = nullptr;
    }
}

