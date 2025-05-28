#include "GreedySolver.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

void GreedySolver::solve(const std::vector<Activity>& zadaniaWejscie,
                         int liczbaZadan,
                         int liczbaZasobow,
                         const std::vector<int>& pojemnosci)
{
    std::vector<Activity> zadania = zadaniaWejscie;
    std::vector<Activity> harmonogram;
    std::vector<bool> zaplanowane(liczbaZadan, false);
    int zaplanowaneLicznik = 0;

    int maxCzas = 0;
    for (const auto& z : zadania) maxCzas += z.duration;
    maxCzas *= 2;

    std::vector<std::vector<int>> zuzycie(maxCzas, std::vector<int>(liczbaZasobow, 0));

    while (zaplanowaneLicznik < liczbaZadan)
    {
        std::vector<int> eligible;
        for (int i = 0; i < liczbaZadan; ++i)
        {
            if (zaplanowane[i]) continue;

            bool ok = true;
            for (int p : zadania[i].predecessors)
                if (!zaplanowane[p]) { ok = false; break; }

            if (ok) eligible.push_back(i);
        }

        std::sort(eligible.begin(), eligible.end(), [&](int a, int b) {
            return zadania[a].duration < zadania[b].duration;
        });

        for (int i : eligible)
        {
            Activity& zad = zadania[i];
            int earliestStart = 0;

            for (int p : zad.predecessors)
                if (zadania[p].end_time > earliestStart)
                    earliestStart = zadania[p].end_time;

            for (int t = earliestStart; t < maxCzas; ++t)
            {
                bool zasobyOk = true;
                for (int dt = 0; dt < zad.duration; ++dt)
                {
                    for (int r = 0; r < liczbaZasobow; ++r)
                        if (zuzycie[t + dt][r] + zad.resourceRequirements[r] > pojemnosci[r])
                            zasobyOk = false;
                    if (!zasobyOk) break;
                }

                if (zasobyOk)
                {
                    for (int dt = 0; dt < zad.duration; ++dt)
                        for (int r = 0; r < liczbaZasobow; ++r)
                            zuzycie[t + dt][r] += zad.resourceRequirements[r];

                    zad.start_time = t;
                    zad.end_time = t + zad.duration;
                    harmonogram.push_back(zad);
                    zaplanowane[i] = true;
                    zaplanowaneLicznik++;
                    break;
                }
            }
        }
    }

    schedule = harmonogram;
    makespan = 0;
    for (const auto& z : harmonogram)
        if (z.end_time > makespan)
            makespan = z.end_time;
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

    out << "job_id,start_time,end_time,duration\n";
    for (const auto& z : schedule)
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << "\n";

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
