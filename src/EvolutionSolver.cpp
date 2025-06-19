// EvolutionSolver.cpp – z maksymalnie szczegółowymi komentarzami edukacyjnymi

#include "EvolutionSolver.h"
#include "ScheduleBuilder.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <iostream>
#include <fstream>
#include <map>

// ============================================================================
// Konstruktor klasy EvolutionSolver
// Ustawia parametry algorytmu ewolucyjnego: wielkość populacji, liczbę pokoleń,
// współczynniki krzyżowania i mutacji, rozmiar turnieju oraz prawdopodobieństwo mutacji osobnika
// ============================================================================
EvolutionSolver::EvolutionSolver(int populationSize, int generations, double crossoverRate, double mutationRate, int tournamentSize, double individualMutationRate)
    : populationSize(populationSize),           // liczba osobników w każdej populacji
    generations(generations),                 // liczba generacji do wykonania
    crossoverRate(crossoverRate),             // prawdopodobieństwo krzyżowania (OX)
    mutationRate(mutationRate),               // NIE używane w tej wersji (można pominąć)
    tournamentSize(tournamentSize),           // liczba uczestników w selekcji turniejowej
    individualMutationRate(individualMutationRate), // prawdopodobieństwo mutacji każdego dziecka
    bestMakespan(INT_MAX)                     // domyślnie: najlepszy wynik bardzo wysoki
{
}

// ============================================================================
// Główna funkcja rozwiązywania RCPSP za pomocą algorytmu ewolucyjnego
// tasks – lista zadań wejściowych, numTasks – liczba zadań, numResources – liczba zasobów
// resourceCapacities – dostępne pojemności każdego zasobu
// ============================================================================
void EvolutionSolver::solve(const std::vector<Activity>& tasks, int numTasks, int numResources, const std::vector<int>& resourceCapacities)
{
    std::mt19937 gen(std::random_device{}()); // generator liczb losowych oparty o entropię systemową

    // === 1. Inicjalizacja populacji ===
    std::vector<Chromosome> population; // każdy chromosom to permutacja zadań

    for (int i = 0; i < populationSize; ++i) {
        Chromosome c(numTasks); // wektor o długości = liczba zadań

        // Wypełnij permutację wartościami 0, 1, ..., numTasks-1
        for (int j = 0; j < numTasks; ++j)
            c[j] = j;

        std::shuffle(c.begin(), c.end(), gen); // permutuj losowo kolejność zadań
        population.push_back(c); // dodaj chromosom do populacji
    }

    std::vector<int> fitness(populationSize); // wektor wartości dopasowania (koszt – makespan) każdego osobnika
    int bestSoFar = INT_MAX; // zapamiętujemy najlepszy wynik globalny

    // === 2. Główna pętla algorytmu – dla każdej generacji ===
    for (int g = 0; g < generations; ++g) {

        // 2a. Ewaluacja osobników
        for (int i = 0; i < populationSize; ++i) {
            std::vector<Activity> tempSchedule;

            // Oblicz koszt osobnika (makespan) przez wygenerowanie harmonogramu
            fitness[i] = evaluate(population[i], tasks, numResources, resourceCapacities, tempSchedule);

            // Zaktualizuj najlepsze rozwiązanie globalne, jeśli znaleziono lepsze
            if (fitness[i] < bestMakespan) {
                bestMakespan = fitness[i];
                bestSchedule = tempSchedule;
            }
        }

        // 2b. Posortuj populację według wartości fitness (rosnąco)
        std::vector<std::pair<Chromosome, int>> rankedPopulation;
        for (int i = 0; i < populationSize; ++i)
            rankedPopulation.push_back(std::make_pair(population[i], fitness[i]));

        // Bąbelkowe sortowanie populacji wg wartości fitness
        for (int i = 0; i < (int)rankedPopulation.size() - 1; ++i)
            for (int j = i + 1; j < (int)rankedPopulation.size(); ++j)
                if (rankedPopulation[i].second > rankedPopulation[j].second)
                    std::swap(rankedPopulation[i], rankedPopulation[j]);

        // 2c. Elitaryzm – zachowujemy najlepszych osobników bez zmian
        int eliteCount = 2; // liczba elity
        std::vector<Chromosome> eliteIndividuals;
        for (int i = 0; i < eliteCount; ++i)
            eliteIndividuals.push_back(rankedPopulation[i].first);

        // 2d. Statystyka: najlepszy wynik z tej generacji
        int bestInGeneration = fitness[0];
        for (int i = 1; i < fitness.size(); ++i)
            if (fitness[i] < bestInGeneration)
                bestInGeneration = fitness[i];
        kosztyPokolen.push_back(bestInGeneration);

        // 2e. Logowanie statystyk generacji do pliku CSV
        int sum = 0;
        int maxFit = fitness[0];
        for (int i = 0; i < fitness.size(); ++i) {
            sum += fitness[i];
            if (fitness[i] > maxFit) maxFit = fitness[i];
        }
        int avgFit = sum / fitness.size();
        int worstFit = maxFit;

        std::ofstream out("statystyki_generacji.csv", std::ios::app);
        if (out.is_open()) {
            if (g == 0) out << "generacja;best;average;worst\n";
            out << g << ";" << bestSoFar << ";" << avgFit << ";" << worstFit << "\n";
            out.close();
        }

        // 2f. Tworzenie nowej populacji
        std::vector<Chromosome> newPopulation;

        // Dodaj elitę do nowej populacji
        for (int i = 0; i < eliteCount; ++i)
            newPopulation.push_back(eliteIndividuals[i]);

        // Twórz nowych osobników przez krzyżowanie i mutację
        while ((int)newPopulation.size() < populationSize) {
            Chromosome parent1 = tournamentSelection(population, fitness);
            Chromosome parent2 = tournamentSelection(population, fitness);

            Chromosome child1 = parent1;
            Chromosome child2 = parent2;

            // Krzyżowanie OX z określonym prawdopodobieństwem
            double los = std::generate_canonical<double, 10>(gen);
            if (los < crossoverRate) {
                std::pair<Chromosome, Chromosome> dzieci = crossoverOX(parent1, parent2);
                child1 = dzieci.first;
                child2 = dzieci.second;
            }

            // Mutacja zaawansowana – wykonuj z określonym prawdopodobieństwem
            double mut1 = std::generate_canonical<double, 10>(gen);
            if (mut1 < individualMutationRate)
                mutateAdvanced(child1);

            double mut2 = std::generate_canonical<double, 10>(gen);
            if (mut2 < individualMutationRate)
                mutateAdvanced(child2);

            newPopulation.push_back(child1);
            if ((int)newPopulation.size() < populationSize)
                newPopulation.push_back(child2);
        }

        // Zastąp starą populację nową
        population = newPopulation;
    }

    // === 3. Po zakończeniu pokoleń — zapisz najlepszy znaleziony harmonogram ===
    schedule = bestSchedule;
    makespan = bestMakespan;

    // Zapisz zużycie zasobów do pliku CSV
    zapiszWykorzystanieZasobow("resource_usage.csv", numResources);
}



// ============================================================================
// Funkcja evaluate – bardzo szczegółowy komentarz krok po kroku
// Oblicza koszt rozwiązania (makespan) dla danej permutacji zadań (chromosomu)
// ============================================================================
int EvolutionSolver::evaluate(const Chromosome& chromosome, const std::vector<Activity>& tasks, int numResources, const std::vector<int>& resourceCapacities, std::vector<Activity>& outSchedule) const
{
    // Krok 1: Obliczamy łączny czas trwania wszystkich zadań,
    // co pozwala ustalić maksymalną długość bufora czasowego.
    int totalDuration = 0;
    for (int i = 0; i < (int)tasks.size(); ++i)
        totalDuration += tasks[i].duration;

    // Krok 2: Dodajemy zapas 50 jednostek czasu, by bufor był wystarczająco długi
    int maxTime = totalDuration + 50;

    // Krok 3: Tworzymy bufor zużycia zasobów:
    // macierz [czas][zasób], gdzie wpisujemy ile jednostek każdego zasobu jest zajętych w danym momencie
    std::vector<std::vector<int>> zuzycie(maxTime, std::vector<int>(numResources, 0));

    // Krok 4: Czyścimy harmonogram wyjściowy
    outSchedule.clear();

    // Krok 5: Budujemy harmonogram dla danego chromosomu (permutacji zadań)
    // Funkcja ustawi czasy startu i końca dla każdego zadania, uwzględniając zasoby i zależności
    ScheduleBuilder::zbudujZPriorytetami(chromosome, tasks, tasks.size(), numResources, resourceCapacities, zuzycie, outSchedule);

    // Krok 6: Liczymy makespan – maksymalny czas zakończenia w harmonogramie
    int localMakespan = 0;
    for (int i = 0; i < (int)outSchedule.size(); ++i)
    {
        if (outSchedule[i].end_time > localMakespan)
            localMakespan = outSchedule[i].end_time;
    }

    // Zwracamy makespan jako miarę jakości tego rozwiązania
    return localMakespan;
}


// ============================================================================
// Funkcja crossoverOX – Order Crossover (OX)
// Realizuje krzyżowanie dwóch rodziców, tworząc dwoje dzieci, zachowując porządek genów
// ============================================================================
std::pair<EvolutionSolver::Chromosome, EvolutionSolver::Chromosome> EvolutionSolver::crossoverOX(const Chromosome& parent1, const Chromosome& parent2)
{
    std::mt19937 gen(std::random_device{}()); // generator liczb losowych
    int size = parent1.size();                // długość chromosomu (liczba zadań)

    // Krok 1: Wylosuj dwa punkty cięcia (początek i koniec fragmentu do skopiowania)
    int start = gen() % size;
    int end = gen() % size;
    if (start > end) {
        int temp = start;
        start = end;
        end = temp; // zamiana jeśli kolejność losowa
    }

    // Krok 2: Utwórz dzieci i zainicjalizuj pustymi genami (-1 oznacza brak zadania)
    Chromosome child1(size, -1);
    Chromosome child2(size, -1);

    // Krok 3: Skopiuj fragment (od start do end) bezpośrednio od rodzica
    for (int i = start; i <= end; ++i) {
        child1[i] = parent1[i];
        child2[i] = parent2[i];
    }

    // Krok 4: Uzupełnianie reszty genów dla child1 z parent2, zachowując porządek
    int cur1 = (end + 1) % size; // indeks, gdzie zaczynamy wstawiać brakujące geny
    for (int i = 0; i < size; ++i) {
        int idx = (end + 1 + i) % size; // indeks po parent2
        bool found = false;
        for (int j = 0; j < size; ++j) {
            if (child1[j] == parent2[idx]) {
                found = true; // gen już istnieje w dziecku, pomijamy
                break;
            }
        }
        if (!found) {
            child1[cur1] = parent2[idx]; // dodaj brakujący gen do dziecka
            cur1 = (cur1 + 1) % size;    // przejdź do następnej pozycji
        }
    }

    // Krok 5: Analogiczne uzupełnienie dla child2 z parent1
    int cur2 = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int idx = (end + 1 + i) % size;
        bool found = false;
        for (int j = 0; j < size; ++j) {
            if (child2[j] == parent1[idx]) {
                found = true;
                break;
            }
        }
        if (!found) {
            child2[cur2] = parent1[idx];
            cur2 = (cur2 + 1) % size;
        }
    }

    // Zwróć parę dzieci jako wynik krzyżowania
    return std::make_pair(child1, child2);
}

// ============================================================================
// Funkcja mutateAdvanced – zaawansowana mutacja osobnika (permutacji zadań)
// Losowo wybiera jedną z trzech operacji: swap, insert, lub inversion
// ============================================================================
void EvolutionSolver::mutateAdvanced(Chromosome& chromosome)
{
    std::mt19937 gen(std::random_device{}()); // generator liczb losowych
    std::uniform_real_distribution<> dis(0.0, 1.0); // do wyboru typu mutacji
    std::uniform_int_distribution<> dist(0, (int)chromosome.size() - 1); // do wyboru indeksów

    if (chromosome.size() < 2) return; // nic nie robimy jeśli chromosom jest za krótki

    double choice = dis(gen); // losujemy liczbę z zakresu [0,1] – decyduje o typie mutacji

    if (choice < 0.33) {
        // === 1. Swap – zamiana dwóch losowych genów miejscami ===
        int i = dist(gen);
        int j = dist(gen);

        int tmp = chromosome[i];
        chromosome[i] = chromosome[j];
        chromosome[j] = tmp;
    }
    else if (choice < 0.66) {
        // === 2. Insert – wytnij element z jednej pozycji i wstaw w inną ===
        int i = dist(gen);
        int j = dist(gen);

        if (i != j) {
            int gene = chromosome[i]; // zapamiętaj wartość genu
            chromosome.erase(chromosome.begin() + i); // usuń go z pozycji i
            chromosome.insert(chromosome.begin() + j, gene); // wstaw go w pozycję j
        }
    }
    else {
        // === 3. Inversion – odwróć kolejność fragmentu permutacji ===
        int i = dist(gen);
        int j = dist(gen);

        if (i > j) {
            int temp = i;
            i = j;
            j = temp;
        }

        // Odwróć kolejność elementów w zakresie [i, j]
        std::reverse(chromosome.begin() + i, chromosome.begin() + j + 1);
    }
}

// ============================================================================
// Funkcja tournamentSelection – szczegółowo opisana wersja
// Wybiera najlepszego osobnika spośród losowo wybranych z populacji (selekcja turniejowa)
// Używana do doboru rodziców do krzyżowania
// ============================================================================
EvolutionSolver::Chromosome EvolutionSolver::tournamentSelection(const std::vector<Chromosome>& population, const std::vector<int>& fitness)
{
    std::mt19937 gen(std::random_device{}()); // generator liczb losowych

    Chromosome best;        // tutaj przechowamy najlepszego osobnika z turnieju
    int bestFit = INT_MAX;  // początkowo ustawiamy najlepszy koszt jako bardzo wysoki

    // Losujemy kilka osobników z populacji i wybieramy tego z najlepszym (najmniejszym) kosztem
    for (int i = 0; i < tournamentSize; ++i)
    {
        int idx = gen() % population.size(); // losowy indeks z przedziału populacji

        // Jeśli ten osobnik ma lepszy wynik (niższy makespan), to zapamiętaj go jako najlepszego
        if (fitness[idx] < bestFit)
        {
            bestFit = fitness[idx];       // aktualizuj najlepszy koszt
            best = population[idx];       // zapamiętaj cały chromosom (permutację)
        }
    }

    return best; // zwracamy najlepszego osobnika z turnieju
}


// ============================================================================
// Funkcja mutateSwap – mutacja typu "swap" z prawdopodobieństwem mutationRate
// Dla każdego genu w chromosomie losowo decydujemy, czy zamienić go z innym
// ============================================================================
void EvolutionSolver::mutateSwap(Chromosome& chromosome)
{
    std::mt19937 gen(std::random_device{}()); // generator liczb losowych

    // Rozkład losowy do określenia, czy mutacja nastąpi (porównywana z mutationRate)
    std::uniform_real_distribution<> probDist(0.0, 1.0);

    // Rozkład losowy do wyboru innego genu do zamiany
    std::uniform_int_distribution<> geneDist(0, (int)chromosome.size() - 1);

    // Iterujemy po każdym genie w chromosomie (czyli każdej pozycji permutacji)
    for (int i = 0; i < (int)chromosome.size(); ++i)
    {
        // Z prawdopodobieństwem mutationRate wykonujemy zamianę
        if (probDist(gen) < mutationRate)
        {
            int j = geneDist(gen); // losujemy drugi indeks

            if (i != j) // upewniamy się, że nie zamieniamy genu sam ze sobą
            {
                std::swap(chromosome[i], chromosome[j]); // zamieniamy dwa geny miejscami
            }
        }
    }
}

// ============================================================================
// Funkcja printSchedule – wypisuje najlepszy znaleziony harmonogram na ekran
// Harmonogram pochodzi z najlepszego osobnika w całym przebiegu algorytmu EA
// ============================================================================
void EvolutionSolver::printSchedule() const
{
    // Nagłówek informacyjny
    std::cout << "\n=== Najlepszy harmonogram (EvolutionSolver) ===\n";

    // Wyświetlamy wartość makespanu (czas zakończenia najpóźniejszego zadania)
    std::cout << "Makespan: " << bestMakespan << "\n";

    // Opis kolumn harmonogramu
    std::cout << "Operacje:\n";
    std::cout << "ID\tPriorytet\tStart\tEnd\n";

    // Iterujemy po wszystkich zadaniach w harmonogramie i wypisujemy je
    for (int i = 0; i < (int)bestSchedule.size(); ++i)
    {
        const Activity& task = bestSchedule[i]; // odwołanie do konkretnego zadania

        // ID – identyfikator zadania
        // Priorytet – pozycja w permutacji wejściowej
        // Start i End – czasy rozpoczęcia i zakończenia
        std::cout << task.id << "\t" << task.priority << "\t\t" << task.start_time << "\t" << task.end_time << "\n";
    }
}

// ============================================================================
// Funkcja saveScheduleCSV – zapisuje najlepszy harmonogram do pliku CSV
// Format zgodny z analizą danych lub wizualizacją w Pythonie
// ============================================================================
void EvolutionSolver::saveScheduleCSV(const std::string& filename) const
{
    // Otwieramy plik do zapisu
    std::ofstream out(filename.c_str());
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << filename << "\n";
        return; // zakończ jeśli nie udało się otworzyć pliku
    }

    // Nagłówek CSV z nazwami kolumn
    out << "job_id,start_time,end_time,duration,resources\n";

    // Iterujemy po wszystkich operacjach z najlepszego harmonogramu
    for (int i = 0; i < (int)bestSchedule.size(); ++i)
    {
        const Activity& task = bestSchedule[i];

        // Zapisujemy ID, czasy i długość trwania
        out << task.id << "," << task.start_time << "," << task.end_time << "," << task.duration << ",\"";

        // Wypisujemy wymagania zasobowe jako ciąg liczb oddzielonych spacją
        for (int j = 0; j < (int)task.resourceRequirements.size(); ++j)
        {
            out << task.resourceRequirements[j];
            if (j < (int)task.resourceRequirements.size() - 1)
                out << " "; // spacja między zasobami
        }

        out << "\"\n"; // zamknięcie pola CSV i przejście do nowej linii
    }

    out.close(); // zamknięcie pliku
}

// ============================================================================
// Funkcja zapiszDoCSV – zapisuje harmonogram do pliku CSV w rozszerzonym formacie
// Zawiera informacje o zadaniach: ID, czasach, długości, zasobach, poprzednikach i następcach
// ============================================================================
void EvolutionSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Otwórz plik do zapisu
    std::ofstream out(nazwaPliku.c_str());
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return; // zakończ jeśli plik nie otwarty
    }

    // Nagłówek kolumn zgodny z rozszerzonym formatem CSV
    out << "job_id,start_time,end_time,duration,resources,predecessors,successors\n";

    // Przejdź po wszystkich zadaniach w najlepszym harmonogramie
    for (int i = 0; i < (int)bestSchedule.size(); ++i)
    {
        const Activity& task = bestSchedule[i];

        // Zapisz dane podstawowe: ID, czasy, długość
        out << task.id << "," << task.start_time << "," << task.end_time << "," << task.duration << ",\"";

        // Zapisz zasoby jako string w cudzysłowie, oddzielone spacją
        for (int j = 0; j < (int)task.resourceRequirements.size(); ++j) {
            out << task.resourceRequirements[j];
            if (j < (int)task.resourceRequirements.size() - 1)
                out << " ";
        }
        out << "\",\"";

        // Zapisz poprzedników jako lista ID w cudzysłowie
        for (int j = 0; j < (int)task.predecessors.size(); ++j) {
            out << task.predecessors[j];
            if (j < (int)task.predecessors.size() - 1)
                out << " ";
        }
        out << "\",\"";

        // Zapisz następców jako lista ID w cudzysłowie
        for (int j = 0; j < (int)task.successors.size(); ++j) {
            out << task.successors[j];
            if (j < (int)task.successors.size() - 1)
                out << " ";
        }
        out << "\"\n"; // zakończ linię
    }

    out.close(); // zamknij plik
}

// ============================================================================
// Funkcja zapiszStatystykiDoCSV – zapisuje statystyki przebiegu algorytmu EA
// Dla danego runa (uruchomienia) oblicza: best, average, worst, stddev z kosztów pokoleń
// Zapisuje te dane do pliku CSV – umożliwia analizę porównawczą różnych uruchomień
// ============================================================================
void EvolutionSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    // Jeśli nie mamy żadnych danych, nie zapisujemy nic
    if (kosztyPokolen.empty()) {
        std::cerr << "Brak danych do zapisania statystyk.\n";
        return;
    }

    // Zainicjalizuj zmienne do statystyk
    double best = kosztyPokolen[0];   // najlepszy koszt (najmniejszy)
    double worst = kosztyPokolen[0];  // najgorszy koszt (największy)
    double sum = 0.0;                 // suma wszystkich kosztów

    // Przejście po wektorze kosztów i obliczenie best/worst/suma
    for (int i = 0; i < (int)kosztyPokolen.size(); ++i)
    {
        if (kosztyPokolen[i] < best) best = kosztyPokolen[i];
        if (kosztyPokolen[i] > worst) worst = kosztyPokolen[i];
        sum += kosztyPokolen[i];
    }

    double avg = sum / kosztyPokolen.size(); // średnia

    // Obliczanie odchylenia standardowego (stddev)
    double sumKw = 0.0;
    for (int i = 0; i < (int)kosztyPokolen.size(); ++i)
    {
        double roznica = kosztyPokolen[i] - avg; // różnica od średniej
        sumKw += roznica * roznica;              // kwadrat różnicy
    }
    double stddev = std::sqrt(sumKw / kosztyPokolen.size()); // pierwiastek średniokwadratowy

    // Otwieranie pliku do dopisania danych
    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku.c_str()).good(); // sprawdzenie czy plik już istnieje
    out.open(nazwaPliku.c_str(), std::ios::app); // tryb dopisywania

    // Sprawdzenie błędu otwarcia
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Jeśli plik dopiero tworzony – zapisz nagłówek kolumn
    if (!istnieje) {
        out << "run;best;average;worst;critical_path;avgDevCPM\n";
    }

    // Zapisz dane statystyczne do pliku (oddzielone średnikami)
    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << ";\n";
    out.close(); // zamknięcie pliku
}

// ============================================================================
// Funkcja zapiszWykorzystanieZasobow – zapisuje profil zużycia zasobów w czasie
// Dla każdego momentu czasu zapisuje ile jednostek każdego zasobu było użyte
// Format CSV: wiersze = chwile czasu, kolumny = zasoby (R1, R2, ...)
// ============================================================================
void EvolutionSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Krok 1: Wyznaczenie maksymalnego czasu trwania harmonogramu
    int maksCzas = 0;
    for (int i = 0; i < (int)bestSchedule.size(); ++i)
    {
        if (bestSchedule[i].end_time > maksCzas)
            maksCzas = bestSchedule[i].end_time; // zapamiętaj najdalszy koniec zadania
    }

    // Krok 2: Utworzenie macierzy zużycia [czas][zasób], inicjalizowane zerami
    std::vector<std::vector<int>> zuzycie(maksCzas + 1, std::vector<int>(liczbaZasobow, 0));

    // Krok 3: Uzupełnienie profilu zużycia zasobów – przechodzimy po każdym zadaniu
    for (int i = 0; i < (int)bestSchedule.size(); ++i)
    {
        const Activity& z = bestSchedule[i]; // aktualne zadanie
        for (int t = z.start_time; t < z.end_time; ++t) // dla każdego czasu trwania zadania
        {
            for (int r = 0; r < liczbaZasobow; ++r) // dla każdego zasobu
            {
                zuzycie[t][r] += z.resourceRequirements[r]; // dodaj wymaganie zasobowe do profilu
            }
        }
    }

    // Krok 4: Otwórz plik do zapisu danych
    std::ofstream out(nazwaPliku.c_str());
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Krok 5: Zapisz nagłówek kolumn: czas, R1, R2, ..., Rn
    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
        out << ",R" << (r + 1);
    out << "\n";

    // Krok 6: Zapisz dane dla każdego momentu czasu
    for (int t = 0; t <= maksCzas; ++t)
    {
        out << t; // czas
        for (int r = 0; r < liczbaZasobow; ++r)
            out << "," << zuzycie[t][r]; // zużycie zasobu r w czasie t
        out << "\n";
    }

    out.close(); // zamknięcie pliku
}