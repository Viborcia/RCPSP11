#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <vector>

// Struktura reprezentująca jedną aktywność w RCPSP
struct Activity {
    int id;  // ID aktywności
    int duration;  // czas trwania
    std::vector<int> resourceRequirements; // zapotrzebowanie na zasoby
    std::vector<int> predecessors; // lista ID poprzedników
    std::vector<int> successors;


    int start_time = 0;  // czas rozpoczęcia
    int end_time = 0;    // czas zakończenia
    int priority = 0;    // priorytet (dla harmonogramowania)
};

#endif // ACTIVITY_H

int obliczDlugoscSciezkiKrytycznej(const std::vector<Activity>& zadania);
