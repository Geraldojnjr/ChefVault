#include "../include/Database.h"
#include "../include/Receita.h"
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>

void limparBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void exibirMenu() {
    std::cout << "\n=== COOKBOOK CLI ===\n";
    std::cout << "1. Cadastrar receita\n";
    std::cout << "2. Listar receitas\n";
    std::cout << "3. Consultar detalhes por ID\n";
    std::cout << "4. Buscar por nome ou parte do nome\n";
    std::cout << "5. Excluir receita\n";
    std::cout << "0. Sair\n";
    std::cout << "Escolha uma opcao: ";
}

void cadastrarReceita(Database& db) {
    std::cout << "\n--- Cadastrar Receita ---\n";
    
    Receita receita;
    
    std::cout << "Nome: ";
    limparBuffer();
    std::getline(std::cin, receita.nome);
    
    std::cout << "Ingredientes: ";
    std::getline(std::cin, receita.ingredientes);
    
    std::cout << "Modo de preparo: ";
    std::getline(std::cin, receita.preparo);
    
    std::cout << "Tempo de preparo (minutos): ";
    std::cin >> receita.tempo;
    
    std::cout << "Categoria: ";
    limparBuffer();
    std::getline(std::cin, receita.categoria);
    
    std::cout << "Porcoes: ";
    std::cin >> receita.porcoes;
    
    if (db.cadastrarReceita(receita)) {
        std::cout << "\nReceita cadastrada com sucesso!\n";
    } else {
        std::cout << "\nErro ao cadastrar receita.\n";
    }
}

void listarReceitas(Database& db) {
    std::cout << "\n--- Lista de Receitas ---\n";
    
    auto receitas = db.listarReceitas();
    
    if (receitas.empty()) {
        std::cout << "Nenhuma receita cadastrada.\n";
        return;
    }
    
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << std::setw(15) << "Categoria" 
              << std::setw(10) << "Tempo" 
              << std::setw(10) << "Porcoes" 
              << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& r : receitas) {
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << "\n";
    }
}

void consultarPorId(Database& db) {
    std::cout << "\n--- Consultar Receita por ID ---\n";
    std::cout << "Digite o ID da receita: ";
    
    int id;
    std::cin >> id;
    
    Receita receita = db.consultarPorId(id);
    
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    std::cout << "\n=== " << receita.nome << " ===\n";
    std::cout << "ID: " << receita.id << "\n";
    std::cout << "Categoria: " << receita.categoria << "\n";
    std::cout << "Tempo: " << receita.tempo << " minutos\n";
    std::cout << "Porcoes: " << receita.porcoes << "\n";
    std::cout << "\nIngredientes:\n" << receita.ingredientes << "\n";
    std::cout << "\nModo de Preparo:\n" << receita.preparo << "\n";
}

void buscarPorNome(Database& db) {
    std::cout << "\n--- Buscar Receita por Nome ---\n";
    std::cout << "Digite o nome ou parte do nome: ";
    
    limparBuffer();
    std::string nome;
    std::getline(std::cin, nome);
    
    auto receitas = db.buscarPorNome(nome);
    
    if (receitas.empty()) {
        std::cout << "Nenhuma receita encontrada.\n";
        return;
    }
    
    std::cout << "\nReceitas encontradas:\n";
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << std::setw(15) << "Categoria" 
              << std::setw(10) << "Tempo" 
              << std::setw(10) << "Porcoes" 
              << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& r : receitas) {
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << "\n";
    }
}

void excluirReceita(Database& db) {
    std::cout << "\n--- Excluir Receita ---\n";
    std::cout << "Digite o ID da receita a ser excluida: ";
    
    int id;
    std::cin >> id;
    
    // Verificar se a receita existe
    Receita receita = db.consultarPorId(id);
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    std::cout << "Tem certeza que deseja excluir a receita \"" << receita.nome << "\"? (s/n): ";
    char confirmacao;
    std::cin >> confirmacao;
    
    if (confirmacao == 's' || confirmacao == 'S') {
        if (db.excluirReceita(id)) {
            std::cout << "Receita excluida com sucesso!\n";
        } else {
            std::cout << "Erro ao excluir receita.\n";
        }
    } else {
        std::cout << "Operacao cancelada.\n";
    }
}

int main() {
    Database db("./data/recipes.db");
    
    if (!db.initialize()) {
        std::cerr << "Erro ao inicializar banco de dados.\n";
        return 1;
    }
    
    int opcao;
    
    do {
        exibirMenu();
        std::cin >> opcao;
        
        switch (opcao) {
            case 1:
                cadastrarReceita(db);
                break;
            case 2:
                listarReceitas(db);
                break;
            case 3:
                consultarPorId(db);
                break;
            case 4:
                buscarPorNome(db);
                break;
            case 5:
                excluirReceita(db);
                break;
            case 0:
                std::cout << "\nSaindo...\n";
                break;
            default:
                std::cout << "\nOpcao invalida!\n";
                break;
        }
    } while (opcao != 0);
    
    db.close();
    return 0;
}

