#include "EvolutionSolver.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <iostream>
#include <fstream>
#include <map>

EvolutionSolver::EvolutionSolver(int populationSize, int generations, double crossoverRate, double mutationRate, int tournamentSize)
    : populationSize(populationSize), generations(generations), crossoverRate(crossoverRate), mutationRate(mutationRate), tournamentSize(tournamentSize), bestMakespan(INT_MAX)
{
}

void EvolutionSolver::solve(const std::vector<Activity>& tasks, int numTasks, int numResources, const std::vector<int>& resourceCapacities)
{
    std::mt19937 gen(std::random_device{}());

    std::vector<Chromosome> population;
    for (int i = 0; i < populationSize; ++i)
    {
        Chromosome c(numTasks);
        std::iota(c.begin(), c.end(), 0);
        std::shuffle(c.begin(), c.end(), gen);
        population.push_back(c);
    }

    std::vector<int> fitness(populationSize);

    for (int g = 0; g < generations; ++g)
    {
        std::cout << "Generacja: " << g << std::endl;

        for (int i = 0; i < populationSize; ++i)
        {
            std::vector<Activity> schedule;
            fitness[i] = evaluate(population[i], tasks, numResources, resourceCapacities, schedule);

            if (fitness[i] < bestMakespan)
            {
                bestMakespan = fitness[i];
                bestSchedule = schedule;
                std::cout << "  Nowy najlepszy makespan: " << bestMakespan << " w generacji " << g << std::endl;
            }
        }
    int bestInGeneration = *std::min_element(fitness.begin(), fitness.end());

        std::cout << "  Najlepszy fitness w tej generacji: " << *std::min_element(fitness.begin(), fitness.end()) << std::endl;
            kosztyPokolen.push_back(bestInGeneration);

        std::vector<Chromosome> newPopulation;

        while (newPopulation.size() < populationSize)
        {
            Chromosome parent1 = tournamentSelection(population, fitness);
            Chromosome parent2 = tournamentSelection(population, fitness);

            std::cout << "    Rodzic 1: ";
            for (int gene : parent1) std::cout << gene << " ";
            std::cout << "\n    Rodzic 2: ";
            for (int gene : parent2) std::cout << gene << " ";
            std::cout << std::endl;

            if (std::generate_canonical<double, 10>(gen) < crossoverRate)
            {
                auto [child1, child2] = crossoverOX(parent1, parent2);

                if (std::generate_canonical<double, 10>(gen) < mutationRate)
                    mutateSwap(child1);
                if (std::generate_canonical<double, 10>(gen) < mutationRate)
                    mutateSwap(child2);

                std::cout << "    Potomek 1: ";
                for (int gene : child1) std::cout << gene << " ";
                std::cout << "\n    Potomek 2: ";
                for (int gene : child2) std::cout << gene << " ";
                std::cout << "\n" << std::endl;

                newPopulation.push_back(child1);
                if (newPopulation.size() < populationSize)
                    newPopulation.push_back(child2); // dopiero jeśli jest miejsce
            }
            else
            {
                // Brak krzyżowania – kopiujemy rodzica
                newPopulation.push_back(parent1);
            }
        }
        // Na końcu pętli generacji
int bestFit = *std::min_element(fitness.begin(), fitness.end());
int avgFit = std::accumulate(fitness.begin(), fitness.end(), 0.0) / fitness.size();
int worstFit = *std::max_element(fitness.begin(), fitness.end());

std::ofstream out("statystyki_generacji.csv", std::ios::app);
if (out.is_open()) {
    if (g == 0)  // nagłówek tylko raz
        out << "generacja;best;average;worst\n";
    out << g << ";" << bestFit << ";" << avgFit << ";" << worstFit << "\n";
    out.close();
}


        population = newPopulation;

    }
    saveScheduleCSV("best_schedule.csv");
    zapiszDoCSV("detailed_schedule.csv");
    zapiszStatystykiDoCSV("statistics.csv", 1);  // np. 1 oznacza pierwsze uruchomienie
    zapiszWykorzystanieZasobow("resource_usage.csv", numResources);
}


int EvolutionSolver::evaluate(const Chromosome& chromosome, const std::vector<Activity>& tasksInput, int numResources, const std::vector<int>& resourceCapacities, std::vector<Activity>& outSchedule) const
{
    std::vector<Activity> tasks = tasksInput;
    for (auto& task : tasks) {
        task.start_time = -1;  // reset
        task.end_time = -1;    // reset
    }
    for (int i = 0; i < chromosome.size(); ++i)
        tasks[chromosome[i]].priority = i;

    std::vector<bool> scheduled(tasks.size(), false);
    std::vector<int> resources = resourceCapacities;
    outSchedule.clear();

    int time = 0;
    int count = 0;

    // Mapa: czas -> lista zadań kończących się w tym czasie
    std::map<int, std::vector<int>> tasksEndingAt;

    while (count < tasks.size())
    {
        // Zwolnij zasoby zadań kończących się o aktualnym czasie
        if (tasksEndingAt.count(time) > 0) {
            for (int tid : tasksEndingAt[time]) {
                for (int r = 0; r < numResources; ++r)
                    resources[r] += tasks[tid].resourceRequirements[r];
            }
            tasksEndingAt.erase(time);
        }

        // Wybierz zadania gotowe do harmonogramowania
        std::vector<int> eligible;
        for (int i = 0; i < tasks.size(); ++i)
        {
            if (scheduled[i]) continue;
            bool ready = true;
            for (int pred : tasks[i].predecessors)
                if (!scheduled[pred] || tasks[pred].end_time > time)
                    ready = false;
            if (ready)
                eligible.push_back(i);
        }

        // Posortuj po priorytecie
        std::sort(eligible.begin(), eligible.end(), [&](int a, int b) {
            return tasks[a].priority < tasks[b].priority;
        });

        bool progress = false;
        for (int i : eligible)
        {
            // Sprawdź dostępność zasobów
            bool canSchedule = true;
            for (int r = 0; r < numResources; ++r)
            {
                if (tasks[i].resourceRequirements[r] > resources[r])
                {
                    canSchedule = false;
                    break;
                }
            }

            if (canSchedule)
            {
                tasks[i].start_time = time;
                tasks[i].end_time = time + tasks[i].duration;
                for (int r = 0; r < numResources; ++r)
                    resources[r] -= tasks[i].resourceRequirements[r];

                scheduled[i] = true;
                count++;
                outSchedule.push_back(tasks[i]);

                // Zapamiętaj, kiedy zadanie się kończy, by zwolnić zasoby
                tasksEndingAt[tasks[i].end_time].push_back(i);

                progress = true;
            }
        }

        if (!progress)
            time++;
    }

    // Oblicz maksymalny czas zakończenia (makespan)
    int localMakespan = 0;
    for (const auto& task : outSchedule)
        localMakespan = std::max(localMakespan, task.end_time);

    return localMakespan;
}
std::pair<EvolutionSolver::Chromosome, EvolutionSolver::Chromosome>
EvolutionSolver::crossoverOX(const Chromosome& parent1, const Chromosome& parent2)
{
    std::mt19937 gen(std::random_device{}());
    int size = parent1.size();
    int start = gen() % size;
    int end = gen() % size;
    if (start > end) std::swap(start, end);

    Chromosome child1(size, -1);
    Chromosome child2(size, -1);

    // 1. Przepisz segment środkowy z odpowiedniego rodzica
    for (int i = start; i <= end; ++i) {
        child1[i] = parent1[i];
        child2[i] = parent2[i];
    }

    // 2. Uzupełnij child1 z parent2
    int cur1 = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int idx = (end + 1 + i) % size;
        if (std::find(child1.begin(), child1.end(), parent2[idx]) == child1.end()) {
            child1[cur1] = parent2[idx];
            cur1 = (cur1 + 1) % size;
        }
    }

    // 3. Uzupełnij child2 z parent1
    int cur2 = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int idx = (end + 1 + i) % size;
        if (std::find(child2.begin(), child2.end(), parent1[idx]) == child2.end()) {
            child2[cur2] = parent1[idx];
            cur2 = (cur2 + 1) % size;
        }
    }

    // 🔍 Debug output:
    std::cout << "OX crossover:\n";
    std::cout << "Parent1: ";
    for (int gene : parent1) std::cout << gene << " ";
    std::cout << "\nParent2: ";
    for (int gene : parent2) std::cout << gene << " ";
    std::cout << "\nStart: " << start << ", End: " << end << "\n";

    std::cout << "Child1:  ";
    for (int gene : child1) std::cout << gene << " ";
    std::cout << "\nChild2:  ";
    for (int gene : child2) std::cout << gene << " ";
    std::cout << "\n----------------------------------------\n";

    return {child1, child2};
}


void EvolutionSolver::mutateSwap(Chromosome& chromosome)
{
    std::mt19937 gen(std::random_device{}());
    int i = gen() % chromosome.size();
    int j = gen() % chromosome.size();
    std::swap(chromosome[i], chromosome[j]);
}

EvolutionSolver::Chromosome EvolutionSolver::tournamentSelection(const std::vector<Chromosome>& population, const std::vector<int>& fitness)
{
    std::mt19937 gen(std::random_device{}());
    Chromosome best;
    int bestFit = INT_MAX;
    for (int i = 0; i < tournamentSize; ++i)
    {
        int idx = gen() % population.size();
        if (fitness[idx] < bestFit)
        {
            bestFit = fitness[idx];
            best = population[idx];
        }
    }
    return best;
}

void EvolutionSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (EvolutionSolver) ===\n";
    std::cout << "Makespan: " << bestMakespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "ID\tPriorytet\tStart\tEnd\n";
    for (const Activity& task : bestSchedule)
    {
        std::cout << task.id << "\t" << task.priority << "\t\t" << task.start_time << "\t" << task.end_time << "\n";
    }
}

void EvolutionSolver::saveScheduleCSV(const std::string& filename) const
{
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << filename << "\n";
        return;
    }
    out << "job_id,start_time,end_time,duration,resources\n";
    for (const auto& task : bestSchedule)
    {
        out << task.id << "," << task.start_time << "," << task.end_time << "," << task.duration << ",\"";
        for (int i = 0; i < task.resourceRequirements.size(); ++i)
        {
            out << task.resourceRequirements[i];
            if (i < task.resourceRequirements.size() - 1)
                out << " ";
        }
        out << "\"\n";
    }
    out.close();
}
void EvolutionSolver::zapiszDoCSV(const std::string& nazwaPliku) const {
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "job_id,start_time,end_time,duration,resources,predecessors,successors\n";
    for (const Activity& task : bestSchedule) {  // <-- zmiana z schedule na bestSchedule
        out << task.id << ","
            << task.start_time << ","
            << task.end_time << ","
            << task.duration << ",\"";

        for (int i = 0; i < task.resourceRequirements.size(); ++i) {
            out << task.resourceRequirements[i];
            if (i < task.resourceRequirements.size() - 1)
                out << " ";
        }

        out << "\",\"";

        for (int i = 0; i < task.predecessors.size(); ++i) {
            out << task.predecessors[i];
            if (i < task.predecessors.size() - 1)
                out << " ";
        }

        out << "\",\"";

        for (int i = 0; i < task.successors.size(); ++i) {
            out << task.successors[i];
            if (i < task.successors.size() - 1)
                out << " ";
        }

        out << "\"\n";
    }

    out.close();
}


void EvolutionSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const {
    if (kosztyPokolen.empty()) {
        std::cerr << "Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = *std::min_element(kosztyPokolen.begin(), kosztyPokolen.end());
    double worst = *std::max_element(kosztyPokolen.begin(), kosztyPokolen.end());
    double avg = std::accumulate(kosztyPokolen.begin(), kosztyPokolen.end(), 0.0) / kosztyPokolen.size();

    int sciezka = obliczDlugoscSciezkiKrytycznej(bestSchedule);
double avgDevCPM = 100.0 * (bestMakespan - sciezka) / (double)sciezka;

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) {
        out << "run;best;average;worst;critical_path;avgDevCPM\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << sciezka << ";" << avgDevCPM << "\n";
    out.close();
}

void EvolutionSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const {
    int maksCzas = 0;
for (const Activity& z : bestSchedule)
    if (z.end_time > maksCzas) maksCzas = z.end_time;

std::vector<std::vector<int>> zuzycie(maksCzas + 1, std::vector<int>(liczbaZasobow, 0));

for (const Activity& z : bestSchedule) {
    for (int t = z.start_time; t < z.end_time; ++t) {
        for (int r = 0; r < liczbaZasobow; ++r) {
            zuzycie[t][r] += z.resourceRequirements[r];
        }
    }
}
    std::ofstream out(nazwaPliku);
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    for (int t = 0; t <= maksCzas; ++t) {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r];
        out << "\n";
    }

    out.close();
}