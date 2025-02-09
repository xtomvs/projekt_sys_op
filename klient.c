#include "default.h"

int semid;
int shmid;
int msgid;
Shared *shmptr;

void initialize_ipc() {
    // Initialize semaphores
    if ((semid = semget(SEM_KEY, TOTAL_SEMS, 0600)) == -1) {
        perror("Semaphore access error");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    if ((shmid = shmget(SHM_KEY, sizeof(Shared), 0600)) == -1) {
        perror("Shared memory access error");
        exit(EXIT_FAILURE);
    }
    
    if ((shmptr = (Shared *)shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("Shared memory mapping error");
        exit(EXIT_FAILURE);
    }

    // Initialize message queue
    if ((msgid = msgget(MSG_KEY, 0600)) == -1) {
        perror("Message queue access error");
        exit(EXIT_FAILURE);
    }
}

void cleanup_customer(pid_t pid) {
    sem_op(semid, SEM_ENTRANCE, -1);
    shmptr->customers_inside--;
    sem_op(semid, SEM_ENTRANCE, 1);
    printf("Customer %d: Leaving the store (remaining customers: %d)\n", pid, shmptr->customers_inside);
}

void handle_signal(int sig) {
    if (sig == SIGCHLD) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }
}

void customer_simulation(pid_t pid) {
    // Try to enter the store
    while (shmptr->is_open && !shmptr->evacuation_mode) {
        sem_op(semid, SEM_ENTRANCE, -1);
        
        if (shmptr->customers_inside < MAX_CUSTOMERS) {
            shmptr->customers_inside++;
            sem_op(semid, SEM_ENTRANCE, 1);
            printf("\nCustomer %d: Entered the store (customers inside: %d)\n", pid, shmptr->customers_inside);
            break;
        }
        
        sem_op(semid, SEM_ENTRANCE, 1);
        printf("Customer %d: Waiting to enter...\n", pid);
        sleep(1);
    }

    if (!shmptr->is_open || shmptr->evacuation_mode) {
        printf("Customer %d: Store closed/evacuation - leaving\n", pid);
        return;
    }

    // Create shopping list
    BakeryOrder order;
    order.customer_pid = pid;
    memset(order.products, 0, sizeof(order.products));
    
    int unique_products = MIN_PRODUCTS + rand() % 3;
    printf("\nCustomer %d: Creating shopping list (%d products)\n", pid, unique_products);
    
    int selected_count = 0;
    while (selected_count < unique_products) {
        int product = rand() % NUM_PRODUCTS;
        if (order.products[product] == 0) {
            order.products[product] = 1 + rand() % 3;
            printf("- %s: %d pcs\n", PRODUCT_NAMES[product], order.products[product]);
            selected_count++;
        }
    }

    // Choose register
    if (shmptr->active_registers < 1) {
        printf("Customer %d: No active registers, leaving\n", pid);
        goto cleanup;
    }
    
    int chosen_register = (rand() % shmptr->active_registers) + 1;
    order.mtype = chosen_register;
    printf("Customer %d: Choosing register %d\n", pid, chosen_register);

    // Send order to register
    if (msgsnd(msgid, &order, sizeof(BakeryOrder) - sizeof(long), 0) == -1) {
        perror("Error sending order");
        goto cleanup;
    }

    // Wait for receipt
    BakeryReceipt receipt;
    if (msgrcv(msgid, &receipt, sizeof(BakeryReceipt) - sizeof(long), pid, 0) == -1) {
        perror("Error receiving receipt");
        goto cleanup;
    }

    printf("Customer %d: Paid %.2f PLN\n", pid, receipt.total_price);
    
cleanup:
    cleanup_customer(pid);
}

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handle_signal);
    srand(time(NULL));
    
    initialize_ipc();

    while (shmptr->is_open) {
        pid_t child_pid = fork();
        
        if (child_pid == 0) {
            srand(time(NULL) ^ getpid());
            customer_simulation(getpid());
            exit(0);
        } else if (child_pid > 0) {
            sleep(2 + rand() % 5);
        }
    }

    return 0;
}