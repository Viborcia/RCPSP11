#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "Activity.h"
#include <vector>

class ScheduleBuilder
{
public:
    // Buduje harmonogram zgodnie z kolejnością zadań w permutacji
    static void zbudujZPriorytetami(
        const std::vector<int>& priorytety, // permutacja indeksów zadań
        const std::vector<Activity>& zadaniaWejscie,
        int liczbaZadan,
        int liczbaZasobow,
        const std::vector<int>& pojemnosci,
        std::vector<std::vector<int>>& zuzycie, // tablica [czas][zasób]
        std::vector<Activity>& wynik             // wynikowy harmonogram
    );
};

#endif
