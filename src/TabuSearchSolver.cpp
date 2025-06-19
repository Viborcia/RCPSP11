#include "TabuSearchSolver.h"
#include "GreedySolver.h"
#include "ScheduleBuilder.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <limits>
#include <deque>
#include <fstream>
#include <numeric>
#include <cmath>
#include <set>

TabuSearchSolver::TabuSearchSolver(int liczbaIteracji, int dlugoscTabu)
    : liczbaIteracji(liczbaIteracji), dlugoscTabu(dlugoscTabu), makespan(0)
{}

// ============================================================================
// TabuSearchSolver::solve(...) – szczegółowo zakomentowana wersja
// ============================================================================

void TabuSearchSolver::solve(const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    // Generator liczb losowych do zamian
    std::mt19937 gen(std::random_device{}());

    // Oblicz maksymalny możliwy czas harmonogramu
    int sumaTrwania = 0;
    for (int i = 0; i < zadaniaWejscie.size(); ++i)
        sumaTrwania += zadaniaWejscie[i].duration;
    int maxCzas = sumaTrwania + 50;

    // Bufor na zużycie zasobów oraz harmonogram startowy
    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));
    std::vector<Activity> harmonogramBufor;

    // Wygeneruj początkowe (losowe) rozwiązanie – permutację zadań
    std::vector<int> current(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i)
        current[i] = i;
    std::shuffle(current.begin(), current.end(), gen);

    // Zbuduj harmonogram dla rozwiązania początkowego
    ScheduleBuilder::zbudujZPriorytetami(current, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci, buforZuzycia, harmonogramBufor);

    // Oblicz jego koszt (makespan)
    int bestCost = obliczMakespan(harmonogramBufor);
    std::vector<int> bestSoFar = current; // zapisz jako najlepsze dotychczasowe

    // Lista tabu: przechowuje ostatnie ruchy (zamiany), by ich nie powtarzać
    std::deque<std::pair<int, int>> tabuQueue; // kolejka FIFO
    std::set<std::pair<int, int>> tabuSet;     // szybki dostęp do tabu

    // Czyść historię przed nowym uruchomieniem
    historiaBestSoFar.clear();
    historiaCurrent.clear();
    kosztyIteracji.clear();

    // Główna pętla algorytmu Tabu Search
    for (int iter = 0; iter < liczbaIteracji; ++iter)
    {
        std::vector<int> bestNeighbor = current;
        int bestNeighborCost = std::numeric_limits<int>::max();
        std::pair<int, int> bestMove = std::make_pair(-1, -1);

        // Wygeneruj 25 sąsiadów przez zamianę dwóch elementów w permutacji
        for (int k = 0; k < 25; ++k)
        {
            int i = gen() % liczbaZadan;
            int j = gen() % liczbaZadan;
            if (i == j) continue; // pomijaj puste ruchy

            std::vector<int> candidate = current;
            std::swap(candidate[i], candidate[j]);

            std::vector<std::vector<int>> buforTemp(maxCzas, std::vector<int>(liczbaZasobow, 0));
            std::vector<Activity> harmonogramTemp;
            ScheduleBuilder::zbudujZPriorytetami(candidate, zadaniaWejscie,
                liczbaZadan, liczbaZasobow, pojemnosci, buforTemp, harmonogramTemp);

            int cost = obliczMakespan(harmonogramTemp);

            // Zapisujemy ruch jako parę (mniejszy, większy), żeby unikać duplikatów
            std::pair<int, int> move = std::make_pair(std::min(i, j), std::max(i, j));
            bool jestTabu = tabuSet.count(move) > 0;

            // Ruch dozwolony jeśli:
            // - nie jest na liście tabu i poprawia koszt,
            // - lub jeśli poprawia globalne best (aspiration criterion)
            if ((cost < bestNeighborCost && !jestTabu) || cost < bestCost)
            {
                bestNeighbor = candidate;
                bestNeighborCost = cost;
                bestMove = move;
            }
        }

        // Zatwierdź wybranego sąsiada jako nowe current
        current = bestNeighbor;

        // Zapisz ruch do tabu, jeśli jakiś był
        if (bestMove.first != -1)
        {
            tabuQueue.push_back(bestMove);
            tabuSet.insert(bestMove);

            // Jeśli lista przekroczy długość, usuń najstarszy ruch
            if (tabuQueue.size() > (unsigned int)dlugoscTabu)
            {
                std::pair<int, int> removed = tabuQueue.front();
                tabuQueue.pop_front();
                tabuSet.erase(removed);
            }
        }

        // Jeśli sąsiad poprawia najlepsze rozwiązanie, zapisz go
        if (bestNeighborCost < bestCost)
        {
            bestCost = bestNeighborCost;
            bestSoFar = current;
        }

        // Zapisywanie statystyk do analizy konwergencji
        historiaBestSoFar.push_back(bestCost);
        historiaCurrent.push_back(bestNeighborCost);
        kosztyIteracji.push_back(bestNeighborCost);
    }

    // Zbuduj ostateczny harmonogram dla najlepszego znalezionego rozwiązania
    schedule.clear();
    std::vector<std::vector<int>> finalBufor(maxCzas, std::vector<int>(liczbaZasobow, 0));
    ScheduleBuilder::zbudujZPriorytetami(bestSoFar, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci, finalBufor, schedule);

    // Zapisz wynikowy makespan
    makespan = bestCost;
}


// ============================================================================
// TabuSearchSolver – pozostałe funkcje z komentarzami
// ============================================================================

// Oblicz maksymalny czas zakończenia – czyli makespan
int TabuSearchSolver::obliczMakespan(const std::vector<Activity>& harmonogram) const
{
    int maks = 0;
    for (int i = 0; i < harmonogram.size(); ++i)
    {
        if (harmonogram[i].end_time > maks)
            maks = harmonogram[i].end_time;
    }
    return maks;
}

// Wypisz harmonogram do konsoli
void TabuSearchSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (TabuSearchSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Zadania:\n";
    std::cout << "ID\tStart\tEnd\n";

    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
    }
}

// Zapisz harmonogram do pliku CSV
void TabuSearchSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek pliku CSV
    out << "job_id,start_time,end_time,duration,predecessors\n";

    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << ",";

        for (int j = 0; j < z.predecessors.size(); ++j)
        {
            out << z.predecessors[j];
            if (j < z.predecessors.size() - 1)
                out << ";";
        }

        out << "\n";
    }

    out.close();
}

// Zapisz statystyki z jednego runa do CSV (best/avg/worst/std)
void TabuSearchSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = kosztyIteracji[0];
    double worst = kosztyIteracji[0];
    double sum = 0.0;
    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        if (kosztyIteracji[i] < best) best = kosztyIteracji[i];
        if (kosztyIteracji[i] > worst) worst = kosztyIteracji[i];
        sum += kosztyIteracji[i];
    }
    double avg = sum / kosztyIteracji.size();

    double sumKw = 0.0;
    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        double roznica = kosztyIteracji[i] - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyIteracji.size());

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "run;best;average;worst;std\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}

// Zapisz porównanie best vs current z każdej iteracji
void TabuSearchSolver::zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const
{
    if (historiaCurrent.empty() || historiaBestSoFar.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania best vs current.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "iter;current;best_so_far\n";
    }

    for (int i = 0; i < historiaCurrent.size(); ++i)
    {
        out << i << ";" << historiaCurrent[i] << ";" << historiaBestSoFar[i] << "\n";
    }

    out.close();
}

// Zapisz best/avg/worst z każdej iteracji najlepszego runa
void TabuSearchSolver::zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const
{
    if (kosztyIteracji.empty() || avgIteracji.empty() || worstIteracji.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania kosztów najlepszego runa.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "iter;best;avg;worst\n";
    }

    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        out << i << ";" << kosztyIteracji[i] << ";" << avgIteracji[i] << ";" << worstIteracji[i] << "\n";
    }

    out.close();
}

// Zapisz zużycie zasobów do pliku CSV (czas x zasoby)
void TabuSearchSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    int maxCzas = 0;
    for (int i = 0; i < schedule.size(); ++i)
    {
        if (schedule[i].end_time > maxCzas)
            maxCzas = schedule[i].end_time;
    }

    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        for (int t = z.start_time; t < z.end_time; ++t)
        {
            for (int r = 0; r < liczbaZasobow; ++r)
            {
                zuzycie[t][r] += z.resourceRequirements[r];
            }
        }
    }

    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    for (int t = 0; t <= maxCzas; ++t)
    {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r];
        out << "\n";
    }

    out.close();
}

// Zapisz surowe dane o zadaniach do pliku CSV (np. do debugowania)
void zapiszZadaniaDoCSV(const std::vector<Activity>& zadania, const std::string& nazwaPliku)
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "job_id,duration,R1,R2,R3,R4,predecessors\n";

    for (int zIndex = 0; zIndex < zadania.size(); ++zIndex)
    {
        const Activity& z = zadania[zIndex];
        out << z.id << "," << z.duration;

        for (int rIndex = 0; rIndex < z.resourceRequirements.size(); ++rIndex)
        {
            out << "," << z.resourceRequirements[rIndex];
        }

        out << ",";

        for (int i = 0; i < z.predecessors.size(); ++i)
        {
            out << z.predecessors[i];
            if (i < z.predecessors.size() - 1)
                out << ";";
        }

        out << "\n";
    }

    out.close();
}
