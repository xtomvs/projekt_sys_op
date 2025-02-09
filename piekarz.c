#include "default.h"

int semid;
int shmid;
Shared *shmptr;

void initialize_semaphores() 
{
    //próba uzyskania dostepu do semaforow
    semid =  semget(SEM_KEY, TOTAL_SEMS, 0600);

    if (semid == -1)
    {
        perror("Błąd podczas dostępu do semaforów");
        exit(EXIT_FAILURE);
    }

}

void setup_shared_memory()
{
    //próba uzyskania segmentu pamieci wspoldzielonej
    shmid = shmget(SHM_KEY, sizeof(Shared), 0600);
    if (shmid == -1)
    {
        perror("Błąd podczas dostępu do segmentu pamięci");
        exit(EXIT_FAILURE);
    }

    shmptr = (Shared *)shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1) 
    {
        perror("Błąd podczas mapowania pamięci wspołdzielonej");
        exit(EXIT_FAILURE);
    }
}

// Funkcja inicjalizująca podajniki
void inicjalizuj_podajniki(Shared* shmptr) {
    for (int i = 0; i < LICZBA_PRODUKTOW; i++) {
        shmptr->podajniki[i].poczatek = 0;
        shmptr->podajniki[i].koniec = 0;
        shmptr->podajniki[i].liczba_produktow = 0;
        shmptr->podajniki[i].max_produktow = MAX_PODAJNIK;
    }
}

// Funkcja dodająca produkt na podajnik
int dodaj_na_podajnik(Shared* shmptr, int semid, int produkt_id, int ilosc) {
    sem_op(semid, SEM_CONVEYOR, -1);  // Blokada dostępu do podajnika
    
    int sukces = 0;
    if (shmptr->podajniki[produkt_id].liczba_produktow + ilosc <= MAX_PODAJNIK) {
        for (int i = 0; i < ilosc; i++) {
            shmptr->podajniki[produkt_id].produkty[shmptr->podajniki[produkt_id].koniec] = produkt_id;
            shmptr->podajniki[produkt_id].koniec = (shmptr->podajniki[produkt_id].koniec + 1) % MAX_PODAJNIK;
            shmptr->podajniki[produkt_id].liczba_produktow++;
        }
        sukces = 1;
    }
    
    sem_op(semid, SEM_CONVEYOR, 1);  // Zwolnienie dostępu do podajnika
    return sukces;
}

void bake_products(Shared* shmptr, int semid) {
    srand(time(NULL));

    //losowanie liczby produktow w partii(1-3)
    int liczba_rodzajow = (rand() % MAX_PARTIA) + 1;
    printf("\n=== NOWA PARTIA WYPIEKÓW ===\n");


    //tablica do sledzenia wybranych produktow
    int wybrane_produkty[MAX_PARTIA] = {-1, -1, -1};

    //wybieranie roznych rodzajow produktow
    for (int i=0; i<liczba_rodzajow; i++)
    {
        int produkt_id;
        int jest_unikalny;

        //upewniamy sie ze wybiera unikalny produkt
        do {
            jest_unikalny = 1;
            produkt_id = rand() % LICZBA_PRODUKTOW;

            for (int j=0; j<1; j++)
            {
                if(wybrane_produkty[j] == produkt_id)
                {
                    jest_unikalny = 0;
                    break;
                }
            }
           } while (!jest_unikalny);

           wybrane_produkty[i] = produkt_id;

           //losowa ilosc sztuk produktu (1-5)
           int ilosc = (rand() % 5) + 1;

           printf("Pieczenie: %d sztuk %s\n", ilosc, shmptr->produkty[produkt_id].nazwa);
        
           //proba dodania produktow na podajnik
           if(dodaj_na_podajnik(shmptr, semid, produkt_id, ilosc))
           {
            printf("Dodano na podajnik: %d sztuk %s (ID: %d)\n", ilosc, shmptr->produkty[produkt_id].nazwa, produkt_id);

           } else {
                printf("Podajnik dla %s jest pełny! Nie zmieszczono %d sztuk\n", shmptr->produkty[produkt_id].nazwa, ilosc);

           }
        
    } 
    //staly czas pieczenia 2s
    sleep(2);

}

int main() {
    signal(SIGINT, SIG_IGN);
    initialize_semaphores();
    setup_shared_memory();

    inicjalizuj_produkty(shmptr->produkty);
    inicjalizuj_podajniki(shmptr);

    printf("Piekarz rozpoczyna pracę...\n");


    while(shmptr->is_open)
    {
            bake_products(shmptr, semid);

    }

}






