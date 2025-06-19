#include "SimulatedAnnealingSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <cmath> // dla exp()
#include "ScheduleBuilder.h"

SimulatedAnnealingSolver::SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter)
{
    temperaturaStartowa = startTemp;
    temperaturaKoncowa = endTemp;
    wspolczynnikChlodzenia = coolingRate;
    maksLiczbaIteracji = maxIter;
    makespan = 0;
}

// ============================================================================
// SimulatedAnnealingSolver::solve(...) – szczegółowo zakomentowana wersja
// ============================================================================

void SimulatedAnnealingSolver::solve(const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci)
{
    // Generator liczb losowych
    std::mt19937 gen(std::random_device{}());

    // Wygeneruj losowe rozwiązanie początkowe (permutacja zadań)
    std::vector<int> aktualnyGenotyp(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i)
        aktualnyGenotyp[i] = i;
    std::shuffle(aktualnyGenotyp.begin(), aktualnyGenotyp.end(), gen);

    // Ustal maksymalny horyzont czasowy (dla bufora zasobów)
    int sumaTrwania = 0;
    for (int i = 0; i < zadaniaWejscie.size(); ++i)
        sumaTrwania += zadaniaWejscie[i].duration;
    int maxCzas = sumaTrwania + 50;

    // Zbuduj harmonogram startowy
    std::vector<std::vector<int>> buforZuzycia(maxCzas, std::vector<int>(liczbaZasobow, 0));
    std::vector<Activity> harmonogramBufor;
    ScheduleBuilder::zbudujZPriorytetami(aktualnyGenotyp, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci,
        buforZuzycia, harmonogramBufor);

    // Oblicz koszt (makespan) aktualnego rozwiązania
    int aktualnyKoszt = obliczMakespan(harmonogramBufor);
    std::vector<int> najlepszyGenotyp = aktualnyGenotyp;
    int najlepszyKoszt = aktualnyKoszt;

    // Inicjalizacja temperatury początkowej
    double temperatura = temperaturaStartowa;

    // Wyczyszczenie historii i statystyk
    kosztyIteracji.clear();
    historiaBestSoFar.clear();
    historiaCurrent.clear();

    // Główna pętla symulowanego wyżarzania
    for (int iter = 0; iter < maksLiczbaIteracji; ++iter)
    {
        // Wygeneruj sąsiada przez zamianę dwóch losowych zadań
        std::vector<int> nowyGenotyp = aktualnyGenotyp;
        int i = gen() % liczbaZadan;
        int j = gen() % liczbaZadan;
        std::swap(nowyGenotyp[i], nowyGenotyp[j]);

        // Zbuduj harmonogram dla nowego rozwiązania
        std::vector<std::vector<int>> lokalnyBufor(maxCzas, std::vector<int>(liczbaZasobow, 0));
        std::vector<Activity> nowyHarmonogram;
        ScheduleBuilder::zbudujZPriorytetami(nowyGenotyp, zadaniaWejscie,
            liczbaZadan, liczbaZasobow, pojemnosci,
            lokalnyBufor, nowyHarmonogram);

        // Oblicz koszt nowego rozwiązania i różnicę kosztów (delta)
        int nowyKoszt = obliczMakespan(nowyHarmonogram);
        int delta = nowyKoszt - aktualnyKoszt;

        // Kryterium akceptacji:
        // akceptuj lepsze lub gorsze z pewnym prawdopodobieństwem
        if (delta < 0 || (std::exp(-delta / temperatura) > (double)(gen() % 10000) / 10000.0))
        {
            aktualnyGenotyp = nowyGenotyp;
            aktualnyKoszt = nowyKoszt;
            harmonogramBufor = nowyHarmonogram;
        }

        // Zapisz najlepsze rozwiązanie jeśli zostało poprawione
        if (aktualnyKoszt < najlepszyKoszt)
        {
            najlepszyKoszt = aktualnyKoszt;
            najlepszyGenotyp = aktualnyGenotyp;
        }

        // Obniż temperaturę zgodnie ze współczynnikiem chłodzenia
        temperatura *= wspolczynnikChlodzenia;

        // Zapisz dane do statystyk
        kosztyIteracji.push_back(nowyKoszt);
        historiaCurrent.push_back(aktualnyKoszt);
        historiaBestSoFar.push_back(najlepszyKoszt);
    }

    // Zapisz końcowy harmonogram i makespan
    makespan = najlepszyKoszt;
    schedule.clear();
    std::vector<std::vector<int>> finalBufor(maxCzas, std::vector<int>(liczbaZasobow, 0));
    ScheduleBuilder::zbudujZPriorytetami(najlepszyGenotyp, zadaniaWejscie,
        liczbaZadan, liczbaZasobow, pojemnosci,
        finalBufor, schedule);
}


// ============================================================================
// SimulatedAnnealingSolver – FUNKCJE POMOCNICZE (bardzo szczegółowo zakomentowane)
// ============================================================================

// Funkcja pomocnicza: oblicz maksymalny czas zakończenia (makespan)
int obliczMakespan(const std::vector<Activity>& harmonogram)
{
    int maks = 0; // Zmienna przechowująca aktualnie największy czas zakończenia

    // Przejdź po każdej operacji w harmonogramie
    for (int i = 0; i < harmonogram.size(); ++i)
    {
        // Jeśli koniec tej operacji jest większy niż obecny maks – zaktualizuj
        if (harmonogram[i].end_time > maks)
            maks = harmonogram[i].end_time;
    }

    // Zwróć maksymalny makespan
    return maks;
}


// Wypisuje na ekran harmonogram zapisany w zmiennej schedule
void SimulatedAnnealingSolver::printSchedule() const
{
    // Nagłówek dla czytelności
    std::cout << "\n=== Najlepszy harmonogram (SimulatedAnnealingSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Zadania:\n";
    std::cout << "ID\tStart\tEnd\n";

    // Dla każdej operacji w harmonogramie wypisz jej ID, czas startu i końca
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        std::cout << z.id << "\t" << z.start_time << "\t" << z.end_time << "\n";
    }
}


// Zapisuje końcowy harmonogram do pliku CSV
void SimulatedAnnealingSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku); // Otwórz plik do zapisu
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Wypisz nagłówek kolumn zgodny z oczekiwanym formatem
    out << "job_id,start_time,end_time,duration\n";

    // Wypisz dane każdej operacji do pliku CSV
    for (int i = 0; i < schedule.size(); ++i)
    {
        const Activity& z = schedule[i];
        out << z.id << "," << z.start_time << "," << z.end_time << "," << z.duration << "\n";
    }

    out.close(); // Zamknij plik
}


// Zapisuje statystyki z jednego uruchomienia (run) do CSV (best/avg/worst/std)
void SimulatedAnnealingSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "Brak danych do zapisania statystyk (kosztyIteracji).\n";
        return;
    }

    // Oblicz best/worst/avg
    double best = kosztyIteracji[0];
    double worst = kosztyIteracji[0];
    double sum = 0.0;

    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        if (kosztyIteracji[i] < best) best = kosztyIteracji[i];
        if (kosztyIteracji[i] > worst) worst = kosztyIteracji[i];
        sum += kosztyIteracji[i];
    }
    double avg = sum / kosztyIteracji.size();

    // Oblicz odchylenie standardowe
    double sumKw = 0.0;
    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        double roznica = kosztyIteracji[i] - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyIteracji.size());

    // Zapis do pliku CSV
    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Jeśli plik był pusty – dodaj nagłówek
    if (!istnieje)
    {
        out << "run;best;average;worst;std\n";
    }

    // Wypisz dane statystyczne
    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}


// Zapisuje best/avg/worst dla każdej iteracji najlepszego runa do CSV
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

    // Wypisz dane dla każdej iteracji
    for (int i = 0; i < kosztyIteracji.size(); ++i)
    {
        out << i << ";"
            << kosztyIteracji[i] << ";"
            << avgIteracji[i] << ";"
            << worstIteracji[i] << "\n";
    }

    out.close();
}


// Zapisuje dane o zużyciu zasobów w czasie (czas x zasoby)
void SimulatedAnnealingSolver::zapiszWykorzystanieZasobow(const std::string& nazwaPliku, int liczbaZasobow) const
{
    // Znajdź maksymalny czas zakończenia zadania
    int maxCzas = 0;
    for (int i = 0; i < schedule.size(); ++i)
    {
        if (schedule[i].end_time > maxCzas)
            maxCzas = schedule[i].end_time;
    }

    // Utwórz bufor zużycia zasobów: [czas][zasób]
    std::vector<std::vector<int>> zuzycie(maxCzas + 1, std::vector<int>(liczbaZasobow, 0));

    // Wypełnij bufor na podstawie harmonogramu
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

    // Zapisz dane do pliku CSV
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "czas";
    for (int r = 0; r < liczbaZasobow; ++r)
    {
        out << ",R" << (r + 1);
    }
    out << "\n";

    for (int t = 0; t <= maxCzas; ++t)
    {
        out << t;
        for (int r = 0; r < liczbaZasobow; ++r)
        {
            out << "," << zuzycie[t][r];
        }
        out << "\n";
    }

    out.close();
}


// Zapisuje porównanie best vs current z każdej iteracji (do wykresu)
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

    for (int i = 0; i < historiaCurrent.size(); ++i)
    {
        out << i << ";" << historiaCurrent[i] << ";" << historiaBestSoFar[i] << "\n";
    }

    out.close();
}
