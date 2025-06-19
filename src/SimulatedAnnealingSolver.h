#ifndef SIMULATED_ANNEALING_SOLVER_H
#define SIMULATED_ANNEALING_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class SimulatedAnnealingSolver
{
private:
    double temperaturaStartowa;
    double temperaturaKoncowa;
    double wspolczynnikChlodzenia;
    int maksLiczbaIteracji;

    std::vector<int> kosztyIteracji;
    std::vector<int> historiaBestSoFar;
    std::vector<int> historiaCurrent;

    std::vector<int> avgIteracji;
    std::vector<int> worstIteracji;

public:
    int makespan;
    std::vector<Activity> schedule;

    SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter);

    void solve(const std::vector<Activity>& zadaniaWejscie,
               int liczbaZadan,
               int liczbaZasobow,
               const std::vector<int>& pojemnosci);

    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    void zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const;
    void zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const;
    int getMakespan() const { return makespan; }
const std::vector<Activity>& getSchedule() const { return schedule; }

};

int obliczMakespan(const std::vector<Activity>& harmonogram);

#endif
