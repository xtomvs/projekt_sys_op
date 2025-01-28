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

void manage_registers()
{
    int customers = shmptr->customers_inside;
    int registers = (customers + MAX_CUSTOMERS/3 -1) / (MAX_CUSTOMERS/3);

    //minimum jedna kasa, maks 3
    registers = registers < 1 ? 1 : registers;
    registers = registers > 3 ? 3 : registers;

    if (customers < (2 * MAX_CUSTOMERS / 3) && shmptr->active_registers > 1)
    {
        printf("Kierownik: zmniejszam liczbę aktywnych kas do %d\n", registers);
        shmptr->active_registers = registers;
    } else if (customers >= (2 * MAX_CUSTOMERS / 3) && shmptr->active_registers < 3) {
        //jeśli dużo klientów, otwieramy więcej kas
        printf("Kierownik: Zwiększam liczbę aktywnych kas do %d\n", registers);
        shmptr->active_registers = registers;
    }
}

void print_summary()
{
    printf("\n==== Podsumowanie stanu piekarni ====\n");

    printf("Liczba klientów w sklepie: %d\n", shmptr->customers_inside);

    printf("Aktywne kasy: %d\n", shmptr->active_registers);
    
    printf("\nStan podajników:\n");
    for(int i = 0; i < NUM_PRODUCTS; i++) {
        printf("%s: %d szt. (cena: %.2f zł)\n", 
               PRODUCT_NAMES[i], shmptr->conveyor_items[i], shmptr->product_prices[i]);
    }

    printf("\nStatystyki produkcji i sprzedaży:\n");
    for(int i = 0; i < NUM_PRODUCTS; i++) {
        printf("%s:\n", PRODUCT_NAMES[i]);
        printf("  - Wyprodukowano: %d szt.\n", shmptr->total_produced[i]);
        printf("  - Sprzedano: %d szt.\n", shmptr->total_sold[i]);
        printf("  - Na stanie: %d szt.\n", shmptr->conveyor_items[i]);
    }
    printf("=======================================\n\n");
}

int main() {

    signal(SIGINT, SIG_IGN);
    initialize_semaphores();
    setup_shared_memory();

    while(shmptr->is_open)
    {
        manage_registers();
    

        if(rand() % 1000 < 2)
        {
            if(rand() %2)
            {
                printf("Kierownik: Ogłaszam ewakuację!\n");
                shmptr->evacuation_mode = 1;
                sleep(5); 
                shmptr->evacuation_mode = 0;
                printf("Kierownik: Koniec ewakuacji.\n");
            } else {
                printf("Kierownik: Rozpoczynam inwentaryzację!\n");
                    shmptr->inventory_mode = 1;
                    sleep(10);  
                    shmptr->inventory_mode = 0;
                    print_summary();
                    printf("Kierownik: Koniec inwentaryzacji.\n");

            }
        }

        sleep(1);
    } 

    print_summary();
    return 0;



}
