#include "RCPSPLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

bool RCPSPLoader::wczytajZPliku(const std::string& sciezka) {
    std::ifstream file(sciezka);
    if (!file.is_open()) {
        std::cerr << "Nie mogę otworzyć pliku: " << sciezka << "\n";
        return false;
    }

    std::string line;

    // 1. Szukamy liczby zadań (linia: "jobs (incl. supersource/sink ):  32")
    while (std::getline(file, line)) {
        if (line.find("jobs") != std::string::npos) {
            std::istringstream iss(line);
            std::string tmp;
            while (iss >> tmp) {
                try {
                    liczbaZadan = std::stoi(tmp);
                    break;
                } catch (...) {}
            }
            break;
        }
    }

    // 2. Szukamy liczby zasobów (linia: "- renewable : 4 R")
    while (std::getline(file, line)) {
        if (line.find("renewable") != std::string::npos) {
            std::istringstream iss(line);
            std::string tmp;
            while (iss >> tmp) {
                try {
                    liczbaZasobow = std::stoi(tmp);
                    break;
                } catch (...) {}
            }
            break;
        }
    }

    std::cout << "[debug] liczbaZadan = " << liczbaZadan << "\n";
    std::cout << "[debug] liczbaZasobow = " << liczbaZasobow << "\n";

    // 3. Szukamy sekcji "PRECEDENCE RELATIONS:"
    while (std::getline(file, line)) {
        if (line.find("PRECEDENCE RELATIONS") != std::string::npos)
            break;
    }

    // Pomijamy nagłówek: "jobnr.  #modes  #successors successors"
    std::getline(file, line);

    zadania.resize(liczbaZadan);
    for (int i = 0; i < liczbaZadan; ++i) {
        int id, modes, count;
        file >> id >> modes >> count;
        Activity act;
        act.id = id - 1;
        for (int j = 0; j < count; ++j) {
            int succ;
            file >> succ;
            act.successors.push_back(succ - 1);
        }
        zadania[act.id] = act;
    }

    // 4. Szukamy sekcji "REQUESTS/DURATIONS:"
    while (std::getline(file, line)) {
        if (line.find("REQUESTS/DURATIONS") != std::string::npos)
            break;
    }

    // Pomijamy linię nagłówkową z nazwami kolumn
    std::getline(file, line);
    std::getline(file, line);

    for (int i = 0; i < liczbaZadan; ++i) {
        int id, mode, duration;
        file >> id >> mode >> duration;
        zadania[id - 1].duration = duration;
        zadania[id - 1].resourceRequirements.resize(liczbaZasobow);
        for (int j = 0; j < liczbaZasobow; ++j) {
            file >> zadania[id - 1].resourceRequirements[j];
        }
    }

    // 5. Szukamy sekcji "RESOURCEAVAILABILITIES:"
    while (std::getline(file, line)) {
        if (line.find("RESOURCEAVAILABILITIES") != std::string::npos)
            break;
    }

    // Pomijamy linię nagłówka "R1 R2 R3 R4"
    std::getline(file, line);

    zasobyPojemnosc.resize(liczbaZasobow);
    for (int i = 0; i < liczbaZasobow; ++i)
        file >> zasobyPojemnosc[i];

        // Uzupełniamy pole predecessors na podstawie successors
    for (const auto& akt : zadania) {
        for (int succ : akt.successors) {
            zadania[succ].predecessors.push_back(akt.id);
        }
    }


    return true;
}





void RCPSPLoader::wypisz() const {
    std::cout << "Liczba zadan: " << liczbaZadan << ", Liczba zasobow: " << liczbaZasobow << "\n";
    std::cout << "Pojemnosci zasobow: ";
    for (int r : zasobyPojemnosc) std::cout << r << " ";
    std::cout << "\n";

    for (const auto& z : zadania) {
        std::cout << "Zadanie " << z.id << " (czas: " << z.duration << ", zasoby: ";
        for (int r : z.resourceRequirements) std::cout << r << " ";
        std::cout << ") poprzednicy: ";
        for (int p : z.predecessors) std::cout << p << " ";
        std::cout << "\n";
    }
}
