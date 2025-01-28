#include "default.h"


int semid;
int shmid;
int msgid;
Shared *shmptr;
int register_number;
int products_sold[NUM_PRODUCTS];

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

void initialize_message_queue()
{
    msgid = msgget(MSG_KEY, 0600);
    if (msgid == -1)
    {
        perror("Błąd podczas tworzenia kolejki komunikatów");
        exit(EXIT_FAILURE);
    }
}

void print_receipt(BakeryOrder *ord, double total)
{
    printf("\n=== PARAGON - KASA &d ===\n", register_number);
    for(int i=0; i<NUM_PRODUCTS; i++)
    {
        if(ord->products[i]>0)
        {
            printf("%s: %d szt. x %.2f zł = %.2f zł\n",
            PRODUCT_NAMES[i], ord->products[i], shmptr->product_prices[i], ord->products[i] * shmptr->product_prices[i]);
        }
    }
    printf("SUMA: %.2f zł\n", total);
    printf("===========================\n");
}


int main(int argc, char *argv[]) {

    if(argc != 2)
    {
        fprintf(stderr, "Użycie: %s <numer_kasy>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    register_number = atoi(argv[1]);
    signal(SIGINT, SIG_IGN);
    initialize_message_queue();
    initialize_semaphores();
    setup_shared_memory();

    printf("Kasjer %d: Rozpoczynam pracę\n", register_number);

    while(shmptr->is_open)
    {
        if(register_number <= shmptr->active_registers)
        {
            BakeryOrder ord;
            BakeryReceipt rec;

            if(msgrcv(msgid, &ord, sizeof(BakeryOrder) - sizeof(long), register_number, IPC_NOWAIT) == -1)
            {
                if(errno = ENOMSG)
                {
                    usleep(100000);
                    continue;
                }
                perror("Błąd odbioru zamówienia");
                continue;
            }

            printf("\nKasjer %d: Obsluguę klienta %d\n", register_number, ord.customer_pid);

            double total = 0.0;
            sem_op(semid, SEM_CONVEYOR, -1);

            for (int i=0; i<NUM_PRODUCTS; i++)
            {
                if(ord.products[i]>0)
                {
                    if(shmptr->conveyor_items[i] >= ord.products[i])
                    {
                        total += shmptr->product_prices[i] * ord.products[i];
                        shmptr->conveyor_items[i] -= ord.products[i];
                        shmptr->total_sold[i] += ord.products[i];
                        products_sold[i] += ord.products[i];

                    } else{
                        ord.products[i] = 0;
            
                    }
                }
            }

            sem_op(semid, SEM_CONVEYOR, 1);

            print_receipt(&ord, total);

            rec.mtype = ord.customer_pid;
            rec.total_price = total;
            memcpy(rec.products_sold, ord.products, sizeof(ord.products));

            if(msgsnd(msgid, &rec, sizeof(BakeryReceipt) - sizeof(long), 0) == -1)
            {
                perror("Błąd wysyłania odpowiedzi");

            }


        } else{
            //kasa nieaktywna
            usleep(500000);
        }
    }

    printf("\n=== Podsumowanie sprzedaży - Kasa %d ===\n", register_number);
    for(int i=0; i<NUM_PRODUCTS; i++)
    {
        if(products_sold[i]>0)
        {
            printf("Produkt %d: Sprzedano %d szt. (%.2f zł/szt)\n", i+1, products_sold[i], shmptr->product_prices[i]);

        }
    }

    printf("=================================\n");

    return 0;


}