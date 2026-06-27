#include "deadlock_algo.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

static int available [NUM_ACCOUNTS];
static int max_claim [NUM_THREADS][NUM_ACCOUNTS];
static int allocation[NUM_THREADS][NUM_ACCOUNTS];
static int need      [NUM_THREADS][NUM_ACCOUNTS];

static pthread_mutex_t banker_lock = PTHREAD_MUTEX_INITIALIZER;

static int is_safe_state(void) {
    int work  [NUM_ACCOUNTS];
    int finish[NUM_THREADS];
    memcpy(work,   available, sizeof(work));
    memset(finish, 0, sizeof(finish));

    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (finish[i]) continue;

            int can_finish = 1;
            for (int j = 0; j < NUM_ACCOUNTS; j++)
                if (need[i][j] > work[j]) { can_finish = 0; break; }

            if (can_finish) {
                for (int j = 0; j < NUM_ACCOUNTS; j++)
                    work[j] += allocation[i][j];
                finish[i] = 1;
                progress  = 1;
            }
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
        if (!finish[i]) return 0;   /* unsafe */
    return 1;                        /* safe   */
}

void algo_init(int total[NUM_ACCOUNTS],
               int max_per_thread[NUM_THREADS][NUM_ACCOUNTS]) {
    pthread_mutex_lock(&banker_lock);

    memcpy(available, total, sizeof(available));
    memset(allocation, 0,    sizeof(allocation));

    for (int i = 0; i < NUM_THREADS; i++)
        for (int j = 0; j < NUM_ACCOUNTS; j++) {
            max_claim[i][j] = max_per_thread[i][j];
            need     [i][j] = max_per_thread[i][j];
        }

    pthread_mutex_unlock(&banker_lock);
    printf("[Banker] Initialised. Accounts: %d  Threads: %d\n",
           NUM_ACCOUNTS, NUM_THREADS);
}

/* ── Request ─────────────────────────────────────────────── */
int algo_request(int tid, int request[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&banker_lock);

    /* 1. Request must not exceed need */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (request[j] > need[tid][j]) {
            printf("[Banker] T%d: request exceeds max claim on acc%d. Denied.\n",
                   tid, j);
            pthread_mutex_unlock(&banker_lock);
            return 0;
        }
    }

    /* 2. Request must not exceed available */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (request[j] > available[j]) {
            printf("[Banker] T%d: acc%d not available. Must wait.\n", tid, j);
            pthread_mutex_unlock(&banker_lock);
            return 0;
        }
    }

    /* 3. Tentatively allocate, then safety check */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        available    [j]     -= request[j];
        allocation   [tid][j] += request[j];
        need         [tid][j] -= request[j];
    }

    if (!is_safe_state()) {
        /* Roll back */
        for (int j = 0; j < NUM_ACCOUNTS; j++) {
            available    [j]     += request[j];
            allocation   [tid][j] -= request[j];
            need         [tid][j] += request[j];
        }
        printf("[Banker] T%d: unsafe state. Request denied.\n", tid);
        pthread_mutex_unlock(&banker_lock);
        return 0;
    }

    printf("[Banker] T%d: request granted.\n", tid);
    pthread_mutex_unlock(&banker_lock);
    return 1;
}

/* ── Release ─────────────────────────────────────────────── */
void algo_release(int tid, int release[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&banker_lock);

    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        allocation[tid][j] -= release[j];
        need      [tid][j] += release[j];
        available [j]      += release[j];
    }

    printf("[Banker] T%d: resources released.\n", tid);
    pthread_mutex_unlock(&banker_lock);
}

/* ── Print State ─────────────────────────────────────────── */
void algo_print_state() {
    pthread_mutex_lock(&banker_lock);

    printf("\n── Banker State ───────────────────────────────────────────\n");

    /* Available row */
    printf("Available:  ");
    for (int j = 0; j < NUM_ACCOUNTS; j++)
        printf("[Acc%d:%d] ", j, available[j]);
    printf("\n\n");

    /* Per-thread table */
    printf("%-8s  %-25s  %-25s  %-25s\n",
           "Thread", "Allocation", "Max", "Need");
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("T%-7d  ", i);
        for (int j = 0; j < NUM_ACCOUNTS; j++) printf("%d ", allocation[i][j]);
        printf("  %*s", (int)(25 - NUM_ACCOUNTS * 2), "");
        for (int j = 0; j < NUM_ACCOUNTS; j++) printf("%d ", max_claim[i][j]);
        printf("  %*s", (int)(25 - NUM_ACCOUNTS * 2), "");
        for (int j = 0; j < NUM_ACCOUNTS; j++) printf("%d ", need[i][j]);
        printf("\n");
    }

    printf("───────────────────────────────────────────────────────────\n\n");
    pthread_mutex_unlock(&banker_lock);
}
