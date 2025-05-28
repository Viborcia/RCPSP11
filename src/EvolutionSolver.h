#ifndef EVOLUTIONSOLVER_H
#define EVOLUTIONSOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <random> 

class EvolutionSolver {
public:
    EvolutionSolver(int populacja, int pokolenia, double prawdopodobMutacji, double prawdopodobKrzyzowania, int tourSize);
    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
    void printSchedule() const;

    struct Individual {
        std::vector<int> priorytety;
        int fitness; // ujednolicone z cpp
    };

private:
    int rozmiarPopulacji;
    int liczbaPokolen;
    double prawdopodobienstwoMutacji;
    double prawdopodobienstwoKrzyzowania;
    int tourSize;
    int liczbaJobow;
    int liczbaMaszyn;

    int najlepszyMakespan;

    std::vector<Individual> populacja;
    std::vector<OperationSchedule> najlepszyHarmonogram;

    Individual stworzLosowyOsobnik(int liczbaOperacji);
    void mutacjaSwap(Individual& individual);
std::pair<Individual, Individual> krzyzowanieOX(const Individual& p1, const Individual& p2, std::mt19937& gen);
    int ocenOsobnik(Individual& individual, const std::vector<OperationSchedule>& operacje);
Individual turniej(const std::vector<Individual>& populacja, int tourSize, std::mt19937& gen);
    std::vector<OperationSchedule> budujHarmonogram(const Individual& individual, const std::vector<OperationSchedule>& operacje);
};

#endif // EVOLUTIONSOLVER_H
