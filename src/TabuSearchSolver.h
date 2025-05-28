#ifndef TABU_SEARCH_SOLVER_H
#define TABU_SEARCH_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class TabuSearchSolver
{
public:
    TabuSearchSolver(int liczbaIteracji, int dlugoscTabu);

    void solve(const std::vector<Activity>& zadaniaWejscie,
               int liczbaZadan,
               int liczbaZasobow,
               const std::vector<int>& pojemnosci);

    int getMakespan() const { return makespan; }
    void printSchedule() const;

    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
    void zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const;
    void zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const;
    const std::vector<Activity>& getSchedule() const { return schedule; }


private:
    int liczbaIteracji;
    int dlugoscTabu;
    int makespan;

    std::vector<Activity> schedule;

    std::vector<int> kosztyIteracji;
    std::vector<int> historiaCurrent;
    std::vector<int> historiaBestSoFar;
    std::vector<double> avgIteracji;
    std::vector<int> worstIteracji;

    // Funkcje pomocnicze (jako metody prywatne)
    std::vector<Activity> zbudujHarmonogramZPriorytetami(
        const std::vector<int>& priorytety,
        const std::vector<Activity>& zadaniaWejscie,
        int liczbaZadan,
        int liczbaZasobow,
        const std::vector<int>& pojemnosci);

    int obliczMakespan(const std::vector<Activity>& harmonogram) const;
};

#endif
