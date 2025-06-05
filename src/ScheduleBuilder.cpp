// ScheduleBuilder.cpp — wersja zoptymalizowana z referencj¹ na wynik
#include "ScheduleBuilder.h"
#include <algorithm>

void ScheduleBuilder::zbudujZPriorytetami(
    const std::vector<int>& priorytety,
    const std::vector<Activity>& zadaniaWejscie,
    int liczbaZadan,
    int liczbaZasobow,
    const std::vector<int>& pojemnosci,
    std::vector<std::vector<int>>& zuzycie,
    std::vector<Activity>& wynik)
{
    wynik.clear();

    std::vector<Activity> zadania;
    zadania.reserve(liczbaZadan);
    for (const Activity& z : zadaniaWejscie) {
        Activity kopia = z;
        kopia.start_time = 0;
        kopia.end_time = 0;
        zadania.push_back(kopia);
    }

    std::vector<bool> zaplanowane(liczbaZadan, false);
    std::vector<int> end_times(liczbaZadan, 0);

    for (size_t t = 0; t < zuzycie.size(); ++t)
        std::fill(zuzycie[t].begin(), zuzycie[t].end(), 0);

    std::vector<int> kolejka = priorytety;
    bool zmiana = true;

    while (!kolejka.empty() && zmiana)
    {
        zmiana = false;
        std::vector<int> nowaKolejka;

        for (int i : kolejka)
        {
            if (zaplanowane[i]) continue;

            Activity& zad = zadania[i];
            int earliestStart = 0;
            bool poprzednicyOK = true;

            for (int p : zad.predecessors) {
                if (!zaplanowane[p]) {
                    poprzednicyOK = false;
                    break;
                }
                if (end_times[p] > earliestStart)
                    earliestStart = end_times[p];
            }

            if (!poprzednicyOK) {
                nowaKolejka.push_back(i);
                continue;
            }

            bool znaleziono = false;
            for (int t = earliestStart; t < (int)zuzycie.size(); ++t)
            {
                bool zasobyOk = true;
                for (int dt = 0; dt < zad.duration && zasobyOk; ++dt)
                {
                    int czas = t + dt;
                    if (czas >= (int)zuzycie.size()) {
                        zasobyOk = false;
                        break;
                    }
                    for (int r = 0; r < liczbaZasobow; ++r)
                        if (zuzycie[czas][r] + zad.resourceRequirements[r] > pojemnosci[r]) {
                            zasobyOk = false;
                            break;
                        }
                }

                if (zasobyOk)
                {
                    for (int dt = 0; dt < zad.duration; ++dt)
                        for (int r = 0; r < liczbaZasobow; ++r)
                            zuzycie[t + dt][r] += zad.resourceRequirements[r];

                    zad.start_time = t;
                    zad.end_time = t + zad.duration;
                    end_times[i] = zad.end_time;

                    zaplanowane[i] = true;
                    wynik.push_back(zad);
                    zmiana = true;
                    znaleziono = true;
                    break;
                }
            }

            if (!znaleziono) {
                nowaKolejka.push_back(i);
            }
        }

        kolejka = nowaKolejka;
    }
}
