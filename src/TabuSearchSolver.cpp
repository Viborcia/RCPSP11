#include "TabuSearchSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <limits>
#include <numeric>
#include <cmath>

TabuSearchSolver::TabuSearchSolver(int liczbaIteracji, int dlugoscTabu)
    : liczbaIteracji(liczbaIteracji), dlugoscTabu(dlugoscTabu), makespan(0)
{}

void TabuSearchSolver::solve(const std::vector<Activity>& zadaniaWejscie,
                             int liczbaZadan,
                             int liczbaZasobow,
                             const std::vector<int>& pojemnosci)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, liczbaZadan - 1);

    std::vector<int> priorytety(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i) priorytety[i] = i;
    std::shuffle(priorytety.begin(), priorytety.end(), gen);

    std::vector<Activity> najlepszyHarmonogram = zbudujHarmonogramZPriorytetami(priorytety, zadaniaWejscie, liczbaZadan, liczbaZasobow, pojemnosci);
    int najlepszyKoszt = obliczMakespan(najlepszyHarmonogram);

    std::vector<std::pair<int, int>> tabuLista;
    std::vector<Activity> aktualnyHarmonogram = najlepszyHarmonogram;
    int bezPoprawy = 0;

    for (int iter = 0; iter < liczbaIteracji; ++iter)
    {
        std::vector<int> najlepszySasiad = priorytety;
        int najlepszySasiadKoszt = std::numeric_limits<int>::max();
        bool znaleziono = false;
        int najlepszyI = -1, najlepszyJ = -1;

        for (int i = 0; i < liczbaZadan - 1; ++i)
{
    for (int j = i + 1; j < liczbaZadan; ++j)
    {
        std::vector<int> sasiad = priorytety;
        std::swap(sasiad[i], sasiad[j]);

        bool wTabu = std::find(tabuLista.begin(), tabuLista.end(), std::make_pair(i, j)) != tabuLista.end() ||
                     std::find(tabuLista.begin(), tabuLista.end(), std::make_pair(j, i)) != tabuLista.end();

        auto harmonogram = zbudujHarmonogramZPriorytetami(sasiad, zadaniaWejscie, liczbaZadan, liczbaZasobow, pojemnosci);
        int koszt = obliczMakespan(harmonogram);

        if (wTabu && koszt >= najlepszyKoszt)
            continue;

        if (koszt < najlepszySasiadKoszt)
        {
            najlepszySasiadKoszt = koszt;
            najlepszySasiad = sasiad;
            najlepszyI = i;
            najlepszyJ = j;
            znaleziono = true;
        }
    }
}


        if (!znaleziono)
        {
            std::shuffle(priorytety.begin(), priorytety.end(), gen);
            bezPoprawy = 0;
            tabuLista.clear();
            continue;
        }

        // Aktualizacja permutacji i tabu
        priorytety = najlepszySasiad;
        if (najlepszyI != -1 && najlepszyJ != -1)
            tabuLista.emplace_back(najlepszyI, najlepszyJ);
        if ((int)tabuLista.size() > dlugoscTabu)
            tabuLista.erase(tabuLista.begin());

        aktualnyHarmonogram = zbudujHarmonogramZPriorytetami(priorytety, zadaniaWejscie, liczbaZadan, liczbaZasobow, pojemnosci);
        int aktualnyKoszt = obliczMakespan(aktualnyHarmonogram);

        kosztyIteracji.push_back(aktualnyKoszt);
        historiaCurrent.push_back(aktualnyKoszt);
        historiaBestSoFar.push_back(najlepszyKoszt);
        avgIteracji.push_back(std::accumulate(kosztyIteracji.begin(), kosztyIteracji.end(), 0.0) / kosztyIteracji.size());
        worstIteracji.push_back(*std::max_element(kosztyIteracji.begin(), kosztyIteracji.end()));

        if (aktualnyKoszt < najlepszyKoszt)
        {
            najlepszyKoszt = aktualnyKoszt;
            najlepszyHarmonogram = aktualnyHarmonogram;
            bezPoprawy = 0;
        }
        else
        {
            bezPoprawy++;
        }

        if (bezPoprawy > 100)
        {
            std::shuffle(priorytety.begin(), priorytety.end(), gen);
            bezPoprawy = 0;
            tabuLista.clear();
        }
    }

    makespan = najlepszyKoszt;
    schedule = najlepszyHarmonogram;
}


std::vector<Activity> TabuSearchSolver::zbudujHarmonogramZPriorytetami(
    const std::vector<int>& priorytety,
    const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    std::vector<Activity> zadania = zadaniaWejscie;
    std::vector<int> priorytetZadan(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i)
        priorytetZadan[priorytety[i]] = i;

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
            return priorytetZadan[a] < priorytetZadan[b];
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

    return harmonogram;
}

int TabuSearchSolver::obliczMakespan(const std::vector<Activity>& harmonogram) const
{
    int maks = 0;
    for (const auto& z : harmonogram)
        if (z.end_time > maks)
            maks = z.end_time;
    return maks;
}

void TabuSearchSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (TabuSearchSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Zadania:\n";
    std::cout << "ID\tStart\tEnd\n";

    for (const auto& z : schedule)
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
}

void TabuSearchSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "job_id,start_time,end_time,duration\n";
    for (const auto& z : schedule)
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << "\n";

    out.close();
}

// Pozostałe funkcje zapisu statystyk (takie jak w SA/RandomSolver) – chcesz, żebym je też teraz dodał?


void TabuSearchSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = *std::min_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double worst = *std::max_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double avg = std::accumulate(kosztyIteracji.begin(), kosztyIteracji.end(), 0.0) / kosztyIteracji.size();

    double sumKw = 0.0;
    for (int koszt : kosztyIteracji)
    {
        double roznica = koszt - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyIteracji.size());

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "run;best;average;worst;std\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}

void TabuSearchSolver::zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const
{
    if (historiaCurrent.empty() || historiaBestSoFar.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania best vs current.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "iter;current;best_so_far\n";
    }

    for (int i = 0; i < historiaCurrent.size(); ++i)
    {
        out << i << ";" << historiaCurrent[i] << ";" << historiaBestSoFar[i] << "\n";
    }

    out.close();
}

void TabuSearchSolver::zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const
{
    if (kosztyIteracji.empty() || avgIteracji.empty() || worstIteracji.empty())
    {
        std::cerr << "[Tabu] Brak danych do zapisania kosztów najlepszego runa.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "iter;best;avg;worst\n";
    }

    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        out << i << ";" << kosztyIteracji[i] << ";" << avgIteracji[i] << ";" << worstIteracji[i] << "\n";
    }

    out.close();
}

void TabuSearchSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    int maxCzas = 0;
    for (const auto& z : schedule)
        if (z.end_time > maxCzas)
            maxCzas = z.end_time;

    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    for (const auto& z : schedule)
    {
        for (int t = z.start_time; t < z.end_time; ++t)
        {
            for (int r = 0; r < liczbaZasobow; ++r)
                zuzycie[t][r] += z.resourceRequirements[r];
        }
    }

    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "[Tabu] Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

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
