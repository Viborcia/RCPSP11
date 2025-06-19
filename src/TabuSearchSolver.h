#ifndef TABU_SEARCH_SOLVER_H
#define TABU_SEARCH_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class TabuSearchSolver
{
public:
    // Konstruktor
    TabuSearchSolver(int liczbaIteracji, int dlugoscTabu);

    // Główna funkcja rozwiązująca
    void solve(const std::vector<Activity>& zadaniaWejscie,
               int liczbaZadan,
               int liczbaZasobow,
               const std::vector<int>& pojemnosci);

    // Dostęp do wyniku
    int getMakespan() const { return makespan; }
    const std::vector<Activity>& getSchedule() const { return schedule; }

    // Pomocnicze wypisywanie i zapisywanie
    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
    void zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const;
    void zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const;

private:
    // Parametry algorytmu
    int liczbaIteracji;
    int dlugoscTabu;
    int makespan;

    // Harmonogram
    std::vector<Activity> schedule;

    // Statystyki
    std::vector<int> kosztyIteracji;
    std::vector<int> historiaCurrent;
    std::vector<int> historiaBestSoFar;
    std::vector<double> avgIteracji;
    std::vector<int> worstIteracji;

    // Obliczanie makespanu
    int obliczMakespan(const std::vector<Activity>& harmonogram) const;
};

// Funkcja pomocnicza (nie należy do klasy)
void zapiszZadaniaDoCSV(const std::vector<Activity>& zadania, const std::string& nazwaPliku);

#endif
