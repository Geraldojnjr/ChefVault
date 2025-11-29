#ifndef RECEITA_H
#define RECEITA_H

#include <string>

struct Receita {
    int id;
    std::string nome;
    std::string ingredientes;
    std::string preparo;
    int tempo;
    std::string categoria;
    int porcoes;

    Receita() : id(0), tempo(0), porcoes(0) {}
    
    Receita(const std::string& nome, const std::string& ingredientes, 
            const std::string& preparo, int tempo, 
            const std::string& categoria, int porcoes)
        : id(0), nome(nome), ingredientes(ingredientes), preparo(preparo),
          tempo(tempo), categoria(categoria), porcoes(porcoes) {}
};

#endif // RECEITA_H

