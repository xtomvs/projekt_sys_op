#ifndef DEFAULT_H
#define DEFAULT_H

#include <stdio.h>    
#include <stdlib.h>  
#include <unistd.h>   
#include <string.h> 
#include <signal.h>   
#include <errno.h> 
#include <sys/ipc.h>  
#include <sys/msg.h>  
#include <sys/sem.h>  
#include <sys/shm.h>  
#include <sys/wait.h> 
#include <time.h>     


#define LICZBA_PRODUKTOW 15 //Liczba produtków P
#define MAX_CUSTOMERS 9 //Liczba klientów  wsklepie N
#define NUM_CASHIER 3 // Liczba kas
#define MAX_PODAJNIK 20 //Pojemność podajnika Ki
#define OPENING_HOUR 6 //Tp
#define CLOSING_HOUR 20 //Tk
#define MAX_PARTIA 3

#define SHM_KEY 12345
#define SEM_KEY 12346
#define MSG_KEY 96324

#define SEM_ENTRANCE 0 //wejscie do sklepu
#define SEM_CASHIER_1 1 //kasa1
#define SEM_CASHIER_2 2 //kasa2
#define SEM_CASHIER_3 3 //kasa3
#define SEM_CONVEYOR 4 //podajniki
#define TOTAL_SEMS 5

// Struktura reprezentująca produkt
typedef struct {
    char nazwa[50];
    float cena;
} Produkt;

// Struktura reprezentująca podajnik
typedef struct {
    int produkty[MAX_PODAJNIK];
    int poczatek;
    int koniec;
    int liczba_produktow;
    int max_produktow;
} Podajnik;

// struktura pamieci wspoldzielonej
typedef struct {
    int is_open;
    Podajnik podajniki[LICZBA_PRODUKTOW];
    Produkt produkty[LICZBA_PRODUKTOW];
} Shared;

void inicjalizuj_produkty(Produkt* produkty) {
    strcpy(produkty[0].nazwa, "Bułka kajzerka");
    produkty[0].cena = 3.0;
    strcpy(produkty[1].nazwa, "Bułka grahamka");
    produkty[1].cena = 4.0;
    strcpy(produkty[2].nazwa, "Chleb pszenny");
    produkty[2].cena = 6.0;
    strcpy(produkty[3].nazwa, "Chleb pełnoziarnisty");
    produkty[3].cena = 7.0;
    strcpy(produkty[4].nazwa, "Chleb żytni");
    produkty[4].cena = 8.0;
    strcpy(produkty[5].nazwa, "Bagietka");
    produkty[5].cena = 9.0;
    strcpy(produkty[6].nazwa, "Chleb na zakwasie");
    produkty[6].cena = 10.0;
    strcpy(produkty[7].nazwa, "Pieczywo bezglutenowe");
    produkty[7].cena = 11.0;
    strcpy(produkty[8].nazwa, "Pączek");
    produkty[8].cena = 2.0;
    strcpy(produkty[9].nazwa, "Rogalik");
    produkty[9].cena = 12.0;
    strcpy(produkty[10].nazwa, "Ciastko kruche");
    produkty[10].cena = 1.0;
    strcpy(produkty[11].nazwa, "Strucla");
    produkty[11].cena = 13.0;
    strcpy(produkty[12].nazwa, "Zapiekanka");
    produkty[12].cena = 14.0;
    strcpy(produkty[13].nazwa, "Focaccia");
    produkty[13].cena = 15.0;
    strcpy(produkty[14].nazwa, "Rogal świętomarciński");
    produkty[14].cena = 16.0;
}



//funkcja do operacji na semaforach
void sem_op(int semid, int sem_num, int op) {
    struct sembuf sb = {sem_num, op, 0};
    if (semop(semid, &sb, 1) == -1) {
        if(errno == EINTR) {
            sem_op(semid, sem_num, op);
        } else {
            perror("Błąd operacji na semaforze");
            exit(EXIT_FAILURE);
        }
    }
}


#endif