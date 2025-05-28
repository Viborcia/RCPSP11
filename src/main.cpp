#include "RCPSPLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "SimulatedAnnealingSolver.h"
#include "GreedySolver.h"

#include <chrono>
//#include "EvolutionSolver.h"
#include <iostream>
using namespace std;

int main() 
{
    RCPSPLoader loader;
if (!loader.wczytajZPliku("j601_1.sm")) {
    std::cerr << "Błąd wczytywania instancji.\n";
    return 1;
}
loader.wypisz();

   

    auto startwsio = std::chrono::high_resolution_clock::now();
    
    int liczbaUruchomien = 1;

   
    // === RANDOM SOLVER===
    auto startRand = std::chrono::high_resolution_clock::now();

    int randIteracji = 1000;
    int najlepszyRun = -1;
    int najlepszyKoszt = std::numeric_limits<int>::max();

    for (int run = 0; run < liczbaUruchomien; ++run)
    {
        RandomSolver solver(randIteracji);
        solver.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
        solver.zapiszWykorzystanieZasobow("zasoby_random.csv", loader.getLiczbaZasobow());


        // Zapisz statystyki z tego runa do pliku CSV
        solver.zapiszStatystykiDoCSV("wyniki_random.csv", run);

        // Jeśli koszt lepszy niż dotychczas – zapamiętaj numer i wartość
        if (solver.getMakespan() < najlepszyKoszt)
        {
            najlepszyKoszt = solver.getMakespan();
            najlepszyRun = run;
            solver.zapiszDoCSV("harmonogram_random.csv"); // tylko najlepszy
            
        }
    }
    std::cout << "Najlepszy RANDOM run: #" << najlepszyRun << "\n";
    std::cout << "Koszt (makespan): " << najlepszyKoszt << "\n";

    auto stopRand = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> elapsedRandom = stopRand - startRand;
   std::cout << "[RandomSolver] Czas wykonania: " << elapsedRandom.count() << " sekund\n";



   // === GREEDY SOLVER ===
auto startGreedy = std::chrono::high_resolution_clock::now();

GreedySolver greedy;
greedy.solve(loader.zadania, loader.liczbaZadan, loader.liczbaZasobow, loader.zasobyPojemnosc);
greedy.printSchedule();
greedy.zapiszDoCSV("harmonogram_greedy.csv");
greedy.zapiszWykorzystanieZasobow("zasoby_greedy.csv", loader.liczbaZasobow);

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
double endTemp = .01;                
double coolingRate = 0.9995;          
int maxIter = 1000000;                
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


/*
  //===EVOLUTIONERY ALGORYTHIM===
    auto startEA = std::chrono::high_resolution_clock::now();

   EvolutionSolver solverAE(100, 1000, 0.01, 0.7, 3); // populacja, pokolenia, mutacja, krzyżowanie, turniej
    solverAE.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
   // solverAE.printSchedule();
    //solverAE.zapiszDoCSV("harmonogram_evolution.csv");
   
   auto stopEA = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedEA = stopEA - startEA;
    std::cout << "[Algorytm EA] Czas wykonania: " << elapsedEA.count() << " sekund\n";

*/


auto stopwsio = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedwsio = stopwsio - startwsio;
    std::cout << "[Caly Algorytm] Czas wykonania: " << elapsedwsio.count() << " sekund\n";
  
    return 0;
}
