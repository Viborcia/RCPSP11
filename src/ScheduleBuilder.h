#pragma once
#include <vector>
#include "Activity.h"

class ScheduleBuilder {
public:
    static void zbudujZPriorytetami(
        const std::vector<int>& priorytety,
        const std::vector<Activity>& zadania,
        int liczbaZadan,
        int liczbaZasobow,
        const std::vector<int>& pojemnosci,
        std::vector<std::vector<int>>& zuzycie,
        std::vector<Activity>& wynik);
};
