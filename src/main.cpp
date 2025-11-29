#include "../include/Database.h"
#include "../include/Receita.h"
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <sstream>
#include <algorithm>

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
    std::cout << "6. Adicionar tag em uma receita\n";
    std::cout << "7. Remover tag de uma receita\n";
    std::cout << "8. Listar tags disponiveis\n";
    std::cout << "9. Filtrar receitas por tag\n";
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
    
    int receitaId = db.cadastrarReceita(receita);
    
    if (receitaId > 0) {
        std::cout << "\nReceita cadastrada com sucesso! (ID: " << receitaId << ")\n";
        
        std::cout << "Deseja adicionar tags? (s/n): ";
        char resposta;
        std::cin >> resposta;
        limparBuffer();
        
        if (resposta == 's' || resposta == 'S') {
            std::cout << "Digite as tags separadas por virgula: ";
            std::string tagsInput;
            std::getline(std::cin, tagsInput);
            
            if (!tagsInput.empty()) {
                std::istringstream iss(tagsInput);
                std::string tag;
                int tagsAdicionadas = 0;
                
                while (std::getline(iss, tag, ',')) {
                    // Remover espaços em branco
                    tag.erase(0, tag.find_first_not_of(" \t"));
                    tag.erase(tag.find_last_not_of(" \t") + 1);
                    
                    if (!tag.empty()) {
                        int tagId = db.createTag(tag);
                        if (tagId > 0) {
                            db.addTagToReceita(receitaId, tagId);
                            tagsAdicionadas++;
                        }
                    }
                }
                if (tagsAdicionadas > 0) {
                    std::cout << tagsAdicionadas << " tag(s) adicionada(s) com sucesso!\n";
                }
            }
        }
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
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (const auto& r : receitas) {
        std::string tagsStr = "";
        if (!r.tags.empty()) {
            for (size_t i = 0; i < r.tags.size(); ++i) {
                tagsStr += r.tags[i];
                if (i < r.tags.size() - 1) {
                    tagsStr += ", ";
                }
            }
        } else {
            tagsStr = "-";
        }
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
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
    
    if (!receita.tags.empty()) {
        std::cout << "Tags: ";
        for (size_t i = 0; i < receita.tags.size(); ++i) {
            std::cout << receita.tags[i];
            if (i < receita.tags.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
    }
    
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
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (const auto& r : receitas) {
        std::string tagsStr = "";
        if (!r.tags.empty()) {
            for (size_t i = 0; i < r.tags.size(); ++i) {
                tagsStr += r.tags[i];
                if (i < r.tags.size() - 1) {
                    tagsStr += ", ";
                }
            }
        } else {
            tagsStr = "-";
        }
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
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

void adicionarTagReceita(Database& db) {
    std::cout << "\n--- Adicionar Tag em Receita ---\n";
    std::cout << "Digite o ID da receita: ";
    
    int receitaId;
    std::cin >> receitaId;
    
    Receita receita = db.consultarPorId(receitaId);
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    std::cout << "Receita: " << receita.nome << "\n";
    std::cout << "Digite as tags separadas por virgula: ";
    
    limparBuffer();
    std::string tagsInput;
    std::getline(std::cin, tagsInput);
    
    if (tagsInput.empty()) {
        std::cout << "Nenhuma tag informada.\n";
        return;
    }
    
    std::istringstream iss(tagsInput);
    std::string tag;
    int tagsAdicionadas = 0;
    
    while (std::getline(iss, tag, ',')) {
        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        
        if (!tag.empty()) {
            int tagId = db.createTag(tag);
            if (tagId > 0) {
                db.addTagToReceita(receitaId, tagId);
                tagsAdicionadas++;
            }
        }
    }
    
    if (tagsAdicionadas > 0) {
        std::cout << tagsAdicionadas << " tag(s) adicionada(s) com sucesso!\n";
    } else {
        std::cout << "Erro ao adicionar tags.\n";
    }
}

void removerTagReceita(Database& db) {
    std::cout << "\n--- Remover Tag de Receita ---\n";
    std::cout << "Digite o ID da receita: ";
    
    int receitaId;
    std::cin >> receitaId;
    
    Receita receita = db.consultarPorId(receitaId);
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    if (receita.tags.empty()) {
        std::cout << "Esta receita nao possui tags.\n";
        return;
    }
    
    std::cout << "Receita: " << receita.nome << "\n";
    std::cout << "Tags atuais: ";
    for (size_t i = 0; i < receita.tags.size(); ++i) {
        std::cout << receita.tags[i];
        if (i < receita.tags.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "\n";
    
    std::cout << "Digite o nome da tag a ser removida: ";
    limparBuffer();
    std::string tagNome;
    std::getline(std::cin, tagNome);
    
    // Buscar o ID da tag
    auto todasTags = db.listAllTags();
    int tagId = -1;
    for (const auto& tag : todasTags) {
        if (tag.second == tagNome) {
            tagId = tag.first;
            break;
        }
    }
    
    if (tagId == -1) {
        std::cout << "Tag nao encontrada.\n";
        return;
    }
    
    db.removeTagFromReceita(receitaId, tagId);
    std::cout << "Tag removida com sucesso!\n";
}

void listarTags(Database& db) {
    std::cout << "\n--- Lista de Tags Disponiveis ---\n";
    
    auto tags = db.listAllTags();
    
    if (tags.empty()) {
        std::cout << "Nenhuma tag cadastrada.\n";
        return;
    }
    
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << "\n";
    std::cout << std::string(35, '-') << "\n";
    
    for (const auto& tag : tags) {
        std::cout << std::left << std::setw(5) << tag.first 
                  << std::setw(30) << tag.second 
                  << "\n";
    }
}

void filtrarReceitasPorTag(Database& db) {
    std::cout << "\n--- Filtrar Receitas por Tag ---\n";
    std::cout << "Digite o nome da tag: ";
    
    limparBuffer();
    std::string tagNome;
    std::getline(std::cin, tagNome);
    
    if (tagNome.empty()) {
        std::cout << "Nome da tag nao pode ser vazio.\n";
        return;
    }
    
    auto receitas = db.getReceitasByTag(tagNome);
    
    if (receitas.empty()) {
        std::cout << "Nenhuma receita encontrada com a tag \"" << tagNome << "\".\n";
        return;
    }
    
    std::cout << "\nReceitas com a tag \"" << tagNome << "\":\n";
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << std::setw(15) << "Categoria" 
              << std::setw(10) << "Tempo" 
              << std::setw(10) << "Porcoes" 
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (const auto& r : receitas) {
        std::string tagsStr = "";
        if (!r.tags.empty()) {
            for (size_t i = 0; i < r.tags.size(); ++i) {
                tagsStr += r.tags[i];
                if (i < r.tags.size() - 1) {
                    tagsStr += ", ";
                }
            }
        } else {
            tagsStr = "-";
        }
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
                  << "\n";
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
            case 6:
                adicionarTagReceita(db);
                break;
            case 7:
                removerTagReceita(db);
                break;
            case 8:
                listarTags(db);
                break;
            case 9:
                filtrarReceitasPorTag(db);
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

