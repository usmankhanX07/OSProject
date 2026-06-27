#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#include "common.h"
#include "deadlock_algo.h"

int balance[NUM_ACCOUNTS] = {1000, 1000, 1000, 1000, 1000};

pthread_mutex_t account_mutex[NUM_ACCOUNTS];
sem_t           transaction_limit;

typedef struct {
    int thread_id;
    int type;     // 1=deposit  2=withdraw  3=transfer
    int acc1;
    int acc2;
    int amount;
} Transaction;

void log_transaction(int id, char *type, int amount) {
    printf("[Thread %d] %s: %d\n", id, type, amount);
}

void check_total_balance() {
    int total = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        total += balance[i];
    printf(">> Total Bank Balance: %d\n\n", total);
}

//shared functionality
void deposit(int acc, int amount, int tid) {
    pthread_mutex_lock(&account_mutex[acc]);
    balance[acc] += amount;
    log_transaction(tid, "Deposit", amount);
    pthread_mutex_unlock(&account_mutex[acc]);
}

void withdraw(int acc, int amount, int tid) {
    pthread_mutex_lock(&account_mutex[acc]);
    if (amount > balance[acc]) {
        printf("[Thread %d] Withdraw failed (Insufficient funds)\n", tid);
    } else {
        balance[acc] -= amount;
        log_transaction(tid, "Withdraw", amount);
    }
    pthread_mutex_unlock(&account_mutex[acc]);
}

void transfer(int from, int to, int amount, int tid) {
    pthread_mutex_lock(&account_mutex[from]);
    pthread_mutex_lock(&account_mutex[to]);

    if (balance[from] >= amount) {
        balance[from] -= amount;
        balance[to]   += amount;
        log_transaction(tid, "Transfer", amount);
    } else {
        printf("[Thread %d] Transfer failed (Insufficient funds)\n", tid);
    }

    pthread_mutex_unlock(&account_mutex[from]);
    pthread_mutex_unlock(&account_mutex[to]);
}

void displaybalance() {
    printf("---- Account Balances ----\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        printf("Account %d: %d\n", i, balance[i]);
    printf("--------------------------\n");
}

void *client_thread(void *arg) {
    Transaction *t = (Transaction *)arg;

    int req[NUM_ACCOUNTS] = {0};
    req[t->acc1] = 1;
    if (t->type == 3)
        req[t->acc2] = 1;

    sem_wait(&transaction_limit);   

    while (!algo_request(t->thread_id, req)) {
        printf("[Thread %d] Request denied by algo — retrying...\n",
               t->thread_id);
        sleep(1);
    }

    printf("[Thread %d] Started\n", t->thread_id);

    if      (t->type == 1) deposit (t->acc1, t->amount, t->thread_id);
    else if (t->type == 2) withdraw(t->acc1, t->amount, t->thread_id);
    else                   transfer(t->acc1, t->acc2, t->amount, t->thread_id);

    check_total_balance();
    printf("[Thread %d] Finished\n\n", t->thread_id);

    int rel[NUM_ACCOUNTS] = {0};
    rel[t->acc1] = 1;
    if (t->type == 3)
        rel[t->acc2] = 1;

    algo_release(t->thread_id, rel);
    algo_print_state();

    sem_post(&transaction_limit);

    return NULL;
}

int main() {
    pthread_t   threads[NUM_THREADS];
    Transaction t[NUM_THREADS];

    sem_init(&transaction_limit, 0, 5);

    for (int i = 0; i < NUM_ACCOUNTS; i++)
        pthread_mutex_init(&account_mutex[i], NULL);

    printf("Enter transactions for %d threads:\n", NUM_THREADS);
    printf("Type: 1=Deposit  2=Withdraw  3=Transfer\n\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        t[i].thread_id = i;
        t[i].acc2      = -1;

        printf("Thread %d:\n", i);
        printf("  Enter type   : "); scanf("%d", &t[i].type);
        printf("  Enter amount : "); scanf("%d", &t[i].amount);
        printf("  Enter account#(0-4): "); scanf("%d", &t[i].acc1);
        if (t[i].type == 3) {
            printf("  Enter acc2   : "); scanf("%d", &t[i].acc2);
        }
        printf("\n");
    }

    int total[NUM_ACCOUNTS];
    for (int j = 0; j < NUM_ACCOUNTS; j++)
        total[j] = 1;

    int max_claims[NUM_THREADS][NUM_ACCOUNTS];
    for (int i = 0; i < NUM_THREADS; i++)
        for (int j = 0; j < NUM_ACCOUNTS; j++)
            max_claims[i][j] = (t[i].acc1 == j ||
                                 t[i].acc2 == j) ? 1 : 0;

    algo_init(total, max_claims);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, client_thread, &t[i]);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    displaybalance();

    sem_destroy(&transaction_limit);
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        pthread_mutex_destroy(&account_mutex[i]);

    return 0;
}
