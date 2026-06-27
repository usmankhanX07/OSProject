#ifndef DEADLOCK_ALGO_H
#define DEADLOCK_ALGO_H

#include "common.h"

void algo_init   (int total[NUM_ACCOUNTS],
                  int max_per_thread[NUM_THREADS][NUM_ACCOUNTS]);

int  algo_request(int tid, int req[NUM_ACCOUNTS]);

void algo_release (int tid, int rel[NUM_ACCOUNTS]);
void algo_print_state();
#endif
