#pragma once
#include <vector>
#include <string>
#include "Activity.h"

class SimulatedAnnealingSolver {
public:
    SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter);

    void solve(const std::vector<Activity>& zadaniaWejscie, int liczbaZadan, int liczbaZasobow, const std::vector<int>& pojemnosci);
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;
    void printSchedule() const;
void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
void zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const;
void zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const;
int getMakespan() const { return makespan; }
const std::vector<Activity>& getSchedule() const { return schedule; }




private:
    int maksLiczbaIteracji;
    double temperaturaStartowa;
    double temperaturaKoncowa;
    double wspolczynnikChlodzenia;
    std::vector<int> kosztyIteracji;

    int makespan;
    std::vector<Activity> schedule;

    // Statystyki
    std::vector<double> historiaCurrent;
    std::vector<double> historiaBestSoFar;
    std::vector<double> avgIteracji;
    std::vector<double> worstIteracji;
};

// Funkcje pomocnicze (nie są częścią klasy!)
int obliczMakespan(const std::vector<Activity>& harmonogram);

std::vector<Activity> generujHarmonogram(
    const std::vector<int>& priorytety,
    const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci);
