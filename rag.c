#include "deadlock_algo.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* ── Graph State ─────────────────────────────────────────────
 * assignment[r][t] = 1  →  thread t holds account r  (R → T edge)
 * request   [t][r] = 1  →  thread t wants account r  (T → R edge)
 * ─────────────────────────────────────────────────────────── */
static int assignment[NUM_ACCOUNTS][NUM_THREADS];
static int request   [NUM_THREADS] [NUM_ACCOUNTS];

static pthread_mutex_t rag_lock = PTHREAD_MUTEX_INITIALIZER;

/* ── Cycle Detection (DFS on thread graph) ───────────────────
 * Build implicit thread→thread edges:
 *   T1 → T2  if T1 requests R and R is assigned to T2
 * Then DFS from 'start' to see if we can reach it again.
 * ─────────────────────────────────────────────────────────── */
static int dfs(int current, int target, int visited[NUM_THREADS]) {
    visited[current] = 1;

    for (int r = 0; r < NUM_ACCOUNTS; r++) {
        if (!request[current][r]) continue;        /* no request edge T→R */

        for (int t = 0; t < NUM_THREADS; t++) {
            if (!assignment[r][t]) continue;       /* no assignment edge R→T */

            if (t == target)   return 1;           /* cycle found */
            if (visited[t])    continue;
            if (dfs(t, target, visited)) return 1;
        }
    }
    return 0;
}

static int has_cycle_if_assigned(int tid, int acc) {
    /* Tentatively assign acc to tid, then check for cycle */
    assignment[acc][tid] = 1;
    request   [tid][acc] = 0;

    int visited[NUM_THREADS];
    memset(visited, 0, sizeof(visited));
    int cycle = dfs(tid, tid, visited);

    /* Roll back tentative edges */
    assignment[acc][tid] = 0;
    request   [tid][acc] = 1;

    return cycle;
}

/* ── Init ────────────────────────────────────────────────── */
void algo_init(int total[NUM_ACCOUNTS],
               int max_per_thread[NUM_THREADS][NUM_ACCOUNTS]) {
    pthread_mutex_lock(&rag_lock);

    memset(assignment, 0, sizeof(assignment));
    memset(request,    0, sizeof(request));

    /* RAG does not use max claims — deadlock is detected
     * structurally via cycles, not by pre-declared maximums. */
    (void)total;
    (void)max_per_thread;

    pthread_mutex_unlock(&rag_lock);
    printf("[RAG] Initialised. Accounts: %d  Threads: %d\n",
           NUM_ACCOUNTS, NUM_THREADS);
}

/* ── Request ─────────────────────────────────────────────── */
int algo_request(int tid, int req[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&rag_lock);

    /* Mark all needed accounts as requested */
    for (int j = 0; j < NUM_ACCOUNTS; j++)
        if (req[j]) request[tid][j] = 1;

    /* Check each needed account: is it free, and does assigning
     * it create a cycle? */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (!req[j]) continue;

        /* Check if already held by someone else */
        for (int t = 0; t < NUM_THREADS; t++) {
            if (assignment[j][t]) {
                printf("[RAG] T%d: acc%d held by T%d. Must wait.\n",
                       tid, j, t);
                /* Clear request edges we just added */
                for (int k = 0; k < NUM_ACCOUNTS; k++)
                    if (req[k]) request[tid][k] = 0;
                pthread_mutex_unlock(&rag_lock);
                return 0;
            }
        }

        /* Check if assigning would create a cycle */
        if (has_cycle_if_assigned(tid, j)) {
            printf("[RAG] T%d: assigning acc%d would cause a cycle. Denied.\n",
                   tid, j);
            for (int k = 0; k < NUM_ACCOUNTS; k++)
                if (req[k]) request[tid][k] = 0;
            pthread_mutex_unlock(&rag_lock);
            return 0;
        }
    }

    /* Safe — commit all assignments */
    for (int j = 0; j < NUM_ACCOUNTS; j++) {
        if (!req[j]) continue;
        assignment[j][tid] = 1;
        request   [tid][j] = 0;
    }

    printf("[RAG] T%d: request granted.\n", tid);
    pthread_mutex_unlock(&rag_lock);
    return 1;
}

/* ── Release ─────────────────────────────────────────────── */
void algo_release(int tid, int rel[NUM_ACCOUNTS]) {
    pthread_mutex_lock(&rag_lock);

    for (int j = 0; j < NUM_ACCOUNTS; j++)
        if (rel[j]) assignment[j][tid] = 0;

    printf("[RAG] T%d: resources released.\n", tid);
    pthread_mutex_unlock(&rag_lock);
}

/* ── Print State ─────────────────────────────────────────── */
void algo_print_state(void) {
    pthread_mutex_lock(&rag_lock);

    printf("\n── RAG State ──────────────────────────────────────────────\n");

    printf("Assignment edges (R → T):\n");
    int any = 0;
    for (int r = 0; r < NUM_ACCOUNTS; r++)
        for (int t = 0; t < NUM_THREADS; t++)
            if (assignment[r][t]) {
                printf("  Acc%d → T%d\n", r, t);
                any = 1;
            }
    if (!any) printf("  (none)\n");

    printf("Request edges (T → R):\n");
    any = 0;
    for (int t = 0; t < NUM_THREADS; t++)
        for (int r = 0; r < NUM_ACCOUNTS; r++)
            if (request[t][r]) {
                printf("  T%d → Acc%d\n", t, r);
                any = 1;
            }
    if (!any) printf("  (none)\n");

    printf("───────────────────────────────────────────────────────────\n\n");
    pthread_mutex_unlock(&rag_lock);
}
