#include "deadlock_algo.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* ── State ───────────────────────────────────────────────────
 * timestamp[t]  — assigned at init; lower = older = higher priority
 * holder[r]     — which thread currently holds account r (-1 if free)
 * ─────────────────────────────────────────────────────────── */
static int timestamp[NUM_THREADS];
static int holder   [NUM_ACCOUNTS];

static pthread_mutex_t wd_lock = PTHREAD_MUTEX_INITIALIZER;

/* ── Init ────────────────────────────────────────────────── */
void algo_init(int total[NUM_ACCOUNTS],
               int max_per_thread[NUM_THREADS][NUM_ACCOUNTS]) {

    pthread_mutex_lock(&wd_lock);

    for (int i = 0; i < NUM_THREADS; i++)
        timestamp[i] = i;           /* T0 is oldest, T9 is youngest */

    for (int j = 0; j < NUM_ACCOUNTS; j++)
        holder[j] = -1;             /* all accounts free at start */

    (void)total;
    (void)max_per_thread;           /* Wait-Die needs no max claims */

    pthread_mutex_unlock(&wd_lock);
    printf("[Wait-Die] Initialised. Accounts: %d  Threads: %d\n",
           NUM_ACCOUNTS, NUM_THREADS);
}

/* ── Request ─────────────────────────────────────────────── */
int algo_request(int tid, int req[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&wd_lock);

    /* Check every account this thread needs */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (!req[j]) continue;

        int h = holder[j];

        if (h == -1) continue;      /* account free — no conflict */

        /* Conflict: account j is held by thread h */
        if (timestamp[tid] < timestamp[h]) {
            /* T is older than holder → WAIT */
            printf("[Wait-Die] T%d (ts=%d) WAITS for acc%d held by T%d (ts=%d)\n",
                   tid, timestamp[tid], j, h, timestamp[h]);
            pthread_mutex_unlock(&wd_lock);
            return 0;
        } else {
            /* T is younger than holder → DIE */
            printf("[Wait-Die] T%d (ts=%d) DIES — acc%d held by older T%d (ts=%d). Will retry.\n",
                   tid, timestamp[tid], j, h, timestamp[h]);
            pthread_mutex_unlock(&wd_lock);
            return 0;
        }
    }

    /* No conflicts — assign all requested accounts */
    for (int j = 0; j < NUM_ACCOUNTS; j++)
        if (req[j]) holder[j] = tid;

    printf("[Wait-Die] T%d: request granted.\n", tid);
    pthread_mutex_unlock(&wd_lock);
    return 1;
}

/* ── Release ─────────────────────────────────────────────── */
void algo_release(int tid, int rel[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&wd_lock);

    for (int j = 0; j < NUM_ACCOUNTS; j++)
        if (rel[j] && holder[j] == tid) holder[j] = -1;

    printf("[Wait-Die] T%d: resources released.\n", tid);
    pthread_mutex_unlock(&wd_lock);
}

/* ── Print State ─────────────────────────────────────────── */
void algo_print_state(void) {
    pthread_mutex_lock(&wd_lock);

    printf("\n── Wait-Die State ─────────────────────────────────────────\n");

    printf("Timestamps:  ");
    for (int i = 0; i < NUM_THREADS; i++)
        printf("T%d:%d  ", i, timestamp[i]);
    printf("\n");

    printf("Holders:     ");
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (holder[j] == -1)
            printf("Acc%d:free  ", j);
        else
            printf("Acc%d:T%d   ", j, holder[j]);
    }
    printf("\n");

    printf("───────────────────────────────────────────────────────────\n\n");
    pthread_mutex_unlock(&wd_lock);
}
