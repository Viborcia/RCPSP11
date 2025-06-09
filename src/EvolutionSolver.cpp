#include "EvolutionSolver.h"
#include "ScheduleBuilder.h"
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
    int bestSoFar = INT_MAX;

    for (int g = 0; g < generations; ++g)
    {
            std::cout << "Generacja: " << g << "\n";
        for (int i = 0; i < populationSize; ++i)
        {
            std::vector<Activity> tempSchedule;
            fitness[i] = evaluate(population[i], tasks, numResources, resourceCapacities, tempSchedule);

            if (fitness[i] < bestMakespan)
            {
                bestMakespan = fitness[i];
                bestSchedule = tempSchedule;
            }
        }
        // Tworzenie rankingu populacji na podstawie fitness
        std::vector<std::pair<Chromosome, int>> rankedPopulation;
        for (int i = 0; i < populationSize; ++i){
             rankedPopulation.emplace_back(population[i], fitness[i]);
        }
        std::sort(rankedPopulation.begin(), rankedPopulation.end(),
          [](const auto& a, const auto& b) {
              return a.second < b.second;  // porównuj fitness
          });
          int eliteCount = 10;  // elitarne osobniki
        std::vector<Chromosome> eliteIndividuals;
        for (int i = 0; i < eliteCount; ++i){
         eliteIndividuals.push_back(rankedPopulation[i].first);
        }

        int bestInGeneration = *std::min_element(fitness.begin(), fitness.end());
        kosztyPokolen.push_back(bestInGeneration);

        if (g == 0)
            bestSoFar = bestInGeneration;
        else
            bestSoFar = std::min(bestSoFar, bestInGeneration);

        int avgFit = std::accumulate(fitness.begin(), fitness.end(), 0.0) / fitness.size();
        int worstFit = *std::max_element(fitness.begin(), fitness.end());
         std::cout << "  Best: " << bestInGeneration << ", Avg: " << avgFit << ", Worst: " << worstFit << "\n";
        std::ofstream out("statystyki_generacji.csv", std::ios::app);
        if (out.is_open()) {
            if (g == 0)
                out << "generacja;best;average;worst\n";
            out << g << ";" << bestSoFar << ";" << avgFit << ";" << worstFit << "\n";
            out.close();
        }

        std::vector<Chromosome> newPopulation;
        
        // Elitaryzm — przeniesienie najlepszego osobnika bez zmian
       //auto minIt = std::min_element(fitness.begin(), fitness.end());
       // int bestIndex = std::distance(fitness.begin(), minIt);
       // newPopulation.push_back(population[bestIndex]);
        //ELITARYZM Powyżej 1 osobnika
        newPopulation.insert(newPopulation.end(), eliteIndividuals.begin(), eliteIndividuals.end());


        while ((int)newPopulation.size() < populationSize)
        {
            Chromosome parent1 = tournamentSelection(population, fitness);
            Chromosome parent2 = tournamentSelection(population, fitness);
            // DEBUG: Wyświetlenie rodziców
            std::vector<Activity> tmpSched1, tmpSched2, tmpChild1, tmpChild2;
            int fitParent1 = evaluate(parent1, tasks, numResources, resourceCapacities, tmpSched1);
            int fitParent2 = evaluate(parent2, tasks, numResources, resourceCapacities, tmpSched2);
             std::cout <<fitParent1<< "   Rodzic 1: ";
        for (int gene : parent1) std::cout << gene << " ";
        std::cout << fitParent2<<"\n    Rodzic 2: ";
        for (int gene : parent2) std::cout << gene << " ";
        std::cout << "\n";

            Chromosome child1 = parent1;
            Chromosome child2 = parent2;

            

            if (std::generate_canonical<double, 10>(gen) < crossoverRate)
            {
                auto children = crossoverOX(parent1, parent2);
                child1 = children.first;
                child2 = children.second;
                 std::cout <<fitParent1<< "  Krzyżowanie:\n    Rodzic 1: ";
        for (int gene : parent1) std::cout << gene << " ";
        std::cout <<fitParent2<< "\n    Rodzic 2: ";
        for (int gene : parent2) std::cout << gene << " ";
        std::cout << "\n";
        // Oblicz fitness dzieci
int fitChild1 = evaluate(child1, tasks, numResources, resourceCapacities, tmpChild1);
int fitChild2 = evaluate(child2, tasks, numResources, resourceCapacities, tmpChild2);
         std::cout <<fitChild1<< "  Po Krzyżowaniu:\n    dziecko 1: ";
        for (int gene : child1) std::cout << gene << " ";
        std::cout <<fitChild1<< "\n    dziecko 2: ";
        for (int gene : child2) std::cout << gene << " ";
        std::cout << "\n";

            }

            // Mutacja ZAWSZE, niezależnie od krzyżowania
            mutateAdvanced(child1);
            mutateAdvanced(child2);
            //mutateSwap(child1);
            //mutateSwap(child2);
               // Oblicz fitness dzieci po mutacji
int fitChild1 = evaluate(child1, tasks, numResources, resourceCapacities, tmpChild1);
int fitChild2 = evaluate(child2, tasks, numResources, resourceCapacities, tmpChild2);
             std::cout << "  Po Mutacji:\n    dziecko 1: ";
             std::cout<<fitChild1 << "    Dziecko 1 (po mut.): ";
        for (int gene : child1) std::cout << gene << " ";
        std::cout <<fitChild2<< "\n    Dziecko 2 (po mut.): ";
        for (int gene : child2) std::cout << gene << " ";
        std::cout << "\n";

            newPopulation.push_back(child1);
            if ((int)newPopulation.size() < populationSize)
                newPopulation.push_back(child2);
        }


        population = newPopulation;
    }

    schedule = bestSchedule;
    makespan = bestMakespan;

    zapiszWykorzystanieZasobow("resource_usage.csv", numResources);
}

void EvolutionSolver::mutateAdvanced(Chromosome& chromosome)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dist(0, (int)chromosome.size() - 1);

    if (chromosome.size() < 2) return;

    double choice = dis(gen);

    if (choice < 0.33) // SWAP
    {
        for (size_t i = 0; i < chromosome.size(); ++i) {
            if (dis(gen) < mutationRate) {
                int j = dist(gen);
                std::swap(chromosome[i], chromosome[j]);
            }
        }
    }
    else if (choice < 0.66) // INSERT
    {
        int i = dist(gen);
        int j = dist(gen);
        if (i != j) {
            int gene = chromosome[i];
            chromosome.erase(chromosome.begin() + i);
            chromosome.insert(chromosome.begin() + j, gene);
        }
    }
    else // INVERSION
    {
        int i = dist(gen);
        int j = dist(gen);
        if (i > j) std::swap(i, j);
        std::reverse(chromosome.begin() + i, chromosome.begin() + j + 1);
    }
}

int EvolutionSolver::evaluate(
    const Chromosome& chromosome,
    const std::vector<Activity>& tasks,
    int numResources,
    const std::vector<int>& resourceCapacities,
    std::vector<Activity>& outSchedule) const
{
    // Ustalenie długości horyzontu czasowego
    int totalDuration = 0;
    for (const auto& z : tasks)
        totalDuration += z.duration;

    int maxTime = totalDuration + 50;

    // Bufor do zużycia zasobów i wyjściowy harmonogram
    std::vector<std::vector<int>> zuzycie(maxTime, std::vector<int>(numResources, 0));
    outSchedule.clear();

    // Wywołanie funkcji budującej harmonogram zgodnie z priorytetami
    ScheduleBuilder::zbudujZPriorytetami(
        chromosome,         // priorytety (czyli permutacja)
        tasks,              // dane wejściowe
        tasks.size(),       // liczba zadań
        numResources,       // liczba zasobów
        resourceCapacities, // pojemności zasobów
        zuzycie,            // tablica do zużycia zasobów
        outSchedule         // wynikowy harmonogram
    );

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
   // std::cout << "OX crossover:\n";
   // std::cout << "Parent1: ";
   // for (int gene : parent1) std::cout << gene << " ";
   // std::cout << "\nParent2: ";
   // for (int gene : parent2) std::cout << gene << " ";
   // std::cout << "\nStart: " << start << ", End: " << end << "\n";

    //std::cout << "Child1:  ";
   // for (int gene : child1) std::cout << gene << " ";
   // std::cout << "\nChild2:  ";
   // for (int gene : child2) std::cout << gene << " ";
  //  std::cout << "\n----------------------------------------\n";

    return {child1, child2};
}


void EvolutionSolver::mutateSwap(Chromosome& chromosome)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> geneDist(0, (int)chromosome.size() - 1);

    for (int i = 0; i < (int)chromosome.size(); ++i)
    {
        if (probDist(gen) < mutationRate)
        {
            int j = geneDist(gen);
            if (i != j)
            {
                std::swap(chromosome[i], chromosome[j]);
            }
        }
    }
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

     double sumKw = 0.0;
    for (int koszt : kosztyPokolen)
    {
        double roznica = koszt - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyPokolen.size());

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

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << ";" << "\n";
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