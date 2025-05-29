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
    int populationSize;
    int generations;
    double crossoverRate;
    double mutationRate;
    int tournamentSize;

    int bestMakespan;
    std::vector<Activity> bestSchedule;
  int makespan;
    std::vector<Activity> schedule;
     std::vector<int> kosztyPokolen;
    using Chromosome = std::vector<int>;

    int evaluate(const Chromosome& chromosome, const std::vector<Activity>& tasks, int numResources, const std::vector<int>& resourceCapacities, std::vector<Activity>& outSchedule) const;
std::pair<Chromosome, Chromosome> crossoverOX(const Chromosome& parent1, const Chromosome& parent2);
    void mutateSwap(Chromosome& chromosome);
    Chromosome tournamentSelection(const std::vector<Chromosome>& population, const std::vector<int>& fitness);
    bool isFeasible(const std::vector<Activity>& schedule);
};