#ifndef RCPSP_LOADER_H
#define RCPSP_LOADER_H
#include "Activity.h"

#include <vector>
#include <string>


class RCPSPLoader {
public:
    int liczbaZadan;
    int liczbaZasobow;
    std::vector<int> zasobyPojemnosc; // Rk
    std::vector<Activity> zadania;
    int getLiczbaZasobow() const { return liczbaZasobow; }


    bool wczytajZPliku(const std::string& sciezka);
    void wypisz() const;
};

#endif
