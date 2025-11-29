#ifndef DATABASE_H
#define DATABASE_H

#include "Receita.h"
#include <vector>
#include <string>
#include <utility>

class Database {
private:
    std::string dbPath;
    void* db; // SQLite database handle

    bool executeQuery(const std::string& query);
    bool executeQuerySilent(const std::string& query);
    bool columnExists(const std::string& tableName, const std::string& columnName);
    bool createTable();
    bool createTagsTables();

public:
    Database(const std::string& path);
    ~Database();

    bool initialize();
    int cadastrarReceita(const Receita& receita);
    std::vector<Receita> listarReceitas();
    Receita consultarPorId(int id);
    std::vector<Receita> buscarPorNome(const std::string& nome);
    bool excluirReceita(int id);
    bool marcarReceitaComoFeita(int id, bool feita);
    std::vector<Receita> getReceitasFeitas();
    bool avaliarReceita(int id, int nota);
    std::vector<Receita> getReceitasPorNota(int nota);
    bool fazerBackup(const std::string& caminhoBackup);
    bool restaurarBackup(const std::string& caminhoBackup);
    void close();
    
    // Métodos de tags
    int createTag(const std::string& nome);
    std::vector<std::string> getTagsFromReceita(int receitaId);
    void addTagToReceita(int receitaId, int tagId);
    void removeTagFromReceita(int receitaId, int tagId);
    std::vector<Receita> getReceitasByTag(const std::string& nomeTag);
    std::vector<std::pair<int, std::string>> listAllTags();
    std::vector<std::string> getTagsByPrefix(const std::string& prefixo);
};

#endif // DATABASE_H

