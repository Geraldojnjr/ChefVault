// ============================================================================
// INCLUDES
// ============================================================================
#include "../include/Database.h"
#include "../include/Receita.h"
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <cctype>

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================
void limparBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string solicitarTagsComAutocompletarSimples(Database& db) {
    std::cout << "Digite as tags separadas por virgula.\n";
    std::cout << "Dica: Digite ? para ver todas as tags disponiveis.\n";
    std::cout << "Tags: ";
    
    std::string input;
    std::getline(std::cin, input);
    
    // Se o usuário digitou apenas ?, mostrar todas as tags
    if (input == "?") {
        auto todasTags = db.listAllTags();
        if (todasTags.empty()) {
            std::cout << "Nenhuma tag cadastrada.\n";
        } else {
            std::cout << "\n--- Tags Disponiveis ---\n";
            for (size_t i = 0; i < todasTags.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << todasTags[i].second;
                if ((i + 1) % 5 == 0 || i == todasTags.size() - 1) {
                    std::cout << "\n";
                } else {
                    std::cout << "  |  ";
                }
            }
        }
        std::cout << "\nAgora digite as tags separadas por virgula: ";
        std::getline(std::cin, input);
    }
    
    // Processar input e mostrar sugestões para cada tag parcial
    if (!input.empty()) {
        std::istringstream iss(input);
        std::string tag;
        std::vector<std::string> tagsProcessadas;
        bool mostrarSugestoes = false;
        
        while (std::getline(iss, tag, ',')) {
            tag.erase(0, tag.find_first_not_of(" \t"));
            tag.erase(tag.find_last_not_of(" \t") + 1);
            
            if (!tag.empty()) {
                // Verificar se a tag existe ou buscar sugestões
                auto sugestoes = db.getTagsByPrefix(tag);
                
                // Se não encontrou exatamente, mostrar sugestões
                bool encontrouExato = false;
                for (const auto& sug : sugestoes) {
                    if (sug == tag) {
                        encontrouExato = true;
                        break;
                    }
                }
                
                if (!encontrouExato && !sugestoes.empty() && tag.length() >= 2) {
                    std::cout << "\nSugestoes para \"" << tag << "\": ";
                    for (size_t i = 0; i < sugestoes.size() && i < 5; ++i) {
                        std::cout << sugestoes[i];
                        if (i < sugestoes.size() - 1 && i < 4) {
                            std::cout << ", ";
                        }
                    }
                    if (sugestoes.size() > 5) {
                        std::cout << " ...";
                    }
                    std::cout << "\n";
                    mostrarSugestoes = true;
                }
                
                tagsProcessadas.push_back(tag);
            }
        }
        
        // Reconstruir string com tags processadas
        std::string resultado;
        for (size_t i = 0; i < tagsProcessadas.size(); ++i) {
            resultado += tagsProcessadas[i];
            if (i < tagsProcessadas.size() - 1) {
                resultado += ", ";
            }
        }
        
        return resultado;
    }
    
    return input;
}

// ============================================================================
// MENU PRINCIPAL
// ============================================================================
void exibirMenu() {
    std::cout << "\n=== COOKBOOK CLI ===\n";
    std::cout << "1.  Cadastrar receita\n";
    std::cout << "2.  Listar receitas\n";
    std::cout << "3.  Consultar detalhes por ID\n";
    std::cout << "4.  Buscar por nome ou parte do nome\n";
    std::cout << "5.  Excluir receita\n";
    std::cout << "6.  Adicionar tag em uma receita\n";
    std::cout << "7.  Remover tag de uma receita\n";
    std::cout << "8.  Listar tags disponiveis\n";
    std::cout << "9.  Filtrar receitas por tag\n";
    std::cout << "10. Marcar receita como feita/nao feita\n";
    std::cout << "11. Listar receitas feitas\n";
    std::cout << "12. Avaliar receita (1-5)\n";
    std::cout << "13. Filtrar receitas por nota\n";
    std::cout << "98. Fazer backup do banco de dados\n";
    std::cout << "99. Restaurar backup do banco de dados\n";
    std::cout << "0.  Sair\n";
    std::cout << "Escolha uma opcao: ";
}

// ============================================================================
// CRUD DE RECEITAS
// ============================================================================
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
    
    std::cout << "Ja foi feita? (s/n): ";
    char feita;
    std::cin >> feita;
    receita.feita = (feita == 's' || feita == 'S');
    receita.nota = 0;
    
    std::cout << "Caminho da imagem (ou Enter para pular): ";
    limparBuffer();
    std::getline(std::cin, receita.imagem);
    
    int receitaId = db.cadastrarReceita(receita);
    
    if (receitaId > 0) {
        std::cout << "\nReceita cadastrada com sucesso! (ID: " << receitaId << ")\n";
        
        std::cout << "Deseja adicionar tags? (s/n): ";
        char resposta;
        std::cin >> resposta;
        limparBuffer();
        
        if (resposta == 's' || resposta == 'S') {
            std::string tagsInput = solicitarTagsComAutocompletarSimples(db);
            
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
              << std::setw(8) << "Feita"
              << std::setw(8) << "Nota"
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(116, '-') << "\n";
    
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
        
        std::string notaStr = (r.nota > 0) ? std::to_string(r.nota) : "-";
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(8) << (r.feita ? "Sim" : "Nao")
                  << std::setw(8) << notaStr
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
    std::cout << "Feita: " << (receita.feita ? "Sim" : "Nao") << "\n";
    if (receita.feita && receita.nota > 0) {
        std::cout << "Nota: " << receita.nota << "/5\n";
    } else if (receita.feita) {
        std::cout << "Nota: Nao avaliada\n";
    }
    
    if (!receita.imagem.empty()) {
        std::cout << "Imagem: " << receita.imagem << "\n";
    }
    
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
              << std::setw(8) << "Feita"
              << std::setw(8) << "Nota"
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(116, '-') << "\n";
    
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
        
        std::string notaStr = (r.nota > 0) ? std::to_string(r.nota) : "-";
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(8) << (r.feita ? "Sim" : "Nao")
                  << std::setw(8) << notaStr
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

// ============================================================================
// GERENCIAMENTO DE TAGS
// ============================================================================
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
    limparBuffer();
    std::string tagsInput = solicitarTagsComAutocompletarSimples(db);
    
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
    std::cout << "Digite o nome da tag (ou ? para ver todas as tags): ";
    
    limparBuffer();
    std::string tagNome;
    std::getline(std::cin, tagNome);
    
    // Se o usuário digitou ?, mostrar todas as tags
    if (tagNome == "?") {
        auto todasTags = db.listAllTags();
        if (todasTags.empty()) {
            std::cout << "Nenhuma tag cadastrada.\n";
            return;
        }
        std::cout << "\n--- Tags Disponiveis ---\n";
        for (size_t i = 0; i < todasTags.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << todasTags[i].second;
            if ((i + 1) % 5 == 0 || i == todasTags.size() - 1) {
                std::cout << "\n";
            } else {
                std::cout << "  |  ";
            }
        }
        std::cout << "\nDigite o nome da tag: ";
        std::getline(std::cin, tagNome);
    }
    
    if (tagNome.empty()) {
        std::cout << "Nome da tag nao pode ser vazio.\n";
        return;
    }
    
    // Se não encontrou exato, buscar por prefixo e mostrar sugestões
    auto todasTags = db.listAllTags();
    bool encontrouExato = false;
    for (const auto& tag : todasTags) {
        if (tag.second == tagNome) {
            encontrouExato = true;
            break;
        }
    }
    
    if (!encontrouExato) {
        auto sugestoes = db.getTagsByPrefix(tagNome);
        if (!sugestoes.empty()) {
            std::cout << "\nTag \"" << tagNome << "\" nao encontrada. Sugestoes:\n";
            for (size_t i = 0; i < sugestoes.size() && i < 10; ++i) {
                std::cout << "  " << (i + 1) << ". " << sugestoes[i] << "\n";
            }
            if (sugestoes.size() == 1) {
                std::cout << "\nUsando tag: " << sugestoes[0] << "\n";
                tagNome = sugestoes[0];
            } else if (sugestoes.size() > 1) {
                std::cout << "\nDigite o nome completo da tag (ou Enter para cancelar): ";
                std::string novaTag;
                std::getline(std::cin, novaTag);
                if (!novaTag.empty()) {
                    tagNome = novaTag;
                } else {
                    return;
                }
            } else {
                std::cout << "Nenhuma tag encontrada.\n";
                return;
            }
        } else {
            std::cout << "Tag \"" << tagNome << "\" nao encontrada.\n";
            return;
        }
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
              << std::setw(8) << "Feita"
              << std::setw(8) << "Nota"
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(116, '-') << "\n";
    
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
        
        std::string notaStr = (r.nota > 0) ? std::to_string(r.nota) : "-";
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(8) << (r.feita ? "Sim" : "Nao")
                  << std::setw(8) << notaStr
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
                  << "\n";
    }
}

// ============================================================================
// STATUS "FEITA" DAS RECEITAS
// ============================================================================
void marcarReceitaComoFeita(Database& db) {
    std::cout << "\n--- Marcar Receita como Feita/Nao Feita ---\n";
    std::cout << "Digite o ID da receita: ";
    
    int id;
    std::cin >> id;
    
    Receita receita = db.consultarPorId(id);
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    std::cout << "Receita: " << receita.nome << "\n";
    std::cout << "Status atual: " << (receita.feita ? "Feita" : "Nao feita") << "\n";
    std::cout << "Marcar como feita? (s/n): ";
    
    char resposta;
    std::cin >> resposta;
    bool feita = (resposta == 's' || resposta == 'S');
    
    if (db.marcarReceitaComoFeita(id, feita)) {
        std::cout << "Status atualizado com sucesso!\n";
    } else {
        std::cout << "Erro ao atualizar status.\n";
    }
}

void listarReceitasFeitas(Database& db) {
    std::cout << "\n--- Lista de Receitas Feitas ---\n";
    
    auto receitas = db.getReceitasFeitas();
    
    if (receitas.empty()) {
        std::cout << "Nenhuma receita feita encontrada.\n";
        return;
    }
    
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << std::setw(15) << "Categoria" 
              << std::setw(10) << "Tempo" 
              << std::setw(10) << "Porcoes" 
              << std::setw(8) << "Feita"
              << std::setw(8) << "Nota"
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(116, '-') << "\n";
    
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
        
        std::string notaStr = (r.nota > 0) ? std::to_string(r.nota) : "-";
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(8) << (r.feita ? "Sim" : "Nao")
                  << std::setw(8) << notaStr
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
                  << "\n";
    }
}

// ============================================================================
// AVALIAÇÃO DE RECEITAS
// ============================================================================
void avaliarReceita(Database& db) {
    std::cout << "\n--- Avaliar Receita ---\n";
    std::cout << "Digite o ID da receita: ";
    
    int id;
    std::cin >> id;
    
    Receita receita = db.consultarPorId(id);
    if (receita.id == 0) {
        std::cout << "Receita nao encontrada.\n";
        return;
    }
    
    if (!receita.feita) {
        std::cout << "A receita precisa ser marcada como feita antes de ser avaliada.\n";
        return;
    }
    
    std::cout << "Receita: " << receita.nome << "\n";
    if (receita.nota > 0) {
        std::cout << "Nota atual: " << receita.nota << "/5\n";
    }
    std::cout << "Digite a nota (1 a 5): ";
    
    int nota;
    std::cin >> nota;
    
    if (nota < 1 || nota > 5) {
        std::cout << "Nota deve estar entre 1 e 5.\n";
        return;
    }
    
    if (db.avaliarReceita(id, nota)) {
        std::cout << "Receita avaliada com sucesso!\n";
    } else {
        std::cout << "Erro ao avaliar receita.\n";
    }
}

void filtrarReceitasPorNota(Database& db) {
    std::cout << "\n--- Filtrar Receitas por Nota ---\n";
    std::cout << "Digite a nota (1 a 5): ";
    
    int nota;
    std::cin >> nota;
    
    if (nota < 1 || nota > 5) {
        std::cout << "Nota deve estar entre 1 e 5.\n";
        return;
    }
    
    auto receitas = db.getReceitasPorNota(nota);
    
    if (receitas.empty()) {
        std::cout << "Nenhuma receita encontrada com nota " << nota << ".\n";
        return;
    }
    
    std::cout << "\nReceitas com nota " << nota << ":\n";
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(30) << "Nome" 
              << std::setw(15) << "Categoria" 
              << std::setw(10) << "Tempo" 
              << std::setw(10) << "Porcoes" 
              << std::setw(8) << "Feita"
              << std::setw(8) << "Nota"
              << std::setw(30) << "Tags"
              << "\n";
    std::cout << std::string(116, '-') << "\n";
    
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
        
        std::string notaStr = (r.nota > 0) ? std::to_string(r.nota) : "-";
        
        std::cout << std::left << std::setw(5) << r.id 
                  << std::setw(30) << (r.nome.length() > 28 ? r.nome.substr(0, 27) + ".." : r.nome)
                  << std::setw(15) << (r.categoria.length() > 13 ? r.categoria.substr(0, 12) + ".." : r.categoria)
                  << std::setw(10) << r.tempo 
                  << std::setw(10) << r.porcoes 
                  << std::setw(8) << (r.feita ? "Sim" : "Nao")
                  << std::setw(8) << notaStr
                  << std::setw(30) << (tagsStr.length() > 28 ? tagsStr.substr(0, 27) + ".." : tagsStr)
                  << "\n";
    }
}

// ============================================================================
// BACKUP E RESTAURAÇÃO
// ============================================================================
void fazerBackup(Database& db) {
    std::cout << "\n--- Fazer Backup do Banco de Dados ---\n";
    
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    
    std::string nomePadrao = "./backups/recipes_backup_" + std::string(timestamp) + ".db";
    
    std::cout << "Caminho do backup (Enter para usar: " << nomePadrao << "): ";
    limparBuffer();
    std::string caminhoBackup;
    std::getline(std::cin, caminhoBackup);
    
    if (caminhoBackup.empty()) {
        caminhoBackup = nomePadrao;
    }
    
    std::cout << "Fazendo backup para: " << caminhoBackup << "\n";
    
    if (db.fazerBackup(caminhoBackup)) {
        if (std::filesystem::exists(caminhoBackup)) {
            auto tamanho = std::filesystem::file_size(caminhoBackup);
            std::cout << "Backup concluido com sucesso!\n";
            std::cout << "Tamanho do arquivo: " << tamanho << " bytes\n";
            
            sqlite3* verifyDb = nullptr;
            if (sqlite3_open_v2(caminhoBackup.c_str(), &verifyDb, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK) {
                sqlite3_stmt* stmt;
                const char* sql = "SELECT COUNT(*) FROM receitas";
                if (sqlite3_prepare_v2(verifyDb, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        int count = sqlite3_column_int(stmt, 0);
                        std::cout << "Receitas no backup: " << count << "\n";
                    }
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(verifyDb);
            }
        } else {
            std::cout << "Backup concluido, mas arquivo nao encontrado.\n";
        }
    } else {
        std::cout << "Erro ao fazer backup.\n";
    }
}

std::vector<std::filesystem::path> listarBackups() {
    std::vector<std::filesystem::path> backups;
    std::string backupDir = "./backups";
    
    if (!std::filesystem::exists(backupDir)) {
        std::cout << "\nDiretorio de backups nao encontrado: " << backupDir << "\n";
        return backups;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".db") {
            backups.push_back(entry.path());
        }
    }
    
    if (backups.empty()) {
        std::cout << "\nNenhum backup encontrado em " << backupDir << "\n";
        return backups;
    }
    
    std::sort(backups.begin(), backups.end(), 
        [](const std::filesystem::path& a, const std::filesystem::path& b) {
            return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });
    
    std::cout << "\n--- Backups Disponiveis ---\n";
    std::cout << std::left << std::setw(5) << "#" 
              << std::setw(50) << "Arquivo" 
              << std::setw(15) << "Tamanho" 
              << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (size_t i = 0; i < backups.size(); ++i) {
        auto tamanho = std::filesystem::file_size(backups[i]);
        std::string tamanhoStr = std::to_string(tamanho) + " bytes";
        if (tamanho > 1024) {
            tamanhoStr = std::to_string(tamanho / 1024) + " KB";
        }
        if (tamanho > 1024 * 1024) {
            tamanhoStr = std::to_string(tamanho / (1024 * 1024)) + " MB";
        }
        
        std::cout << std::left << std::setw(5) << (i + 1)
                  << std::setw(50) << backups[i].filename().string()
                  << std::setw(15) << tamanhoStr
                  << "\n";
    }
    
    return backups;
}

void restaurarBackup(Database& db) {
    std::cout << "\n--- Restaurar Backup do Banco de Dados ---\n";
    std::cout << "ATENCAO: Esta operacao substituira o banco de dados atual!\n";
    
    auto backups = listarBackups();
    
    if (backups.empty()) {
        return;
    }
    
    std::cout << "\nDigite o numero do backup (1-" << backups.size() << ") ou o caminho completo: ";
    limparBuffer();
    std::string entrada;
    std::getline(std::cin, entrada);
    
    if (entrada.empty()) {
        std::cout << "Operacao cancelada.\n";
        return;
    }
    
    std::string caminhoBackup;
    
    // Verificar se é um número
    bool ehNumero = true;
    for (char c : entrada) {
        if (!std::isdigit(c)) {
            ehNumero = false;
            break;
        }
    }
    
    if (ehNumero) {
        int numero = std::stoi(entrada);
        if (numero >= 1 && numero <= static_cast<int>(backups.size())) {
            caminhoBackup = backups[numero - 1].string();
            std::cout << "Backup selecionado: " << backups[numero - 1].filename().string() << "\n";
        } else {
            std::cout << "Numero invalido. Deve estar entre 1 e " << backups.size() << ".\n";
            return;
        }
    } else {
        // É um caminho completo
        caminhoBackup = entrada;
    }
    
    if (!std::filesystem::exists(caminhoBackup)) {
        std::cout << "Arquivo de backup nao encontrado: " << caminhoBackup << "\n";
        return;
    }
    
    std::cout << "Tem certeza que deseja restaurar o backup? (s/n): ";
    char confirmacao;
    std::cin >> confirmacao;
    
    if (confirmacao != 's' && confirmacao != 'S') {
        std::cout << "Operacao cancelada.\n";
        return;
    }
    
    std::cout << "Restaurando backup de: " << caminhoBackup << "\n";
    
    if (db.restaurarBackup(caminhoBackup)) {
        std::cout << "\nBackup restaurado com sucesso!\n";
        std::cout << "O banco de dados foi restaurado e a conexao foi reaberta.\n";
        std::cout << "Voce pode agora listar as receitas para verificar.\n";
    } else {
        std::cout << "Erro ao restaurar backup.\n";
    }
}

// ============================================================================
// FUNÇÃO PRINCIPAL
// ============================================================================
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
            case 10:
                marcarReceitaComoFeita(db);
                break;
            case 11:
                listarReceitasFeitas(db);
                break;
            case 12:
                avaliarReceita(db);
                break;
            case 13:
                filtrarReceitasPorNota(db);
                break;
            case 98:
                fazerBackup(db);
                break;
            case 99:
                restaurarBackup(db);
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
