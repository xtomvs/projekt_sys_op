#include "default.h"

int semid;
int shmid;
int msgid;
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

void initialize_message_queue()
{
    msgid = msgget(MSG_KEY, 0600);
    if (msgid == -1)
    {
        perror("Błąd podczas tworzenia kolejki komunikatów");
        exit(EXIT_FAILURE);
    }
}

void handle_signal(int sig)
{
    if(sig == SIGCHLD)
    {
        while(waitpid(-1, NULL, WNOHANG) > 0);
    }
}

void customer_simulation(pid_t pid)
{

      //wejscie do sklepu
    while(shmptr->is_open && !shmptr->evacuation_mode)
    {
        sem_op(semid, SEM_ENTRANCE, -1);

        if(shmptr->customers_inside < MAX_CUSTOMERS)
        {
            shmptr->customers_inside++;
            sem_op(semid, SEM_ENTRANCE, 1);
            printf("\nKlient %d wszedł do sklepu (w środku jest %d klientów)\n", pid, shmptr->customers_inside);
            break;
        }

        sem_op(semid, SEM_ENTRANCE, 1);
        printf("Klient %d czeka na wejście...\n", pid);
        sleep(1);
    }

    // jesli piekarnia zamknieta lub ewakuacja klient wychodzi
    if(!shmptr->is_open || shmptr->evacuation_mode) 
    {
        printf("Klient %d: Sklep zamkniety/ewakuacja - wychodzę\n", pid);
        return;

    }

    BakeryOrder order;
    order.mtype = 1;
    order.customer_pid = pid;

    int num_products = MIN_PRODUCTS + rand() % 3;
    memset(order.products, 0, sizeof(order.products));

    printf("\nKlient %d tworzy liste zakupów (%d produktow)\n", pid, num_products);

    for(int i=0; i<num_products; i++)
    {
        int product;
        do
        {
            product = rand() % NUM_PRODUCTS;
        } while(order.products[product]>0);

        order.products[product] = 1 + rand() % 3;
        printf("- %s: %d szt.\n", PRODUCT_NAMES[product], order.products[product]);

        
    }


    // wybor kasy
    int chosen_register = (rand() % shmptr->active_registers) + 1;
    order.mtype = chosen_register;

    printf("Klient %d wybiera kasę %d\n", pid, chosen_register);

    // wyslanie zamowienia do kasy
    if(msgsnd(msgid, &order, sizeof(BakeryOrder) - sizeof(long), 0) == -1) 
    {
        perror("Błąd podczas składania zamówienia");

        goto cleanup;
    }

    // oczekiwanie na realizacje
    BakeryReceipt receipt;
    if(msgrcv(msgid, &receipt, sizeof(BakeryReceipt) - sizeof(long), pid, 0) == -1) 
    {
        perror("Błąd podczas odbierania paragonu");
        goto cleanup;
    }


    printf("Klient %d: Zapłacono %.2f zł\n", pid, receipt.total_price);

    sem_op(semid, SEM_ENTRANCE, -1);
    shmptr->customers_inside--;
    sem_op(semid, SEM_ENTRANCE, 1);
    printf("Klient %d: Zakupy zakończone, wychodzę (pozostało klientów: %d)\n", 
       pid, shmptr->customers_inside);
    return;

cleanup:
    //wyjscie ze sklepu
    sem_op(semid, SEM_ENTRANCE, -1);
    shmptr->customers_inside--;
    sem_op(semid, SEM_ENTRANCE, 1);
    
    printf("Klient %d opuszcza sklep (pozostało klientów: %d)\n", pid, shmptr->customers_inside);


}

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handle_signal);

    srand(time(NULL));

    initialize_message_queue();
    initialize_semaphores();
    setup_shared_memory();

    while(shmptr->is_open)
    {
        pid_t child_pid = fork();

        if(child_pid == 0)
        {
            srand(time(NULL) ^ getpid());
            customer_simulation(getpid());
            exit(0);
        } 
        else if (child_pid > 0)
        {
            //czaspomiedzy pojawieniem sie nowych klientow
            sleep(2 + rand() % 5);
        }
    }

    return 0;
}