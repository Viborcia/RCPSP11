#include "ScheduleBuilder.h"
#include <vector>
#include <iostream>

// ============================================================================
// Funkcja pomocnicza: pozwala wygodnie modyfikować lub odczytać zużycie zasobu
// zasob(zuzycie, t, r) zwraca referencję do wartości zasobu r w czasie t
// ============================================================================
int& zasob(std::vector<std::vector<int>>& zuzycie, int t, int r)
{
    return zuzycie[t][r];
}

// ============================================================================
// Funkcja ScheduleBuilder::zbudujZPriorytetami
// Buduje harmonogram wykonania zadań na podstawie podanej permutacji (priorytety)
// Ustalana jest kolejność wykonania, uwzględniane są zasoby i zależności
// ============================================================================
void ScheduleBuilder::zbudujZPriorytetami(
    const std::vector<int>& priorytety,                // permutacja ID zadań wg kolejności wykonania
    const std::vector<Activity>& zadaniaWejscie,       // oryginalna lista zadań (ze wszystkimi danymi)
    int liczbaZadan,                                   // liczba zadań
    int liczbaZasobow,                                 // liczba typów zasobów (np. R1, R2...)
    const std::vector<int>& pojemnosci,                // wektor pojemności każdego zasobu (ile mamy dostępnego)
    std::vector<std::vector<int>>& zuzycie,            // bufor [czas][zasob] – ile jednostek zasobu zajętych w danym momencie
    std::vector<Activity>& wynik)                      // wynikowy harmonogram (lista zadań z ustawionym czasem startu i końca)
{
    wynik.clear(); // Czyścimy harmonogram wynikowy (na wszelki wypadek)

    // Utwórz lokalną kopię zadań i wyzeruj ich czasy startu/końca
    std::vector<Activity> zadania;
    zadania.reserve(liczbaZadan); // Rezerwujemy miejsce w pamięci
    for (int i = 0; i < zadaniaWejscie.size(); ++i)
    {
        Activity kopia = zadaniaWejscie[i];
        kopia.start_time = 0;
        kopia.end_time = 0;
        zadania.push_back(kopia);
    }

    // Tablica: czy dane zadanie zostało już zaplanowane
    std::vector<bool> zaplanowane(liczbaZadan, false);
    // Tablica: kiedy kończy się każde zadanie (dla jego potomków)
    std::vector<int> end_times(liczbaZadan, 0);

    // Wyzeruj bufor zużycia zasobów (czas × zasób)
    for (int t = 0; t < zuzycie.size(); ++t)
        for (int r = 0; r < zuzycie[t].size(); ++r)
            zasob(zuzycie, t, r) = 0;

    // Skopiuj kolejność zadań (permutację priorytetów)
    std::vector<int> kolejka = priorytety;
    bool zmiana = true; // Flaga: czy udało się coś zaplanować w tej iteracji

    // Główna pętla: dopóki są zadania do zaplanowania i w danej iteracji coś zaplanowano
    while (!kolejka.empty() && zmiana)
    {
        zmiana = false; // Na początku iteracji zakładamy, że nie zaplanujemy nic
        std::vector<int> nowaKolejka; // Zadania, których nie dało się teraz zaplanować

        // Przejdź po zadaniach z kolejki
        for (int i = 0; i < kolejka.size(); ++i)
        {
            int indeks = kolejka[i];
            if (zaplanowane[indeks]) continue; // Już zaplanowane – pomijamy

            Activity& zad = zadania[indeks]; // Referencja do bieżącego zadania
            int earliestStart = 0;           // Najwcześniejszy czas startu (zależny od poprzedników)
            bool poprzednicyOK = true;       // Czy wszyscy poprzednicy zostali już zaplanowani

            // Sprawdź poprzedników zadania
            for (int j = 0; j < zad.predecessors.size(); ++j)
            {
                int p = zad.predecessors[j];
                if (!zaplanowane[p])
                {
                    poprzednicyOK = false; // któryś poprzednik niegotowy → zadanie odłożone
                    break;
                }
                if (end_times[p] > earliestStart)
                    earliestStart = end_times[p]; // musimy zaczekać na koniec poprzednika
            }

            if (!poprzednicyOK)
            {
                nowaKolejka.push_back(indeks); // nie gotowe – wróci w kolejnej iteracji
                continue;
            }

            // Znajdź pierwszy moment w czasie, kiedy wszystkie zasoby są dostępne
            int t = earliestStart; // startujemy od najwcześniejszego dozwolonego czasu
            while (true)
            {
                bool zasobyOK = true; // Czy zasoby są dostępne?

                // Sprawdź każdy moment trwania zadania
                for (int dt = 0; dt < zad.duration; ++dt)
                {
                    int moment = t + dt;
                    if (moment >= zuzycie.size()) // przekroczyliśmy zakres tablicy
                    {
                        zasobyOK = false;
                        break;
                    }

                    // Sprawdź wszystkie zasoby dla tej jednostki czasu
                    for (int r = 0; r < liczbaZasobow; ++r)
                    {
                        if (zasob(zuzycie, moment, r) + zad.resourceRequirements[r] > pojemnosci[r])
                        {
                            zasobyOK = false;
                            break;
                        }
                    }
                    if (!zasobyOK) break; // przerwij dalsze sprawdzanie
                }

                if (zasobyOK) break; // znaleźliśmy odpowiednie okno czasowe

                t += 1; // spróbuj w następnym czasie
            }

            // Zapisz czasy startu i końca zadania
            zad.start_time = t;
            zad.end_time = t + zad.duration;

            // Zarezerwuj zasoby na czas działania zadania
            for (int dt = 0; dt < zad.duration; ++dt)
            {
                int moment = t + dt;
                for (int r = 0; r < liczbaZasobow; ++r)
                {
                    zasob(zuzycie, moment, r) += zad.resourceRequirements[r];
                }
            }

            zaplanowane[indeks] = true;        // Oznacz zadanie jako zaplanowane
            end_times[indeks] = zad.end_time;  // Zapisz czas zakończenia
            wynik.push_back(zad);              // Dodaj do wyniku
            zmiana = true;                     // Coś zaplanowano w tej iteracji
        }

        kolejka = nowaKolejka; // W kolejnej iteracji spróbujemy zaplanować pozostałe
    }
}
