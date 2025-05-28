#ifndef RANDOM_SOLVER_H
#define RANDOM_SOLVER_H

#include "Activity.h"

#include <vector>
#include <string>


class RandomSolver {
public:
    RandomSolver(int liczbaProb);

   void solve(const std::vector<Activity>& zadania, int liczbaZadan, int liczbaZasobow, const std::vector<int>& pojemnosci);

    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    int getMakespan() const { return makespan; }
    const std::vector<Activity>& getSchedule() const { return schedule; }


private:
    std::vector<Activity> schedule;
    int makespan;
    int liczbaProb;
    std::vector<double> kosztyProb;
};

#endif 
