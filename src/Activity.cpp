#include "Activity.h"
#include <vector>
#include <algorithm>

// ============================================================================
// Funkcja oblicza długość ścieżki krytycznej (makespan) w grafie zadań.
// Zakłada, że każde zadanie ma zdefiniowanych poprzedników (predecessors).
// Działa podobnie do obliczania najwcześniejszych czasów zakończenia.
// ============================================================================

int obliczDlugoscSciezkiKrytycznej(const std::vector<Activity>& zadania)
{
    // Tablica przechowująca najwcześniejszy czas zakończenia dla każdego zadania
    std::vector<int> najwczesniejszyKoniec(zadania.size(), 0);

    // Przejdź po każdym zadaniu, aby obliczyć jego najwcześniejszy koniec
    for (int i = 0; i < zadania.size(); ++i)
    {
        int start = 0; // Czas rozpoczęcia zadania (zależy od poprzedników)

        // Przejdź po wszystkich poprzednikach zadania
        for (int j = 0; j < zadania[i].predecessors.size(); ++j)
        {
            int poprzednik = zadania[i].predecessors[j];

            // Aktualizuj czas startu: musi być >= końca poprzednika
            if (najwczesniejszyKoniec[poprzednik] > start)
                start = najwczesniejszyKoniec[poprzednik];
        }

        // Oblicz koniec aktualnego zadania: start + czas trwania
        najwczesniejszyKoniec[zadania[i].id] = start + zadania[i].duration;
    }

    // Znajdź maksymalny czas zakończenia – to długość ścieżki krytycznej
    int maks = 0;
    for (int i = 0; i < najwczesniejszyKoniec.size(); ++i)
    {
        if (najwczesniejszyKoniec[i] > maks)
            maks = najwczesniejszyKoniec[i];
    }

    return maks; // Zwróć długość ścieżki krytycznej
}
 