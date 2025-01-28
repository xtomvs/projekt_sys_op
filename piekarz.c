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


void bake_products() {
    sem_op(semid, SEM_CONVEYOR, -1);

    int num_products = 1 + rand() % 5;
    printf("\nPiekarz: Rozpoczynam nową partię produkcji (%d rodzajów)\n", num_products);

    for(int i=0; i<num_products; i++)
    {
        //losowy produkt
        ProductType product = rand() % NUM_PRODUCTS;

        //sprawdzamy czy jest miejsce na podajniku
        if(shmptr->conveyor_items[product] < MAX_CONVEYOR_ITEMS)
        {
            int quantity = 1+ rand() % 8;

            if(shmptr->conveyor_items[product] + quantity > MAX_CONVEYOR_ITEMS)
            {
                quantity = MAX_CONVEYOR_ITEMS - shmptr->conveyor_items[product];
            }

            if(quantity>0)
            {
                shmptr->conveyor_items[product] += quantity;
                shmptr->total_produced[product] += quantity;
                printf("Piekarz: Wyprodukowano %d szt. %s (%.2f zł/szt\n)", quantity, PRODUCT_NAMES[product], PRODUCT_PRICES[product]);
            }
        }
    }
}

int main() {
    signal(SIGINT, SIG_IGN);
    initialize_semaphores();
    setup_shared_memory();

    while(shmptr->is_open)
    {
        if(!shmptr->evacuation_mode && !shmptr->inventory_mode)
        {
            bake_products();
        }

        sleep(2 + rand() % 4);
    }

}






