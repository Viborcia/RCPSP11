#ifndef GREEDY_SOLVER_H
#define GREEDY_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class GreedySolver {
public:
    enum TrybSortowania {
        NAJKROTSZY_CZAS,
        NAJWIECEJ_SUCCESSOROW,
        PRODUKT_DURATION_SUCCESSORS,
        NAJWIEKSZE_ZASOBY
    };

    void setTrybSortowania(TrybSortowania t) { tryb = t; }

    void solve(const std::vector<Activity>& zadaniaWejscie,
               int liczbaZadan,
               int liczbaZasobow,
               const std::vector<int>& pojemnosci);

    void solveAuto(const std::vector<Activity>& zadaniaWejscie,
                   int liczbaZadan,
                   int liczbaZasobow,
                   const std::vector<int>& pojemnosci);

    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    int getMakespan() const { return makespan; }
    const std::vector<Activity>& getSchedule() const { return schedule; }


private:
    std::vector<Activity> schedule;
    int makespan = 0;
    TrybSortowania tryb = NAJKROTSZY_CZAS;
};

#endif
