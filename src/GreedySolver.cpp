#include "GreedySolver.h"
#include <fstream>
#include <iostream>
#include <limits>
#include "ScheduleBuilder.h"

// ============================================================================
// GreedySolver::solveAuto(...) – szczegółowo zakomentowana wersja
// ============================================================================

void GreedySolver::solveAuto(const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    // Wektor do przechowania najlepszej permutacji (opcjonalny – nieużywany dalej)
    std::vector<int> najlepszaPermutacja;

    // Ustawienie początkowego najlepszego makespanu na maksymalną wartość
    int najlepszyMakespan = std::numeric_limits<int>::max();

    // Obliczenie sumy czasów trwania wszystkich zadań (przyda się do bufora)
    int sumaTrwania = 0;
    for (int i = 0; i < liczbaZadan; ++i)
        sumaTrwania += zadaniaWejscie[i].duration;

    // Horyzont czasowy – zakładamy trochę więcej niż suma trwania
    int maxCzas = sumaTrwania + 50;

    // Bufor zużycia zasobów [czas][zasób]
    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));

    // ------------------------------------------------------------------
    // SPT – Shortest Processing Time: najpierw zadania o najkrótszym czasie
    // ------------------------------------------------------------------
    std::vector<int> permSPT(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i) permSPT[i] = i; // inicjalizacja permutacji

    // Sortowanie rosnąco po czasie trwania
    for (int i = 0; i < liczbaZadan - 1; ++i)
        for (int j = i + 1; j < liczbaZadan; ++j)
            if (zadaniaWejscie[permSPT[i]].duration > zadaniaWejscie[permSPT[j]].duration)
            {
                int tmp = permSPT[i];
                permSPT[i] = permSPT[j];
                permSPT[j] = tmp;
            }

    // Budowa harmonogramu dla permutacji SPT
    std::vector<Activity> schedSPT;
    ScheduleBuilder::zbudujZPriorytetami(permSPT, zadaniaWejscie, liczbaZadan,
        liczbaZasobow, pojemnosci, buforZuzycia, schedSPT);

    // Obliczenie makespanu
    int makespanSPT = 0;
    for (int i = 0; i < schedSPT.size(); ++i)
        if (schedSPT[i].end_time > makespanSPT)
            makespanSPT = schedSPT[i].end_time;

    // Zapisz jako najlepszy jeśli lepszy niż poprzedni
    if (makespanSPT < najlepszyMakespan)
    {
        najlepszyMakespan = makespanSPT;
        schedule = schedSPT;
    }

    // ------------------------------------------------------------------
    // LPT – Longest Processing Time: najpierw zadania o najdłuższym czasie
    // ------------------------------------------------------------------
    std::vector<int> permLPT(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i) permLPT[i] = i;

    // Sortowanie malejąco po czasie trwania
    for (int i = 0; i < liczbaZadan - 1; ++i)
        for (int j = i + 1; j < liczbaZadan; ++j)
            if (zadaniaWejscie[permLPT[i]].duration < zadaniaWejscie[permLPT[j]].duration)
            {
                int tmp = permLPT[i];
                permLPT[i] = permLPT[j];
                permLPT[j] = tmp;
            }

    // Budowa harmonogramu dla permutacji LPT
    std::vector<Activity> schedLPT;
    ScheduleBuilder::zbudujZPriorytetami(permLPT, zadaniaWejscie, liczbaZadan,
        liczbaZasobow, pojemnosci, buforZuzycia, schedLPT);

    // Obliczenie makespanu
    int makespanLPT = 0;
    for (int i = 0; i < schedLPT.size(); ++i)
        if (schedLPT[i].end_time > makespanLPT)
            makespanLPT = schedLPT[i].end_time;

    // Zapisz jako najlepszy jeśli lepszy niż poprzedni
    if (makespanLPT < najlepszyMakespan)
    {
        najlepszyMakespan = makespanLPT;
        schedule = schedLPT;
    }

    // Ustaw ostateczny najlepszy makespan
    makespan = najlepszyMakespan;
} // koniec solveAuto()

// ============================================================================
// GreedySolver::printSchedule() – wypisanie harmonogramu na ekran
// ============================================================================

void GreedySolver::printSchedule() const
{
    // Nagłówek sekcji
    std::cout << "\n=== Harmonogram (GreedySolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "ID\tStart\tEnd\n";

    // Wypisanie informacji o każdym zadaniu
    for (int i = 0; i < schedule.size(); ++i)
    {
        std::cout << schedule[i].id << "\t"
            << schedule[i].start_time << "\t"
            << schedule[i].end_time << "\n";
    }
}


// ============================================================================
// GreedySolver::zapiszDoCSV() – zapis harmonogramu do pliku CSV
// ============================================================================

void GreedySolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Otwórz plik do zapisu
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) return;

    // Nagłówek kolumn CSV
    out << "job_id,start_time,end_time,duration,predecessors,R1,R2,R3,R4\n";

    // Zapisz dane każdego zadania do pliku
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << ",";

        // Zapisz poprzedników jako jedną kolumnę (oddzieloną spacją)
        for (int j = 0; j < z.predecessors.size(); ++j) {
            out << z.predecessors[j];
            if (j + 1 < z.predecessors.size()) out << " ";
        }

        // Zapisz zapotrzebowanie na zasoby w kolejnych kolumnach
        for (int j = 0; j < z.resourceRequirements.size(); ++j) {
            out << "," << z.resourceRequirements[j];
        }

        out << "\n";
    }

    out.close();
}


// ============================================================================
// GreedySolver::zapiszWykorzystanieZasobow() – CSV zużycia zasobów w czasie
// ============================================================================

void GreedySolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Znajdź maksymalny czas zakończenia zadań
    int maxCzas = 0;
    for (int i = 0; i < schedule.size(); ++i)
        if (schedule[i].end_time > maxCzas)
            maxCzas = schedule[i].end_time;

    // Przygotuj tablicę zużycia zasobów: [czas][zasób]
    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    // Wypełnij tablicę zużycia – dla każdej jednostki czasu w której zadanie trwa
    for (int i = 0; i < schedule.size(); ++i)
    {
        for (int t = schedule[i].start_time; t < schedule[i].end_time; ++t)
        {
            for (int r = 0; r < liczbaZasobow; ++r)
            {
                zuzycie[t][r] += schedule[i].resourceRequirements[r];
            }
        }
    }

    // Otwórz plik do zapisu
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) return;

    // Nagłówek pliku CSV
    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    // Wypisz wartości dla każdego momentu czasu
    for (int t = 0; t <= maxCzas; ++t)
    {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r];
        out << "\n";
    }

    out.close();
}
