#include "GreedySolver.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>
#include <numeric>
#include <limits>
#include "ScheduleBuilder.h"

void GreedySolver::solve(const std::vector<Activity>& zadaniaWejscie,
                         int liczbaZadan,
                         int liczbaZasobow,
                         const std::vector<int>& pojemnosci)
{
    std::vector<int> permutacja(liczbaZadan);
    std::iota(permutacja.begin(), permutacja.end(), 0);

    std::sort(permutacja.begin(), permutacja.end(), [&](int a, int b) {
        return zadaniaWejscie[a].duration < zadaniaWejscie[b].duration;
    });

    int sumaTrwania = 0;
    for (const auto& z : zadaniaWejscie) sumaTrwania += z.duration;
    int maxCzas = sumaTrwania + 50;

    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));
    schedule.clear();
    ScheduleBuilder::zbudujZPriorytetami(permutacja,
                                         zadaniaWejscie,
                                         liczbaZadan,
                                         liczbaZasobow,
                                         pojemnosci,
                                         buforZuzycia,
                                         schedule);

    makespan = 0;
    for (const auto& z : schedule)
        if (z.end_time > makespan)
            makespan = z.end_time;
}

void GreedySolver::solveAuto(const std::vector<Activity>& zadaniaWejscie,
                             int liczbaZadan,
                             int liczbaZasobow,
                             const std::vector<int>& pojemnosci)
{
    std::vector<int> najlepszaPermutacja;
    int najlepszyMakespan = std::numeric_limits<int>::max();

    std::vector<std::function<bool(int, int)>> reguly = {
        [&](int a, int b) { return zadaniaWejscie[a].duration < zadaniaWejscie[b].duration; },
        [&](int a, int b) { return zadaniaWejscie[a].duration > zadaniaWejscie[b].duration; }
    };

    int sumaTrwania = 0;
    for (const auto& z : zadaniaWejscie) sumaTrwania += z.duration;
    int maxCzas = sumaTrwania + 50;
    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));

    for (const auto& regula : reguly)
    {
        std::vector<int> permutacja(liczbaZadan);
        std::iota(permutacja.begin(), permutacja.end(), 0);
        std::sort(permutacja.begin(), permutacja.end(), regula);

        std::vector<Activity> tempSchedule;
        ScheduleBuilder::zbudujZPriorytetami(permutacja,
                                             zadaniaWejscie,
                                             liczbaZadan,
                                             liczbaZasobow,
                                             pojemnosci,
                                             buforZuzycia,
                                             tempSchedule);

        int tempMakespan = 0;
        for (const auto& z : tempSchedule)
            if (z.end_time > tempMakespan)
                tempMakespan = z.end_time;

        if (tempMakespan < najlepszyMakespan)
        {
            najlepszyMakespan = tempMakespan;
            najlepszaPermutacja = permutacja;
            schedule = tempSchedule;
        }
    }

    makespan = najlepszyMakespan;
}

void GreedySolver::printSchedule() const
{
    std::cout << "\n=== Harmonogram (GreedySolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "ID\tStart\tEnd\n";
    for (const auto& z : schedule)
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
}

void GreedySolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) return;

    out << "job_id,start_time,end_time,duration,predecessors,R1,R2,R3,R4\n";

    for (const auto& z : schedule)
    {
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << ",";

        for (size_t i = 0; i < z.predecessors.size(); ++i) {
            out << z.predecessors[i];
            if (i + 1 < z.predecessors.size()) out << " ";
        }

        for (int zasob : z.resourceRequirements) {
            out << "," << zasob;
        }

        out << "\n";
    }

    out.close();
}

void GreedySolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    int maxCzas = 0;
    for (const auto& z : schedule)
        if (z.end_time > maxCzas)
            maxCzas = z.end_time;

    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    for (const auto& z : schedule)
        for (int t = z.start_time; t < z.end_time; ++t)
            for (int r = 0; r < liczbaZasobow; ++r)
                zuzycie[t][r] += z.resourceRequirements[r];

    std::ofstream out(nazwaPliku);
    if (!out.is_open()) return;

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
