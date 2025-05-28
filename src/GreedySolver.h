#ifndef GREEDY_SOLVER_H
#define GREEDY_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class GreedySolver {
public:
    void solve(const std::vector<Activity>& zadaniaWejscie,
               int liczbaZadan,
               int liczbaZasobow,
               const std::vector<int>& pojemnosci);

    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    int getMakespan() const { return makespan; }

private:
    std::vector<Activity> schedule;
    int makespan;
};

#endif
