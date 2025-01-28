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


#define NUM_PRODUCTS 15 //Liczba produtków P
#define MAX_CUSTOMERS 30 //Liczba klientów  wsklepie N
#define NUM_CASHIER 3 // Liczba kas
#define MIN_PRODUCTS 2 //Minimalna liczba produktów klienta
#define MAX_CONVEYOR_ITEMS 50 //Pojemność podajnika Ki
#define OPENING_HOUR 6 //Tp
#define CLOSING_HOUR 21 //Tk

#define SHM_KEY 12345
#define SEM_KEY 56791
#define MSG_KEY 96324

#define SEM_ENTRANCE 0 //wejscie do sklepu
#define SEM_CASHIER_1 1 //kasa1
#define SEM_CASHIER_2 2 //kasa2
#define SEM_CASHIER_3 3 //kasa3
#define SEM_CONVEYOR 4 //podajniki
#define TOTAL_SEMS 5

typedef enum {
    BULKA_KAJZERKA,
    BULKA_GRAHAMKA,
    CHLEB_PSZENNY,
    CHLEB_PELNOZIARNISTY,
    CHLEB_ZYTNI,
    BAGIETKA,
    CHLEB_NA_ZAKWASIE,
    PIECZYWO_BEZGLUT,
    PACZEK,
    ROGALIK,
    CIASTKO_KRUCHE,
    STRUCLA,
    ZAPIEKANKA,
    FOCACCIA,
    ROGAL_SWIETOMARCINSKI
    
} ProductType;

static const double PRODUCT_PRICES[] = {
    3.0,
    4.0,
    6.0,
    7.0,
    7.0,
    4.0,
    8.0,
    7.0,
    3.0,
    4.0,
    2.0,
    3.0,
    7.0,
    7.0,
    7.0,
    6.0

};

static const char* const PRODUCT_NAMES[NUM_PRODUCTS] = {
    "Bułka kajzerka",
    "Bułka grahamka", 
    "Chleb pszenny",
    "Chleb pełnoziarnisty",
    "Chleb żytni",
    "Bagietka",
    "Chleb na zakwasie",
    "Pieczywo bezglutenowe",
    "Pączek",
    "Rogalik",
    "Ciastko kruche",
    "Strucla",
    "Zapiekanka",
    "Focaccia",
    "Rogal świętomarciński"
};



//struktury komunikatow
typedef struct {
    long mtype;
    int products[NUM_PRODUCTS];
    pid_t customer_pid;

} BakeryOrder;


typedef struct {
    long mtype;
    double total_price;
    int products_sold[NUM_PRODUCTS];
} BakeryReceipt;

// struktura pamieci wspoldzielonej
typedef struct {
    int is_open;
    int inventory_mode;
    int evacuation_mode;
    int customers_inside;
    int active_registers;
    int conveyor_items[NUM_PRODUCTS];
    int total_produced[NUM_PRODUCTS];
    int total_sold[NUM_PRODUCTS];
    double product_prices[NUM_PRODUCTS];

} Shared;

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