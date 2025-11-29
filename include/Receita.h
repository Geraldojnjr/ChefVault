#ifndef RECEITA_H
#define RECEITA_H

#include <string>
#include <vector>
#include <sstream>

struct Ingrediente {
    int id;
    std::string nome;
    double quantidade;
    std::string unidade;
    
    Ingrediente() : id(0), quantidade(0.0) {}
    
    Ingrediente(const std::string& nome, double quantidade, const std::string& unidade)
        : id(0), nome(nome), quantidade(quantidade), unidade(unidade) {}
    
    std::string formatar() const {
        std::ostringstream oss;
        oss << quantidade;
        std::string qtdStr = oss.str();
        size_t pos = qtdStr.find_last_not_of('0');
        if (pos != std::string::npos && qtdStr[pos] == '.') {
            pos--;
        }
        if (pos != std::string::npos) {
            qtdStr = qtdStr.substr(0, pos + 1);
        }
        
        if (unidade.empty() || unidade == "unidade" || unidade == "un") {
            return qtdStr + " " + nome;
        } else {
            return qtdStr + " " + unidade + (quantidade > 1.0 && unidade.back() != 's' ? "s" : "") + " de " + nome;
        }
    }
};

struct Receita {
    int id;
    std::string nome;
    std::string ingredientes; 
    std::vector<Ingrediente> ingredientesEstruturados;
    std::string preparo;
    int tempo;
    std::string categoria;
    int porcoes;
    bool feita;
    int nota;
    std::string imagem; // Caminho para o arquivo de imagem
    std::vector<std::string> tags;

    Receita() : id(0), tempo(0), porcoes(0), feita(false), nota(0) {}
    
    Receita(const std::string& nome, const std::string& ingredientes, 
            const std::string& preparo, int tempo, 
            const std::string& categoria, int porcoes)
        : id(0), nome(nome), ingredientes(ingredientes), preparo(preparo),
          tempo(tempo), categoria(categoria), porcoes(porcoes), feita(false), nota(0) {}
    
    void atualizarIngredientesString() {
        ingredientes = "";
        for (size_t i = 0; i < ingredientesEstruturados.size(); ++i) {
            ingredientes += ingredientesEstruturados[i].formatar();
            if (i < ingredientesEstruturados.size() - 1) {
                ingredientes += ", ";
            }
        }
    }
};

#endif // RECEITA_H

