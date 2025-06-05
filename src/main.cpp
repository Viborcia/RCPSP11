#include "RCPSPLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "SimulatedAnnealingSolver.h"
#include "GreedySolver.h"
#include "EvolutionSolver.h"

#include <chrono>
#include <iostream>
#include <fstream>
using namespace std;




void zapiszFitnessDoCSV(const std::string& algorytm, const std::string& instancja, int makespan, int criticalPath, const std::string& nazwaPliku = "porownanie_fitness.csv") {
    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
        out << "algorytm;instancja;makespan;critical_path;avgDevCPM\n";

    double avgDev = 100.0 * (makespan - criticalPath) / (double)criticalPath;

    out << algorytm << ";" << instancja << ";" << makespan << ";" << criticalPath << ";" << avgDev << "\n";
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
std::string sciezkaDoPliku = "C:\\Users\\micha\\Desktop\\RCPSP11-main\\RCPSP11-main\\RCPSP11\\src\\j1201_1.sm";
std::string nazwaInstancji = wyciagnijNazwePliku(sciezkaDoPliku);

if (!loader.wczytajZPliku(sciezkaDoPliku))


{
    std::cerr << "Błąd wczytywania instancji.\n";
    return 1;
}
//loader.wypisz();

   

    auto startwsio = std::chrono::high_resolution_clock::now();
    
    int liczbaUruchomien = 1;
    /*

    // === RANDOM SOLVER ===
auto startRand = std::chrono::high_resolution_clock::now();

int randIteracji = 1;
int najlepszyRun = -1;
int najlepszyKoszt = std::numeric_limits<int>::max();
RandomSolver najlepszySolver(randIteracji); 

for (int run = 0; run < liczbaUruchomien; ++run)
{
    RandomSolver solver(randIteracji);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
    solver.zapiszWykorzystanieZasobow("zasoby_random.csv", loader.getLiczbaZasobow());

    solver.zapiszStatystykiDoCSV("wyniki_random.csv", run);

    if (solver.getMakespan() < najlepszyKoszt)
    {
        najlepszyKoszt = solver.getMakespan();
        najlepszyRun = run;
        najlepszySolver = solver; 
    }
}

najlepszySolver.zapiszDoCSV("harmonogram_random.csv");
int sciezkarand = obliczDlugoscSciezkiKrytycznej(najlepszySolver.getSchedule());
zapiszFitnessDoCSV("Random", nazwaInstancji, najlepszySolver.getMakespan(), sciezkarand);

std::cout << "Najlepszy RANDOM run: #" << najlepszyRun << "\n";
std::cout << "Koszt (makespan): " << najlepszyKoszt << "\n";

auto stopRand = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedRandom = stopRand - startRand;
std::cout << "[RandomSolver] Czas wykonania: " << elapsedRandom.count() << " sekund\n";



   // === GREEDY SOLVER ===
auto startGreedy = std::chrono::high_resolution_clock::now();

GreedySolver greedy;
greedy.solveAuto(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
//greedy.printSchedule();
greedy.zapiszDoCSV("harmonogram_greedy.csv");
greedy.zapiszWykorzystanieZasobow("zasoby_greedy.csv", loader.liczbaZasobow);
int sciezka = obliczDlugoscSciezkiKrytycznej(greedy.getSchedule());
zapiszFitnessDoCSV("Greedy", nazwaInstancji, greedy.getMakespan(), sciezka);


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
TabuSearchSolver najlepszyTabu(tabuIteracje, dlugoscTabu);
zapiszZadaniaDoCSV(loader.zadania, "zadania_input.csv");


for (int run = 0; run < liczbaUruchomien; ++run)
{
    TabuSearchSolver solver(tabuIteracje, dlugoscTabu);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
    
    solver.zapiszStatystykiDoCSV("wyniki_tabu.csv", run);
    

    if (solver.getMakespan() < najlepszyKosztTS)
    {
        najlepszyKosztTS = solver.getMakespan();
        najlepszyRunTS = run;
        najlepszyTabu = solver;
        najlepszyTabu.zapiszWykorzystanieZasobow("zasoby_tabu.csv", loader.getLiczbaZasobow());
        int sciezka = obliczDlugoscSciezkiKrytycznej(solver.getSchedule());
        zapiszFitnessDoCSV("Tabu", nazwaInstancji, solver.getMakespan(), sciezka);
    }
}

najlepszyTabu.zapiszDoCSV("harmonogram_tabu.csv");
najlepszyTabu.zapiszBestVsCurrentCSV("best_vs_current_tabu.csv");
//najlepszyTabu.zapiszKosztyNajlepszegoRunCSV("koszty_tabu.csv");

std::cout << "Najlepszy TABU run: #" << najlepszyRunTS << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztTS << "\n";

auto stopTS = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedTS = stopTS - startTS;
std::cout << "[TabuSearch] Czas wykonania: " << elapsedTS.count() << " sekund\n";


    // === SIMULATED ANNEALING ===
auto startSA = std::chrono::high_resolution_clock::now();

double startTemp = 100000.0;          
double endTemp = .1;                
double coolingRate = 0.95;          
int maxIter = 10000;                
int najlepszyRunSA = -1;
int najlepszyKosztSA = std::numeric_limits<int>::max();
SimulatedAnnealingSolver najlepszySA(startTemp, endTemp, coolingRate, maxIter);

for (int run = 0; run < liczbaUruchomien; ++run)
{
    SimulatedAnnealingSolver solver(startTemp, endTemp, coolingRate, maxIter);
    solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);


    solver.zapiszStatystykiDoCSV("wyniki_sa.csv", run);
    


    if (solver.getMakespan() < najlepszyKosztSA)
    {
        najlepszyKosztSA = solver.getMakespan();
        najlepszyRunSA = run;
        najlepszySA = solver;
        najlepszySA.zapiszWykorzystanieZasobow("zasoby_sa.csv", loader.getLiczbaZasobow());
        int sciezka = obliczDlugoscSciezkiKrytycznej(solver.getSchedule());
        zapiszFitnessDoCSV("SA", nazwaInstancji, solver.getMakespan(), sciezka);
    }
}

// Zapisz tylko harmonogram najlepszego rozwiązania
najlepszySA.zapiszDoCSV("harmonogram_sa.csv");
//najlepszySA.zapiszKosztyNajlepszegoRunCSV("koszty_sa.csv");
najlepszySA.zapiszBestVsCurrentCSV("best_vs_current_sa.csv");


std::cout << "Najlepszy SA run: #" << najlepszyRunSA << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztSA << "\n";

auto stopSA = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedSA = stopSA - startSA;
std::cout << "[SimulatedAnnealing] Czas wykonania: " << elapsedSA.count() << " sekund\n";

*/
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
    }
}

najlepszyEA.zapiszDoCSV("harmonogram_evolution.csv");

std::cout << "Najlepszy EVOLUTION run: #" << najlepszyRunEA << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztEA << "\n";

auto stopEA = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedEA = stopEA - startEA;
std::cout << "[EvolutionSolver] Czas wykonania: " << elapsedEA.count() << " sekund\n";

auto stopwsio = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedwsio = stopwsio - startwsio;
    std::cout << "[Caly Algorytm] Czas wykonania: " << elapsedwsio.count() << " sekund\n";
  
    return 0;
}
