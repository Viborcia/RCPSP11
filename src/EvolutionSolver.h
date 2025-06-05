#ifndef EVOLUTION_SOLVER_H
#define EVOLUTION_SOLVER_H

#include "Activity.h"
#include <vector>
#include <string>

class EvolutionSolver
{
public:
    EvolutionSolver(int populationSize, int generations, double crossoverRate, double mutationRate, int tournamentSize);

    void solve(const std::vector<Activity>& tasks, int numTasks, int numResources, const std::vector<int>& resourceCapacities);
    void printSchedule() const;
    void saveScheduleCSV(const std::string& filename) const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;
    void zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const;

    int getMakespan() const { return makespan; }
    const std::vector<Activity>& getSchedule() const { return schedule; }

private:
    using Chromosome = std::vector<int>;

    int populationSize;
    int generations;
    double crossoverRate;
    double mutationRate;
    int tournamentSize;

    int bestMakespan = 0;
    int makespan = 0;
    std::vector<Activity> bestSchedule;
    std::vector<Activity> schedule;
    std::vector<int> kosztyPokolen;

    int evaluate(const Chromosome& chromosome,
                 const std::vector<Activity>& tasks,
                 int numResources,
                 const std::vector<int>& resourceCapacities,
                 std::vector<Activity>& outSchedule) const;

    std::pair<Chromosome, Chromosome> crossoverOX(const Chromosome& parent1, const Chromosome& parent2);
    void mutateSwap(Chromosome& chromosome); // (opcjonalna, jeśli nadal używana)
    void mutateAdvanced(Chromosome& chromosome); // nowa ulepszona mutacja
    Chromosome tournamentSelection(const std::vector<Chromosome>& population, const std::vector<int>& fitness);
};

#endif // EVOLUTION_SOLVER_H
