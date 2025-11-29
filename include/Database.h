#ifndef DATABASE_H
#define DATABASE_H

#include "Receita.h"
#include <vector>
#include <string>

class Database {
private:
    std::string dbPath;
    void* db; // SQLite database handle

    bool executeQuery(const std::string& query);
    bool createTable();

public:
    Database(const std::string& path);
    ~Database();

    bool initialize();
    bool cadastrarReceita(const Receita& receita);
    std::vector<Receita> listarReceitas();
    Receita consultarPorId(int id);
    std::vector<Receita> buscarPorNome(const std::string& nome);
    bool excluirReceita(int id);
    void close();
};

#endif // DATABASE_H

