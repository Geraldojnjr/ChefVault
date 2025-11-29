#include "../include/Database.h"
#include "../include/Receita.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <vector>
#include <string>

// Contador de testes
int tests_passed = 0;
int tests_failed = 0;

void test_result(const std::string& test_name, bool passed) {
    if (passed) {
        std::cout << "[PASS] " << test_name << std::endl;
        tests_passed++;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        tests_failed++;
    }
}

// Testes das funcionalidades
void test_criar_receita(Database& db) {
    Receita receita("Teste Receita", "Ingrediente 1, Ingrediente 2", 
                    "Modo de preparo teste", 30, "Teste", 4);
    receita.feita = false;
    receita.nota = 0;
    
    int id = db.cadastrarReceita(receita);
    test_result("Criar receita", id > 0);
}

void test_listar_receitas(Database& db) {
    auto receitas = db.listarReceitas();
    test_result("Listar receitas", !receitas.empty());
}

void test_consultar_por_id(Database& db) {
    Receita receita = db.consultarPorId(1);
    test_result("Consultar receita por ID", receita.id > 0 && receita.nome == "Teste Receita");
}

void test_buscar_por_nome(Database& db) {
    auto receitas = db.buscarPorNome("Teste");
    test_result("Buscar receita por nome", !receitas.empty());
}

void test_excluir_receita(Database& db) {
    // Criar uma receita para excluir
    Receita receita("Receita para excluir", "Ingredientes", "Preparo", 20, "Teste", 2);
    receita.feita = false;
    receita.nota = 0;
    int id = db.cadastrarReceita(receita);
    
    bool excluida = db.excluirReceita(id);
    Receita verificacao = db.consultarPorId(id);
    test_result("Excluir receita", excluida && verificacao.id == 0);
}

// Testes - Tags
void test_criar_tag(Database& db) {
    int tagId = db.createTag("teste-tag");
    test_result("Criar tag", tagId > 0);
}

void test_listar_tags(Database& db) {
    auto tags = db.listAllTags();
    test_result("Listar tags", !tags.empty());
}

void test_adicionar_tag_receita(Database& db) {
    // Criar receita
    Receita receita("Receita com tag", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = false;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Criar tag
    int tagId = db.createTag("tag-teste");
    
    // Adicionar tag
    db.addTagToReceita(receitaId, tagId);
    
    // Verificar
    auto tags = db.getTagsFromReceita(receitaId);
    bool temTag = false;
    for (const auto& tag : tags) {
        if (tag == "tag-teste") {
            temTag = true;
            break;
        }
    }
    test_result("Adicionar tag a receita", temTag);
}

void test_remover_tag_receita(Database& db) {
    // Criar receita
    Receita receita("Receita para remover tag", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = false;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Criar e adicionar tag
    int tagId = db.createTag("tag-remover");
    db.addTagToReceita(receitaId, tagId);
    
    // Remover tag
    db.removeTagFromReceita(receitaId, tagId);
    
    // Verificar
    auto tags = db.getTagsFromReceita(receitaId);
    bool temTag = false;
    for (const auto& tag : tags) {
        if (tag == "tag-remover") {
            temTag = true;
            break;
        }
    }
    test_result("Remover tag de receita", !temTag);
}

void test_filtrar_por_tag(Database& db) {
    // Criar receita
    Receita receita("Receita filtro tag", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = false;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Criar e adicionar tag
    int tagId = db.createTag("filtro-tag");
    db.addTagToReceita(receitaId, tagId);
    
    // Filtrar
    auto receitas = db.getReceitasByTag("filtro-tag");
    test_result("Filtrar receitas por tag", !receitas.empty());
}

// Testes de Feita
void test_marcar_como_feita(Database& db) {
    // Criar receita
    Receita receita("Receita para marcar feita", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = false;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Marcar como feita
    bool sucesso = db.marcarReceitaComoFeita(receitaId, true);
    Receita verificacao = db.consultarPorId(receitaId);
    test_result("Marcar receita como feita", sucesso && verificacao.feita);
}

void test_listar_receitas_feitas(Database& db) {
    auto receitas = db.getReceitasFeitas();
    test_result("Listar receitas feitas", !receitas.empty());
}

// Testes de Avaliação
void test_avaliar_receita(Database& db) {
    // Criar receita e marcar como feita
    Receita receita("Receita para avaliar", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = true;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Avaliar
    bool sucesso = db.avaliarReceita(receitaId, 5);
    Receita verificacao = db.consultarPorId(receitaId);
    test_result("Avaliar receita", sucesso && verificacao.nota == 5);
}

void test_avaliar_receita_nao_feita(Database& db) {
    // Criar receita nao feita
    Receita receita("Receita não feita", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = false;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Tentar avaliar **nao pode**
    bool sucesso = db.avaliarReceita(receitaId, 5);
    test_result("Não permitir avaliar receita não feita", !sucesso);
}

void test_filtrar_por_nota(Database& db) {
    // Criar receita, marcar como feita e avaliar
    Receita receita("Receita nota 4", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = true;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    db.avaliarReceita(receitaId, 4);
    
    // Filtrar
    auto receitas = db.getReceitasPorNota(4);
    test_result("Filtrar receitas por nota", !receitas.empty());
}

void test_validacao_nota_invalida(Database& db) {
    // Criar receita e marcar como feita
    Receita receita("Receita validacao", "Ingredientes", "Preparo", 25, "Teste", 3);
    receita.feita = true;
    receita.nota = 0;
    int receitaId = db.cadastrarReceita(receita);
    
    // Tentar avaliar com nota errada + **nao pode**
    bool sucesso = db.avaliarReceita(receitaId, 6);
    test_result("Validar nota inválida (6)", !sucesso);
    
    // Tentar avaliar com nota errada - **nao pode**
    sucesso = db.avaliarReceita(receitaId, 0);
    test_result("Validar nota inválida (0)", !sucesso);
}

int main() {
    std::cout << "=== Testes ChefVault ===" << std::endl;
    std::cout << std::endl;
    
    // Usar banco de dados temporário para testes
    std::string testDbPath = "./test_recipes.db";
    
    // Remover banco de teste se existir
    if (std::filesystem::exists(testDbPath)) {
        std::filesystem::remove(testDbPath);
    }
    
    Database db(testDbPath);
    
    if (!db.initialize()) {
        std::cerr << "Erro ao inicializar banco de dados de teste!" << std::endl;
        return 1;
    }
    
    std::cout << "--- Testes Funcionalidades(CRUD) ---" << std::endl;
    test_criar_receita(db);
    test_listar_receitas(db);
    test_consultar_por_id(db);
    test_buscar_por_nome(db);
    test_excluir_receita(db);
    
    std::cout << std::endl;
    std::cout << "--- Testes Funcionalidade Tags ---" << std::endl;
    test_criar_tag(db);
    test_listar_tags(db);
    test_adicionar_tag_receita(db);
    test_remover_tag_receita(db);
    test_filtrar_por_tag(db);
    
    std::cout << std::endl;
    std::cout << "--- Testes Funcionalidade Feita ---" << std::endl;
    test_marcar_como_feita(db);
    test_listar_receitas_feitas(db);
    
    std::cout << std::endl;
    std::cout << "--- Testes Funcionalidade Avaliação ---" << std::endl;
    test_avaliar_receita(db);
    test_avaliar_receita_nao_feita(db);
    test_filtrar_por_nota(db);
    test_validacao_nota_invalida(db);
    
    std::cout << std::endl;
    std::cout << "=== Resultados ===" << std::endl;
    std::cout << "Testes passados: " << tests_passed << std::endl;
    std::cout << "Testes falhados: " << tests_failed << std::endl;
    std::cout << "Total: " << (tests_passed + tests_failed) << std::endl;
    
    // Limpar banco de teste
    db.close();
    if (std::filesystem::exists(testDbPath)) {
        std::filesystem::remove(testDbPath);
    }
    
    if (tests_failed == 0) {
        std::cout << std::endl << "Todos os testes passaram!" << std::endl;
        return 0;
    } else {
        std::cout << std::endl << "Alguns testes falharam!" << std::endl;
        return 1;
    }
}

