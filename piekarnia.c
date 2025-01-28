#include "default.h"


int msgid;
int semid;
int shmid;
Shared *shmptr;
pid_t pid_kierownik, pid_piekarz, pid_klient, pid_kasjer[NUM_CASHIER];



void setup_shared_memory()
{
    shmid = shmget(SHM_KEY, sizeof(Shared), IPC_CREAT | 0600);
    if (shmid == -1)
    {
        perror("Błąd podczas tworzenia segmentu pamięci wspoldzielonej");
        exit(EXIT_FAILURE);

    }

    shmptr = (Shared *)shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1)
    {
        perror("Błąd podczas dołączania segmentu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

}



void initialize_message_queue()
{
    msgid = msgget(MSG_KEY, IPC_CREAT | 0600);
    if (msgid == -1)
    {
        perror("Błąd podczas tworzenia kolejki komunikatów");
        exit(EXIT_FAILURE);
    }
}

void initialize_semaphores()
{
    //towrzenie zestawu semaforów
    semid = semget(SEM_KEY, TOTAL_SEMS, IPC_CREAT | 0600);
    if (semid == -1)
    { 
        perror("Błąd podczas tworzenia semaforów");
        exit(EXIT_FAILURE);
    }

    int setup_fail = 0;
    setup_fail |= semctl(semid, SEM_ENTRANCE, SETVAL, 1);
    setup_fail |= semctl(semid, SEM_CASHIER_1, SETVAL, 1);
    setup_fail |= semctl(semid, SEM_CASHIER_2, SETVAL, 1);
    setup_fail |= semctl(semid, SEM_CASHIER_3, SETVAL, 1);
    setup_fail |= semctl(semid, SEM_CONVEYOR, SETVAL, 1);

    if (setup_fail == -1)
    {
        perror("Błąd podczas inicjalizacji semaforow");
        exit(EXIT_FAILURE);
    }
}



int is_bakery_open()
{ 
    time_t timestamp;
    struct tm *time_info;
    time(&timestamp);
    time_info = localtime(&timestamp);
    
    if (time_info->tm_hour >= OPENING_HOUR && time_info->tm_hour < CLOSING_HOUR)
    {
        return 1;
    }
    
    return 0;
}

void cleanup_resources()
{
    if(shmdt(shmptr) == -1)
    {
        perror("Błąd podczas odłączania pamięci wspoldzielonej");

    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("Błąd podczas usuwania kolejki komunikatów");
    }

    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("Błąd podczas usuwania semaforów");

    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("Błąd podcas usuwania segmentu pamięci współdzielonej");
    }
}

void handle_signal(int sig_num)
{
    if (sig_num == SIGINT)
    {
        printf("\nRozpoczęto procedurę zamykania piekarni...\n");
        shmptr->is_open = 0;

        //czekamy az wszyscy klienci opuszcza sklep
        while(shmptr->customers_inside > 0)
        {
            printf("W sklepie pozostało jeszcze %d kleintów...\n", shmptr->customers_inside);
            sleep(1);
        }

        //zakonczenie dzialania procesow pracownikow i klientow
        kill(pid_kierownik, SIGTERM);
        kill(pid_piekarz, SIGTERM);
        kill(pid_klient, SIGTERM);

        //zakoncz prcesy kasjerow
        for(int kasa=0; kasa < NUM_CASHIER; kasa++)
        {
            kill(pid_kasjer[kasa], SIGTERM);
        }

        waitpid(pid_kierownik, NULL, 0);
        waitpid(pid_piekarz, NULL, 0);
        waitpid(pid_klient, NULL, 0);

        for(int kasa=0; kasa<NUM_CASHIER; kasa++)
        {
            waitpid(pid_kasjer[kasa], NULL, 0);
        }
        cleanup_resources();
        exit(0);
    }
}



int main() {
    srand(time(NULL));
    signal(SIGINT, handle_signal);

    //inicjalizacja ipc
    initialize_message_queue();
    initialize_semaphores();
    setup_shared_memory();

    shmptr->is_open = 1;
    shmptr->inventory_mode = 0;
    shmptr->evacuation_mode = 0;
    shmptr->customers_inside = 0;
    shmptr->active_registers = 1;

    //inicjalizacja cen produktów
    for(int i=0; i<NUM_PRODUCTS; i++)
    {
        shmptr->product_prices[i] = PRODUCT_PRICES[i];
        shmptr->conveyor_items[i] = 0;
        shmptr->total_produced[i] = 0;
        shmptr->total_sold[i] = 0;

    }

    //Uruchamianie procesów
    pid_kierownik = fork();
    if (pid_kierownik == 0)
    {
        execl("./kierownik", "kierownik", NULL);
        perror("Nie udało się uruchomić procesu kierownika");
        exit(EXIT_FAILURE);

    }
    
    pid_piekarz = fork();
    if (pid_piekarz == 0)
    {
        execl("./piekarz", "piekarz", NULL);
        perror("Nie udało się uruchomić procesu piekarza");
        exit(EXIT_FAILURE);
    }

    //uruchomienie kas
    for (int i=0; i<NUM_CASHIER; i++)
    {
        pid_kasjer[i] = fork();
        if (pid_kasjer[i] == 0)
        {
            char regs[2];
            sprintf(regs, "%d", i+1);
            execl("./kasjer", "kasjer", regs, NULL);
            perror("Nie udało się uruchomić procesu kasjera");
            exit(EXIT_FAILURE);
        }
    }

    pid_klient = fork();
    if (pid_klient == 0)
    {
        execl("./klient", "klient", NULL);
        perror("Nie udało się uruchomić procesu klienta");
        exit(EXIT_FAILURE);
    
    }

    //pętla kontrolna
    while (shmptr->is_open)
    {
        if (!is_bakery_open())
        {
            printf("Godziny pracy piekarni dobiegają końca: (%d:00). Zamykanie...\n", CLOSING_HOUR);
            kill(getpid(), SIGINT);
        }
        sleep(60);
    }

    return 0;
    
}