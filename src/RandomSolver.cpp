#include "RandomSolver.h"
#include "Activity.h"
#include <iostream>
#include <fstream>
#include <random>
#include <limits>

#include <algorithm>
#include <numeric> // std::accumulate
#include <fstream>


RandomSolver::RandomSolver(int liczbaProb)
    : liczbaProb(liczbaProb), makespan(0)
{}


void RandomSolver::solve(const std::vector<Activity>& zadaniaWejscie, int liczbaZadan, int liczbaZasobow, const std::vector<int>& pojemnosci)
{
    std::mt19937 gen(std::random_device{}());
    makespan = std::numeric_limits<int>::max();
    kosztyProb.clear();

    for (int prob = 0; prob < liczbaProb; ++prob)
    {
        std::vector<Activity> zadania = zadaniaWejscie;

        // Losowe priorytety
        std::vector<int> priorytety(liczbaZadan);
        for (int i = 0; i < liczbaZadan; ++i) priorytety[i] = i;
        std::shuffle(priorytety.begin(), priorytety.end(), gen);
        for (int i = 0; i < liczbaZadan; ++i) zadania[i].priority = priorytety[i];

        // Harmonogramowanie (Parallel SGS)
        std::vector<int> zasoby = pojemnosci;
        std::vector<bool> zaplanowane(liczbaZadan, false);
        int zaplanowaneLicznik = 0;
        int czas = 0;
        std::vector<Activity> harmonogram;

            while (zaplanowaneLicznik < liczbaZadan)
{
    // Zidentyfikuj gotowe do uruchomienia zadania (wszyscy poprzednicy zakończeni)
    std::vector<int> eligible;
    for (int i = 0; i < liczbaZadan; ++i)
    {
        if (zaplanowane[i]) continue;

        bool wszyscyPoprzednicyOK = true;
        for (int p : zadania[i].predecessors)
        {
            if (!zaplanowane[p] || zadania[p].end_time > czas) {
                wszyscyPoprzednicyOK = false;
                break;
            }
        }

        if (wszyscyPoprzednicyOK) eligible.push_back(i);
    }

    // Sortowanie według losowego priorytetu
    std::sort(eligible.begin(), eligible.end(), [&](int a, int b) {
        return zadania[a].priority < zadania[b].priority;
    });

    bool cosZaplanowano = false;

    for (int i : eligible)
    {
        // Sprawdź dostępność zasobów
        bool zasobyOK = true;
        for (int r = 0; r < liczbaZasobow; ++r)
        {
            if (zadania[i].resourceRequirements[r] > zasoby[r])
            {
                zasobyOK = false;
                break;
            }
        }

        if (zasobyOK)
        {
            // Zaplanuj zadanie od teraz
            zadania[i].start_time = czas;
            zadania[i].end_time = czas + zadania[i].duration;

            // Zablokuj zasoby na czas trwania
            for (int r = 0; r < liczbaZasobow; ++r)
                zasoby[r] -= zadania[i].resourceRequirements[r];

            harmonogram.push_back(zadania[i]);
            zaplanowane[i] = true;
            zaplanowaneLicznik++;
            cosZaplanowano = true;
        }
    }

    // Jeśli nic nie zaplanowano, zwiększ czas i przywróć zasoby z zakończonych zadań
    if (!cosZaplanowano)
    {
        czas++;
        for (int i = 0; i < liczbaZadan; ++i)
        {
            if (!zaplanowane[i]) continue;
            if (zadania[i].end_time == czas)
            {
                for (int r = 0; r < liczbaZasobow; ++r)
                    zasoby[r] += zadania[i].resourceRequirements[r];
            }
        }
    }
}


        // Licz maksymalny czas
        int lokalnyMakespan = 0;
        for (const auto& z : harmonogram)
            if (z.end_time > lokalnyMakespan) lokalnyMakespan = z.end_time;

        if (lokalnyMakespan < makespan)
        {
            makespan = lokalnyMakespan;
            schedule = harmonogram;
        }

        kosztyProb.push_back(lokalnyMakespan);
    }
}



void RandomSolver::printSchedule() const
{
    // Nagłówek – informacja o harmonogramie
    std::cout << "\n=== Najlepszy harmonogram (RandomSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";

    // Iterujemy po wszystkich operacjach w zapisanym harmonogramie
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& op = schedule[i]; // jawnie deklarujemy typ

        // Wypisujemy dane jednej operacji w formie tabeli
        std::cout << op.id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

void RandomSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);

    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek
    out << "job_id,start_time,end_time,duration,resources,predecessors,successors\n";

    // Iteracja po harmonogramie
    for (const Activity& task : schedule)
    {
        out << task.id << ","
            << task.start_time << ","
            << task.end_time << ","
            << task.duration << ",\"";

        // resources
        for (int i = 0; i < task.resourceRequirements.size(); ++i) {
            out << task.resourceRequirements[i];
            if (i < task.resourceRequirements.size() - 1)
                out << " ";
        }

        out << "\",\"";

        // predecessors
        for (int i = 0; i < task.predecessors.size(); ++i) {
            out << task.predecessors[i];
            if (i < task.predecessors.size() - 1)
                out << " ";
        }

        out << "\",\"";

        // successors
        for (int i = 0; i < task.successors.size(); ++i) {
            out << task.successors[i];
            if (i < task.successors.size() - 1)
                out << " ";
        }

        out << "\"\n";
    }

    out.close();
}


void RandomSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyProb.empty()) 
    {
        std::cerr << "Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = *std::min_element(kosztyProb.begin(), kosztyProb.end());
    double worst = *std::max_element(kosztyProb.begin(), kosztyProb.end());
    double avg = std::accumulate(kosztyProb.begin(), kosztyProb.end(), 0.0) / kosztyProb.size();
    
   // int sciezka = obliczDlugoscSciezkiKrytycznej(schedule);
   // double avgDevCPM = 100.0 * (makespan - sciezka) / (double)sciezka;
 double sumKw = 0.0;
    for (int koszt : kosztyProb)
    {
        double roznica = koszt - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyProb.size());

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app); // dopisujemy


    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }


    if (!istnieje) {
        out << "run;best;average;worst;critical_path;avgDevCPM\n";; // nagłówek tylko jeśli plik nie istniał
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << ";" << "\n";
    out.close();
}


void RandomSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Ustal maksymalny czas w harmonogramie
    int maksCzas = 0;
    for (const Activity& z : schedule)
        if (z.end_time > maksCzas) maksCzas = z.end_time;

    // Macierz czas × zasób
    std::vector<std::vector<int>> zuzycie(maksCzas + 1, std::vector<int>(liczbaZasobow, 0));

    // Wypełnianie macierzy
    for (const Activity& z : schedule)
    {
        for (int t = z.start_time; t < z.end_time; ++t)
        {
            for (int r = 0; r < liczbaZasobow; ++r)
            {
                zuzycie[t][r] += z.resourceRequirements[r];
            }
        }
    }

    // Zapis do pliku CSV
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek
    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    // Wiersze danych
    for (int t = 0; t <= maksCzas; ++t)
    {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r];
        out << "\n";
    }

    out.close();
}
