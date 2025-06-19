#include "RandomSolver.h"
#include "Activity.h"
#include "ScheduleBuilder.h"
#include <iostream>
#include <fstream>
#include <random>
#include <limits>

#include <algorithm>
#include <numeric> // std::accumulate



RandomSolver::RandomSolver(int liczbaProb)
    : liczbaProb(liczbaProb), makespan(0)
{}

void RandomSolver::solve(const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    // Generator liczb losowych (Mersenne Twister) na podstawie ziarna z systemu
    std::mt19937 gen(std::random_device{}());

    // Inicjalna wartość makespan - ustawiamy na maksymalną możliwą wartość
    makespan = std::numeric_limits<int>::max();

    // Wyczyść wektor kosztów z poprzednich uruchomień
    kosztyProb.clear();

    // Zakładamy maksymalny horyzont czasowy harmonogramu
    int maxCzas = 10000;

    // Pętla po liczbie prób (iteracji losowych)
    for (int prob = 0; prob < liczbaProb; ++prob)
    {
        // 1. Stwórz permutację identyfikatorów zadań [0, 1, ..., n-1]
        std::vector<int> permutacja(liczbaZadan);
        for (int i = 0; i < liczbaZadan; ++i)
            permutacja[i] = i;

        // 2. Wymieszaj losowo permutację
        std::shuffle(permutacja.begin(), permutacja.end(), gen);

        // 3. Przygotuj bufor zużycia zasobów [czas][zasób]
        std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));

        // 4. Bufor na harmonogram uzyskany w tej iteracji
        std::vector<Activity> tempSchedule;

        // 5. Buduj harmonogram na podstawie wygenerowanej permutacji
        ScheduleBuilder::zbudujZPriorytetami(permutacja,
            zadaniaWejscie,
            liczbaZadan,
            liczbaZasobow,
            pojemnosci,
            buforZuzycia,
            tempSchedule);

        // 6. Oblicz makespan - maksymalny czas zakończenia jakiegokolwiek zadania
        int lokalnyMakespan = 0;
        for (int i = 0; i < tempSchedule.size(); ++i)
            if (tempSchedule[i].end_time > lokalnyMakespan)
                lokalnyMakespan = tempSchedule[i].end_time;

        // 7. Jeśli ten makespan jest lepszy od najlepszego dotychczasowego - zapisz go
        if (lokalnyMakespan < makespan)
        {
            makespan = lokalnyMakespan;         // nowy najlepszy wynik
            schedule = tempSchedule;           // zapisz harmonogram
        }

        // 8. Zapisz koszt tej próby do statystyk
        kosztyProb.push_back(lokalnyMakespan);
    }
} // koniec funkcji


// ============================================================================
// RandomSolver::printSchedule() – wypisanie harmonogramu na ekran
// ============================================================================

void RandomSolver::printSchedule() const
{
    // Nagłówek
    std::cout << "\n=== Najlepszy harmonogram (RandomSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "ID\tPriorytet\tStart\tEnd\n";

    // Wypisz każde zadanie w harmonogramie
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& op = schedule[i];
        std::cout << op.id << "\t" << op.priority << "\t\t" << op.start_time << "\t" << op.end_time << "\n";
    }
}


// ============================================================================
// RandomSolver::zapiszDoCSV() – zapis harmonogramu do pliku CSV
// ============================================================================

void RandomSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Otwórz plik do zapisu
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek CSV
    out << "job_id,start_time,end_time,duration,resources,predecessors,successors\n";

    // Wypisz dane każdego zadania
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& task = schedule[i];
        out << task.id << "," << task.start_time << "," << task.end_time << "," << task.duration << ",\"";

        for (int j = 0; j < task.resourceRequirements.size(); ++j) {
            out << task.resourceRequirements[j];
            if (j < task.resourceRequirements.size() - 1)
                out << " ";
        }

        out << "\",\"";

        for (int j = 0; j < task.predecessors.size(); ++j) {
            out << task.predecessors[j];
            if (j < task.predecessors.size() - 1)
                out << " ";
        }

        out << "\",\"";

        for (int j = 0; j < task.successors.size(); ++j) {
            out << task.successors[j];
            if (j < task.successors.size() - 1)
                out << " ";
        }

        out << "\"\n";
    }

    out.close();
}


// ============================================================================
// RandomSolver::zapiszStatystykiDoCSV() – zapis statystyk prób do pliku CSV
// ============================================================================

void RandomSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyProb.empty())
    {
        std::cerr << "Brak danych do zapisania statystyk.\n";
        return;
    }

    // Znajdź best, worst, avg, std
    double best = kosztyProb[0];
    double worst = kosztyProb[0];
    double sum = 0.0;
    for (int i = 0; i < kosztyProb.size(); ++i)
    {
        if (kosztyProb[i] < best) best = kosztyProb[i];
        if (kosztyProb[i] > worst) worst = kosztyProb[i];
        sum += kosztyProb[i];
    }

    double avg = sum / kosztyProb.size();

    double sumKw = 0.0;
    for (int i = 0; i < kosztyProb.size(); ++i)
    {
        double roznica = kosztyProb[i] - avg;
        sumKw += roznica * roznica;
    }

    double stddev = std::sqrt(sumKw / kosztyProb.size());

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) {
        out << "run;best;average;worst;std\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}


// ============================================================================
// RandomSolver::zapiszWykorzystanieZasobow() – CSV zużycia zasobów w czasie
// ============================================================================

void RandomSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Oblicz maksymalny czas końca
    int maksCzas = 0;
    for (int i = 0; i < schedule.size(); ++i)
        if (schedule[i].end_time > maksCzas)
            maksCzas = schedule[i].end_time;

    // Przygotuj tablicę zużycia zasobów w czasie
    std::vector<std::vector<int>> zuzycie(maksCzas + 1, std::vector<int>(liczbaZasobow, 0));

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

    // Zapisz do pliku CSV
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    for (int t = 0; t <= maksCzas; ++t)
    {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r];
        out << "\n";
    }

    out.close();
}
