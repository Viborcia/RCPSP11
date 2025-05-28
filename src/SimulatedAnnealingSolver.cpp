#include "SimulatedAnnealingSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath> // dla exp()

// Konstruktor: ustawiamy wszystkie parametry
SimulatedAnnealingSolver::SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter)
{
    temperaturaStartowa = startTemp; temperaturaKoncowa = endTemp; wspolczynnikChlodzenia = coolingRate; maksLiczbaIteracji = maxIter;
    makespan = 0;
}

void SimulatedAnnealingSolver::solve(const std::vector<Activity>& zadaniaWejscie, int liczbaZadan, int liczbaZasobow, const std::vector<int>& pojemnosci)
{
    std::mt19937 gen(std::random_device{}());
    makespan = 0;
    kosztyIteracji.clear();
    historiaCurrent.clear();
    historiaBestSoFar.clear();
    avgIteracji.clear();
    worstIteracji.clear();

    // Losowa permutacja priorytetów
    std::vector<int> aktualnyGenotyp(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i) aktualnyGenotyp[i] = i;
    std::shuffle(aktualnyGenotyp.begin(), aktualnyGenotyp.end(), gen);

    std::vector<Activity> aktualnyHarmonogram = generujHarmonogram(aktualnyGenotyp, zadaniaWejscie, liczbaZadan, liczbaZasobow, pojemnosci);
    int aktualnyKoszt = obliczMakespan(aktualnyHarmonogram);

    std::vector<int> najlepszyGenotyp = aktualnyGenotyp;
    std::vector<Activity> najlepszyHarmonogram = aktualnyHarmonogram;
    int najlepszyKoszt = aktualnyKoszt;

    double T = temperaturaStartowa;
    int iteracja = 0;
    std::uniform_int_distribution<> dist(0, liczbaZadan - 1);

    while (T > temperaturaKoncowa && iteracja < maksLiczbaIteracji)
    {
        std::vector<int> nowyGenotyp = aktualnyGenotyp;
        int i = dist(gen), j = dist(gen);
        while (i == j) j = dist(gen);
        std::swap(nowyGenotyp[i], nowyGenotyp[j]);

        std::vector<Activity> nowyHarmonogram = generujHarmonogram(nowyGenotyp, zadaniaWejscie, liczbaZadan, liczbaZasobow, pojemnosci);
        int nowyKoszt = obliczMakespan(nowyHarmonogram);
        int delta = nowyKoszt - aktualnyKoszt;

        if (delta < 0 || (std::exp(-delta / T) > ((double)rand() / RAND_MAX)))
        {
            aktualnyGenotyp = nowyGenotyp;
            aktualnyHarmonogram = nowyHarmonogram;
            aktualnyKoszt = nowyKoszt;
        }

        if (aktualnyKoszt < najlepszyKoszt)
        {
            najlepszyKoszt = aktualnyKoszt;
            najlepszyGenotyp = aktualnyGenotyp;
            najlepszyHarmonogram = aktualnyHarmonogram;
        }

        kosztyIteracji.push_back(aktualnyKoszt);
        historiaCurrent.push_back(aktualnyKoszt);
        historiaBestSoFar.push_back(najlepszyKoszt);

        int suma = 0, najgorszy = aktualnyKoszt;
        for (int k = 0; k < kosztyIteracji.size(); ++k)
        {
            suma += kosztyIteracji[k];
            if (kosztyIteracji[k] > najgorszy)
                najgorszy = kosztyIteracji[k];
        }
        double avg = static_cast<double>(suma) / kosztyIteracji.size();
        avgIteracji.push_back(avg);
        worstIteracji.push_back(najgorszy);

        T *= wspolczynnikChlodzenia;
        iteracja++;
    }

    makespan = najlepszyKoszt;
    schedule = najlepszyHarmonogram;
}

std::vector<Activity> generujHarmonogram(
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

    std::vector<bool> zaplanowane(liczbaZadan, false);
    std::vector<Activity> zaplanowaneZadania;

    // Ustal maksymalny horyzont czasowy (np. 2x suma czasów trwania)
    int maxCzas = 0;
    for (const auto& z : zadania) maxCzas += z.duration;
    maxCzas *= 2;

    // Macierz zużycia zasobów: czas × zasób
    std::vector<std::vector<int>> zuzycie(maxCzas, std::vector<int>(liczbaZasobow, 0));

    int zaplanowaneLicznik = 0;
    while (zaplanowaneLicznik < liczbaZadan)
    {
        std::vector<int> eligible;
        for (int i = 0; i < liczbaZadan; ++i)
        {
            if (zaplanowane[i]) continue;

            bool ok = true;
            for (int p : zadania[i].predecessors)
                if (!zaplanowane[p]) {
                    ok = false;
                    break;
                }

            if (ok) eligible.push_back(i);
        }

        std::sort(eligible.begin(), eligible.end(), [&](int a, int b) {
            return priorytetZadan[a] < priorytetZadan[b];
        });

        for (int i : eligible)
        {
            const Activity& zad = zadania[i];
            int earliestStart = 0;

            // najwcześniejszy moment startu ze względu na poprzedników
            for (int p : zad.predecessors)
                if (zadania[p].end_time > earliestStart)
                    earliestStart = zadania[p].end_time;

            // Szukamy pierwszego możliwego czasu startu z dostępnością zasobów
            for (int t = earliestStart; t < maxCzas; ++t)
            {
                bool zasobyOk = true;
                for (int dt = 0; dt < zad.duration; ++dt)
                {
                    int czas = t + dt;
                    for (int r = 0; r < liczbaZasobow; ++r)
                    {
                        if (zuzycie[czas][r] + zad.resourceRequirements[r] > pojemnosci[r])
                        {
                            zasobyOk = false;
                            break;
                        }
                    }
                    if (!zasobyOk) break;
                }

                if (zasobyOk)
                {
                    // Rezerwujemy zasoby
                    for (int dt = 0; dt < zad.duration; ++dt)
                    {
                        int czas = t + dt;
                        for (int r = 0; r < liczbaZasobow; ++r)
                            zuzycie[czas][r] += zad.resourceRequirements[r];
                    }

                    zadania[i].start_time = t;
                    zadania[i].end_time = t + zad.duration;
                    zaplanowane[i] = true;
                    zaplanowaneZadania.push_back(zadania[i]);
                    zaplanowaneLicznik++;
                    break;
                }
            }
        }
    }

    return zadania;
}


int obliczMakespan(const std::vector<Activity>& harmonogram)
{
    int maks = 0;
    for (const auto& z : harmonogram)
        if (z.end_time > maks) maks = z.end_time;
    return maks;
}



void SimulatedAnnealingSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (SimulatedAnnealingSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Zadania:\n";
    std::cout << "ID\tStart\tEnd\n";

    for (const auto& z : schedule)
    {
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
    }
}


// Zapis do CSV
void SimulatedAnnealingSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek zgodny ze skryptem Pythona
    out << "job_id,start_time,end_time,duration\n";
    for (const auto& z : schedule)
    {
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << "\n";
    }

    out.close();
}




void SimulatedAnnealingSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "Brak danych do zapisania statystyk (kosztyIteracji).\n";
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
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "run;best;average;worst;std\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}


void SimulatedAnnealingSolver::zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const
{
    if (kosztyIteracji.empty() || avgIteracji.empty() || worstIteracji.empty()) 
    {
        std::cerr << "[SA] Brak danych do zapisania kosztów najlepszego runa.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) 
    {
        std::cerr << "[SA] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) 
    {
        out << "iter;best;avg;worst\n";
    }

    for (int i = 0; i < kosztyIteracji.size(); ++i) 
    {
        out << i << ";" 
            << kosztyIteracji[i] << ";" 
            << avgIteracji[i] << ";" 
            << worstIteracji[i] << "\n";
    }

    out.close();
}


void SimulatedAnnealingSolver::zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const
{
    if (historiaCurrent.empty() || historiaBestSoFar.empty()) 
    {
        std::cerr << "[SA] Brak danych do zapisania best vs current.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) 
    {
        std::cerr << "[SA] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) 
    {
        out << "iter;current;best_so_far\n";
    }

    for (int i = 0; i < historiaCurrent.size(); ++i) {
        out << i << ";" << historiaCurrent[i] << ";" << historiaBestSoFar[i] << "\n";
    }

    out.close();
}


void SimulatedAnnealingSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Szukamy maksymalnego czasu (makespan)
    int maxCzas = 0;
    for (const auto& z : schedule)
        if (z.end_time > maxCzas)
            maxCzas = z.end_time;

    // Tablica: czas x zasoby
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
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówki
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
