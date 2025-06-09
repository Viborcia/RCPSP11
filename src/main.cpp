#include "RCPSPLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "SimulatedAnnealingSolver.h"
#include "GreedySolver.h"
#include "EvolutionSolver.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <fstream>
#include <numeric>
using namespace std;
#include <iomanip>
#include <locale>

int optimum = 125; // ← ustaw raz globalnie w main.cpp
//43,47,47
//77,68,68
//73,92,66
//105,109,125
void zapiszFitnessDoCSV(const std::string& algorytm,
                        const std::string& instancja,
                        int bestMakespan,
                        int criticalPath,
                        const std::vector<int>& makespanyZRunow,
                        const std::string& nazwaPliku = "porownanie_fitness.csv")
{
    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(6);

    double avg = std::accumulate(makespanyZRunow.begin(), makespanyZRunow.end(), 0.0) / makespanyZRunow.size();

    double sumaKwadratow = 0.0;
    for (int val : makespanyZRunow)
        sumaKwadratow += (val - avg) * (val - avg);
    double stddev = std::sqrt(sumaKwadratow / makespanyZRunow.size());

    double avgDev = 100.0 * (bestMakespan - criticalPath) / (double)criticalPath;
    double devFromOptimum = 100.0 * (bestMakespan - optimum) / (double)optimum;

    // Nagłówek CSV — tylko jeśli plik nowy
    if (!istnieje)
        out << "algorytm;instancja;makespan;critical_path;avgDevCPM;std;devFromOptimum;optimum\n";

    // Wiersz danych
    out << algorytm << ";" << instancja << ";" << bestMakespan << ";" << criticalPath << ";"
        << avgDev << ";" << stddev << ";" << devFromOptimum << ";" << optimum << "\n";

    out.close();
}




std::string wyciagnijNazwePliku(const std::string& sciezka) {
    size_t pos = sciezka.find_last_of("/\\");
    if (pos == std::string::npos)
        return sciezka;
    return sciezka.substr(pos + 1);
}


int main() 
{
    
//if (!loader.wczytajZPliku("C:\\Users\\MICHA~1\\Desktop\\RCPSP11-main\\RCPSP11-main\\RCPSP11\\src\\j1201_1.sm")) 
//if (!loader.wczytajZPliku("C:\\Users\\micha\\Desktop\\RCPSP11-main\\RCPSP11-main\\RCPSP11\\src\\j301_1.sm"))
//if (!loader.wczytajZPliku("j901_1.sm")) 

RCPSPLoader loader;
//std::string sciezkaDoPliku = "C:\\Users\\Zuzia\\Desktop\\RCPSP1\\j120.sm\\j1201_1.sm";
std::string sciezkaDoPliku = "C:\\Users\\micha\\Desktop\\RCPSP11-main\\RCPSP11-main\\RCPSP11\\src\\j1201_3.sm";

std::string nazwaInstancji = wyciagnijNazwePliku(sciezkaDoPliku);

if (!loader.wczytajZPliku(sciezkaDoPliku))
{
    std::cerr << "Błąd wczytywania instancji.\n";
    return 1;
}
//loader.wypisz();

   

    auto startwsio = std::chrono::high_resolution_clock::now();
    
    int liczbaUruchomien = 10;
    


    // === RANDOM SOLVER ===
auto startRand = std::chrono::high_resolution_clock::now();

int randIteracji = 1;
int najlepszyRun = -1;
int najlepszyKoszt = std::numeric_limits<int>::max();
RandomSolver najlepszySolver(randIteracji); 
std::vector<int> makespanyRandom;

for (int run = 0; run < liczbaUruchomien; ++run)
{
    RandomSolver rsolver(randIteracji);
    rsolver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);

    rsolver.zapiszWykorzystanieZasobow("zasoby_random.csv", loader.getLiczbaZasobow());
    rsolver.zapiszStatystykiDoCSV("wyniki_random.csv", run);

    int wynik = rsolver.getMakespan();
    makespanyRandom.push_back(wynik);

    if (wynik < najlepszyKoszt)
    {
        najlepszyKoszt = wynik;
        najlepszyRun = run;
        najlepszySolver = rsolver;
    }
}

    najlepszySolver.zapiszDoCSV("harmonogram_random.csv");

    int sciezkarand = obliczDlugoscSciezkiKrytycznej(najlepszySolver.getSchedule());

    zapiszFitnessDoCSV("Random", nazwaInstancji, najlepszyKoszt, sciezkarand, makespanyRandom);

std::cout << "Najlepszy RANDOM run: #" << najlepszyRun << "\n";
std::cout << "Koszt (makespan): " << najlepszyKoszt << "\n";

auto stopRand = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedRandom = stopRand - startRand;
std::cout << "[RandomSolver] Czas wykonania: " << elapsedRandom.count() << " sekund\n";



  // === GREEDY SOLVER ===
auto startGreedy = std::chrono::high_resolution_clock::now();

GreedySolver greedy;
greedy.solveAuto(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
greedy.zapiszDoCSV("harmonogram_greedy.csv");
greedy.zapiszWykorzystanieZasobow("zasoby_greedy.csv", loader.liczbaZasobow);

int sciezka = obliczDlugoscSciezkiKrytycznej(greedy.getSchedule());

std::vector<int> jedenWynikGreedy = {greedy.getMakespan()};
zapiszFitnessDoCSV("Greedy", nazwaInstancji, greedy.getMakespan(), sciezka, jedenWynikGreedy);

std::cout << "Greedy Makespan: " << greedy.getMakespan() << std::endl;

auto stopGreedy = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedGreedy = stopGreedy - startGreedy;
std::cout << "[GreedySolver] Czas wykonania: " << elapsedGreedy.count() << " sekund" << std::endl;

 
   // === TABU SEARCH ===
auto startTS = std::chrono::high_resolution_clock::now();

int tabuIteracje = 1000;
int dlugoscTabu = 100;
int najlepszyRunTS = -1;
int najlepszyKosztTS = std::numeric_limits<int>::max();

std::vector<int> makespanyTabu;  

zapiszZadaniaDoCSV(loader.zadania, "zadania_input.csv");
TabuSearchSolver najlepszyTabu(tabuIteracje, dlugoscTabu);

for (int run = 0; run < liczbaUruchomien; ++run)
{
    TabuSearchSolver solver(tabuIteracje, dlugoscTabu);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
    solver.zapiszStatystykiDoCSV("wyniki_tabu.csv", run);

    int wynik = solver.getMakespan();
    makespanyTabu.push_back(wynik);  // dodaj wynik tego runa do wektora

    if (wynik < najlepszyKosztTS)
    {
        najlepszyKosztTS = wynik;
        najlepszyRunTS = run;
        najlepszyTabu = solver;

        najlepszyTabu.zapiszWykorzystanieZasobow("zasoby_tabu.csv", loader.getLiczbaZasobow());
    }
}

najlepszyTabu.zapiszDoCSV("harmonogram_tabu.csv");
najlepszyTabu.zapiszBestVsCurrentCSV("best_vs_current_tabu.csv");
//najlepszyTabu.zapiszKosztyNajlepszegoRunCSV("koszty_tabu.csv");
zapiszFitnessDoCSV("Tabu", nazwaInstancji, najlepszyTabu.getMakespan(), sciezka, makespanyTabu);

std::cout << "Najlepszy TABU run: #" << najlepszyRunTS << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztTS << "\n";

auto stopTS = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedTS = stopTS - startTS;
std::cout << "[TabuSearch] Czas wykonania: " << elapsedTS.count() << " sekund\n";


    // === SIMULATED ANNEALING ===
auto startSA = std::chrono::high_resolution_clock::now();

double startTemp = 10000.0;
double endTemp = 0.1;
double coolingRate = 0.997;
int maxIter = 10000;

int najlepszyRunSA = -1;
int najlepszyKosztSA = std::numeric_limits<int>::max();
SimulatedAnnealingSolver najlepszySA(startTemp, endTemp, coolingRate, maxIter);

std::vector<int> makespanySA; // zbieramy makespany z każdego runa

for (int run = 0; run < liczbaUruchomien; ++run)
{
    SimulatedAnnealingSolver solver(startTemp, endTemp, coolingRate, maxIter);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);

    solver.zapiszStatystykiDoCSV("wyniki_sa.csv", run);

    int wynik = solver.getMakespan();
    makespanySA.push_back(wynik);  // dodajemy wynik tego runa

    if (wynik < najlepszyKosztSA)
    {
        najlepszyKosztSA = wynik;
        najlepszyRunSA = run;
        najlepszySA = solver;

        najlepszySA.zapiszWykorzystanieZasobow("zasoby_sa.csv", loader.getLiczbaZasobow());
    }
}

// Zapisz tylko harmonogram najlepszego rozwiązania
najlepszySA.zapiszDoCSV("harmonogram_sa.csv");
najlepszySA.zapiszBestVsCurrentCSV("best_vs_current_sa.csv");

int sciezkaSA = obliczDlugoscSciezkiKrytycznej(najlepszySA.getSchedule());

// Zapisz końcowy rekord fitness do porownanie_fitness.csv
zapiszFitnessDoCSV("SA", nazwaInstancji, najlepszyKosztSA, sciezkaSA, makespanySA);

std::cout << "Najlepszy SA run: #" << najlepszyRunSA << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztSA << "\n";

auto stopSA = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedSA = stopSA - startSA;
std::cout << "[SimulatedAnnealing] Czas wykonania: " << elapsedSA.count() << " sekund\n";

/*

auto startEA = std::chrono::high_resolution_clock::now();

int populacja = 500;
int pokolenia = 1000;
double prawdMutacji = 0.01;
double prawdKrzyzowania = 0.3;
int rozmiarTurnieju = 3;

int najlepszyRunEA = -1;
int najlepszyKosztEA = std::numeric_limits<int>::max();
EvolutionSolver najlepszyEA(populacja, pokolenia, prawdMutacji, prawdKrzyzowania, rozmiarTurnieju);

for (int run = 0; run < liczbaUruchomien; ++run)
{
    EvolutionSolver solver(populacja, pokolenia, prawdMutacji, prawdKrzyzowania, rozmiarTurnieju);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
    solver.zapiszStatystykiDoCSV("wyniki_evolution.csv", run);

    if (solver.getMakespan() < najlepszyKosztEA)
    {
        najlepszyKosztEA = solver.getMakespan();
        najlepszyRunEA = run;
        najlepszyEA = solver;
        najlepszyEA.zapiszWykorzystanieZasobow("zasoby_evolution.csv", loader.getLiczbaZasobow());
        int sciezka = obliczDlugoscSciezkiKrytycznej(solver.getSchedule());
        zapiszFitnessDoCSV("EA", nazwaInstancji, najlepszyEA.getMakespan(), sciezka);

    }
}

najlepszyEA.zapiszDoCSV("harmonogram_evolution.csv");
std::cout << "Najlepszy EVOLUTION run: #" << najlepszyRunEA << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztEA << "\n";

auto stopEA = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedEA = stopEA - startEA;
std::cout << "[EvolutionSolver] Czas wykonania: " << elapsedEA.count() << " sekund\n";
*/

auto stopwsio = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedwsio = stopwsio - startwsio;
    std::cout << "[Caly Algorytm] Czas wykonania: " << elapsedwsio.count() << " sekund\n";

    return 0;
}
