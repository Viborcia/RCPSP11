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

void TabuSearchSolver::solve(const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    std::mt19937 gen(std::random_device{}());

    int sumaTrwania = 0;
    for (const auto& z : zadaniaWejscie) sumaTrwania += z.duration;
    int maxCzas = sumaTrwania + 50;

    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));
    std::vector<Activity> harmonogramBufor;

    std::vector<int> current(liczbaZadan);
    std::iota(current.begin(), current.end(), 0);
    std::shuffle(current.begin(), current.end(), gen);

    ScheduleBuilder::zbudujZPriorytetami(current, zadaniaWejscie,
        liczbaZadan, liczbaZasobow,
        pojemnosci, buforZuzycia, harmonogramBufor);
    int bestCost = obliczMakespan(harmonogramBufor);
    std::vector<int> bestSoFar = current;

    std::deque<std::pair<int, int>> tabuQueue;
    std::set<std::pair<int, int>> tabuSet;
    int bezPoprawy = 0;

    for (int iter = 0; iter < liczbaIteracji; ++iter)
    {
        std::vector<int> bestNeighbor = current;
        int bestNeighborCost = std::numeric_limits<int>::max();
        std::pair<int, int> bestMove = { -1, -1 };

        for (int k = 0; k < 25; ++k)
        {
            int i = gen() % liczbaZadan;
            int j = gen() % liczbaZadan;
            if (i == j) continue;

            std::vector<int> candidate = current;
            std::swap(candidate[i], candidate[j]);

            int koszt = INT_MAX;
            try {
                std::vector<Activity> tempBufor;
                ScheduleBuilder::zbudujZPriorytetami(candidate, zadaniaWejscie,
                    liczbaZadan, liczbaZasobow,
                    pojemnosci, buforZuzycia, tempBufor);
                if (!tempBufor.empty())
                    koszt = obliczMakespan(tempBufor);
            }
            catch (...) {
                continue;
            }

            if (koszt >= 1000000000) continue;

            std::pair<int, int> ruch = { std::min(i, j), std::max(i, j) };
            if (tabuSet.count(ruch) && koszt >= bestCost) continue;

            if (koszt < bestNeighborCost) {
                bestNeighbor = candidate;
                bestNeighborCost = koszt;
                bestMove = ruch;
            }
        }

        if (bestNeighborCost < bestCost) {
            bestSoFar = bestNeighbor;
            bestCost = bestNeighborCost;
            bezPoprawy = 0;
        }
        else 
        {
            bezPoprawy++;
        }

        current = bestNeighbor;

        if (bestMove.first != -1) {
            tabuQueue.push_back(bestMove);
            tabuSet.insert(bestMove);
            if ((int)tabuQueue.size() > dlugoscTabu) {
                tabuSet.erase(tabuQueue.front());
                tabuQueue.pop_front();
            }
        }

        if (bezPoprawy >= 300) {
            std::iota(current.begin(), current.end(), 0);
            std::shuffle(current.begin(), current.end(), gen);

            ScheduleBuilder::zbudujZPriorytetami(current, zadaniaWejscie,
                liczbaZadan, liczbaZasobow,
                pojemnosci, buforZuzycia, harmonogramBufor);
            int kosztPoRestarcie = obliczMakespan(harmonogramBufor);
            kosztyIteracji.push_back(kosztPoRestarcie);
            historiaCurrent.push_back(kosztPoRestarcie);
            historiaBestSoFar.push_back(bestCost);

            bezPoprawy = 0;
            tabuQueue.clear();
            tabuSet.clear();

            std::cout << "[Restart 300] Losowa permutacja\n";
        }
        else {
            kosztyIteracji.push_back(bestNeighborCost);
            historiaCurrent.push_back(bestNeighborCost);
            historiaBestSoFar.push_back(bestCost);
        }

        if (iter % 100 == 0)
            std::cout << "[Iteracja " << iter << "] makespan = " << bestNeighborCost
            << " (best = " << bestCost << ")\n";
    }

    ScheduleBuilder::zbudujZPriorytetami(bestSoFar, zadaniaWejscie,
        liczbaZadan, liczbaZasobow,
        pojemnosci, buforZuzycia, schedule);
    makespan = bestCost;
}

int TabuSearchSolver::obliczMakespan(const std::vector<Activity>& harmonogram) const
{
    int maks = 0;
    for (const auto& z : harmonogram)
        if (z.end_time > maks)
            maks = z.end_time;
    return maks;
}

void TabuSearchSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (TabuSearchSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Zadania:\n";
    std::cout << "ID\tStart\tEnd\n";

    for (const auto& z : schedule)
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
}

void TabuSearchSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówki CSV
    out << "job_id,start_time,end_time,duration,predecessors\n";

    for (const auto& z : schedule)
    {
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << ",";

        for (size_t i = 0; i < z.predecessors.size(); ++i)
        {
            out << z.predecessors[i];
            if (i < z.predecessors.size() - 1)
                out << ";";
        }

        out << "\n";
    }

    out.close();
}


void TabuSearchSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = *std::min_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double worst = *std::max_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double avg = std::accumulate(kosztyIteracji.begin(), kosztyIteracji.end(), 0.0) / kosztyIteracji.size();

    double sumKw = 0.0;
    for (int koszt : kosztyIteracji)
    {
        double roznica = koszt - avg;
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

void TabuSearchSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    int maxCzas = 0;
    for (const auto& z : schedule)
        if (z.end_time > maxCzas)
            maxCzas = z.end_time;

    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    for (const auto& z : schedule)
    {
        for (int t = z.start_time; t < z.end_time; ++t)
        {
            for (int r = 0; r < liczbaZasobow; ++r)
                zuzycie[t][r] += z.resourceRequirements[r];
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


#include <fstream>

void zapiszZadaniaDoCSV(const std::vector<Activity>& zadania, const std::string& nazwaPliku)
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "job_id,duration,R1,R2,R3,R4,predecessors\n";

    for (const auto& z : zadania)
    {
        out << z.id << "," << z.duration;

        for (int r : z.resourceRequirements)
            out << "," << r;

        out << ",";

        for (size_t i = 0; i < z.predecessors.size(); ++i)
        {
            out << z.predecessors[i];
            if (i < z.predecessors.size() - 1)
                out << ";";
        }

        out << "\n";
    }

    out.close();
}