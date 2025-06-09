#include "SimulatedAnnealingSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath> // dla exp()
#include "ScheduleBuilder.h"


// Konstruktor: ustawiamy wszystkie parametry
SimulatedAnnealingSolver::SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter)
{
    temperaturaStartowa = startTemp; temperaturaKoncowa = endTemp; wspolczynnikChlodzenia = coolingRate; maksLiczbaIteracji = maxIter;
    makespan = 0;
}

void SimulatedAnnealingSolver::solve(const std::vector<Activity>& zadaniaWejscie,
                                     int liczbaZadan,
                                     int liczbaZasobow,
                                     const std::vector<int>& pojemnosci)
{
    std::mt19937 gen(std::random_device{}());
    std::vector<int> aktualnyGenotyp(liczbaZadan);
    std::iota(aktualnyGenotyp.begin(), aktualnyGenotyp.end(), 0);
    std::shuffle(aktualnyGenotyp.begin(), aktualnyGenotyp.end(), gen);

    int sumaTrwania = 0;
    for (const auto& z : zadaniaWejscie) sumaTrwania += z.duration;
    int maxCzas = sumaTrwania + 50;

    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));
    std::vector<Activity> harmonogramBufor;

    ScheduleBuilder::zbudujZPriorytetami(aktualnyGenotyp, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci, buforZuzycia, harmonogramBufor);
    int aktualnyKoszt = obliczMakespan(harmonogramBufor);

    std::vector<int> najlepszyGenotyp = aktualnyGenotyp;
    int najlepszyKoszt = aktualnyKoszt;

    double temperatura = temperaturaStartowa;

    kosztyIteracji.clear();
    historiaBestSoFar.clear();
    historiaCurrent.clear();

    for (int iter = 0; iter < maksLiczbaIteracji; ++iter)
    {
        std::vector<int> nowyGenotyp = aktualnyGenotyp;
        int i = gen() % liczbaZadan;
        int j = gen() % liczbaZadan;
        std::swap(nowyGenotyp[i], nowyGenotyp[j]);

        std::vector<std::vector<int>> lokalnyBufor(maxCzas, std::vector<int>(liczbaZasobow, 0));
        std::vector<Activity> lokalnyHarmonogram;
        ScheduleBuilder::zbudujZPriorytetami(nowyGenotyp, zadaniaWejscie,
            liczbaZadan, liczbaZasobow, pojemnosci, lokalnyBufor, lokalnyHarmonogram);
        int nowyKoszt = obliczMakespan(lokalnyHarmonogram);

        int delta = nowyKoszt - aktualnyKoszt;
        bool akceptuj = false;

        if (delta < 0)
        {
            akceptuj = true;
        }
        else
        {
            double prawdopodobienstwo = std::exp(-delta / temperatura);
            std::uniform_real_distribution<> dis(0.0, 1.0);
            if (dis(gen) < prawdopodobienstwo)
                akceptuj = true;
        }

        if (akceptuj)
        {
            aktualnyGenotyp = nowyGenotyp;
            aktualnyKoszt = nowyKoszt;
            if (nowyKoszt < najlepszyKoszt)
            {
                najlepszyGenotyp = nowyGenotyp;
                najlepszyKoszt = nowyKoszt;
            }
        }

        kosztyIteracji.push_back(aktualnyKoszt);
        historiaCurrent.push_back(aktualnyKoszt);
        historiaBestSoFar.push_back(najlepszyKoszt);

        temperatura *= wspolczynnikChlodzenia;

        if (iter % 500 == 0)
            std::cout << "[Iteracja " << iter << "] makespan = " << aktualnyKoszt
            << " (best = " << najlepszyKoszt << ")\n";
    }

    ScheduleBuilder::zbudujZPriorytetami(najlepszyGenotyp, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci, buforZuzycia, schedule);
    makespan = najlepszyKoszt;
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
