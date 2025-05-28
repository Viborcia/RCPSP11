/*#include "Activity.h"
#include <vector>
#include <algorithm>

// Funkcja oblicza długość ścieżki krytycznej w grafie zadań
int obliczDlugoscSciezkiKrytycznej(const std::vector<Activity>& zadania) {
    std::vector<int> najwczesniejszyKoniec(zadania.size(), 0);

    // Dla każdego zadania oblicz najwcześniejszy możliwy czas zakończenia
    for (const auto& akt : zadania) {
        int start = 0;
        for (int p : akt.predecessors) {
            start = std::max(start, najwczesniejszyKoniec[p]);
        }
        najwczesniejszyKoniec[akt.id] = start + akt.duration;
    }

    // Zwróć maksymalny czas zakończenia — to długość ścieżki krytycznej
    return *std::max_element(najwczesniejszyKoniec.begin(), najwczesniejszyKoniec.end());
}
*/