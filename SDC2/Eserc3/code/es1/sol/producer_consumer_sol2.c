#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>       // nanosleep()
#include "common.h"

#define BUFFER_SIZE         128
#define INITIAL_DEPOSIT     0
#define MAX_TRANSACTION     1000
#define NUM_CONSUMERS       10
#define NUM_PRODUCERS       1
#define PRNG_SEED           0

#define NUM_OPERATIONS      40000
#define OPS_PER_CONSUMER    (NUM_OPERATIONS/NUM_CONSUMERS)
#define OPS_PER_PRODUCER    (NUM_OPERATIONS/NUM_PRODUCERS)

// we use the preprocessor to check if our parameters are okay
#if OPS_PER_CONSUMER*NUM_CONSUMERS != OPS_PER_PRODUCER*NUM_PRODUCERS
#error "Choose NUM_CONSUMERS and NUM_PRODUCERS so that we get exactly NUM_OPERATIONS operations"
#endif

// struct used to specify arguments for a thread
typedef struct {
    int threadId;
    int numOps;
} thread_args_t;

// shared data
int transactions[BUFFER_SIZE];
int deposit = INITIAL_DEPOSIT;
int read_index, write_index;

sem_t sem_sync;
sem_t sem_buffer;
sem_t sem_consumer;

// generates a number between -MAX_TRANSACTION and +MAX_TRANSACTION
static inline int performRandomTransaction() {
    // struct timespec pause = {0};
    // pause.tv_nsec = 10000000; // 10 ms (100*10^6 ns)
    // nanosleep(&pause, NULL);

    // int amount = rand() % (2 * MAX_TRANSACTION); // {0, ..., 2*MAX_TRANSACTION - 1}
    // if (amount >= MAX_TRANSACTION) {
    //     return MAX_TRANSACTION - (amount+1); // {-MAX_TRANSACTION, ..., -1}
    // } else {
    //     return amount + 1; // {1, ..., MAX_TRANSACTION}
    // }
    return 1;
}

// producer thread
void* performTransactions(void* x) {
    thread_args_t* args = (thread_args_t*)x;
    printf("Starting producer thread %d\n", args->threadId);

    while (args->numOps > 0) {
        // produce the item
        int currentTransaction = performRandomTransaction();
        
        int ret = sem_wait(&sem_buffer);
        if(ret == -1) handle_error("Wait error in semaphore buffer performTransaction");
        // write the item and update write_index accordingly
            transactions[write_index] = currentTransaction;
            write_index = (write_index + 1) % BUFFER_SIZE;

        ret = sem_post(&sem_sync);
        if(ret == -1) handle_error("Post error in semaphore sync performTransaction");
        
        args->numOps--;
        //printf("P %d\n", args->numOps);
    }

    free(args);
    pthread_exit(NULL);
}

void* processTransactions(void* x) {
    thread_args_t* args = (thread_args_t*)x;
    printf("Starting consumer thread %d\n", args->threadId);

    while (args->numOps > 0) {
        // consume the item and update (shared) variable deposit

        int ret = sem_wait(&sem_sync);
        if(ret == -1) handle_error("Wait error in semaphore sync processTransaction");
        
         ret = sem_wait(&sem_consumer);
        if(ret == -1) handle_error("Wait error in semaphore consumer processTransaction");

        deposit += transactions[read_index];
        read_index = (read_index + 1) % BUFFER_SIZE;
        if (read_index % 100 == 0)
			printf("After the last 100 transactions balance is now %d.\n", deposit);

        ret = sem_post(&sem_consumer);
        if(ret == -1) handle_error("Post error in semaphore consumer processTransaction");
        
        args->numOps--;

      
        ret = sem_post(&sem_buffer);
        if(ret == -1) handle_error("Post error in semaphore buffer processTransaction");
        //printf("C %d\n", args->numOps);
    }

    free(args);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    printf("Welcome! This program simulates financial transactions on a deposit.\n");
    printf("\nThe maximum amount of a single transaction is %d (negative or positive).\n", MAX_TRANSACTION);
    printf("\nInitial balance is %d. Press CTRL+C to quit.\n\n", INITIAL_DEPOSIT);

    // initialize read and write indexes
    int ret;
    read_index  = 0;
    write_index = 0;

    ret = sem_init(&sem_sync,0,0);
    if(ret == -1 ) handle_error("Error in semaphore sync initialization");
    ret = sem_init(&sem_buffer,0,BUFFER_SIZE);
    if(ret == -1 ) handle_error("Error in semaphore buffer initialization");
    ret = sem_init(&sem_consumer,0,1);
    if(ret == -1 ) handle_error("Error in semaphore consumer initialization");

    // set seed for pseudo-random number generator: we use this to make
    // this code yield the same result across different runs, as long
    // as they are race-free and you make no mistakes :-)
    srand(PRNG_SEED);

    pthread_t producer[NUM_PRODUCERS], consumer[NUM_CONSUMERS];

    int i;
    for (i=0; i<NUM_PRODUCERS; ++i) {
        thread_args_t* arg = malloc(sizeof(thread_args_t));
        arg->threadId = i;
        arg->numOps = OPS_PER_PRODUCER;

        ret = pthread_create(&producer[i], NULL, performTransactions, arg);
        if (ret != 0)  handle_error_en(ret,"Error in pthread create (producer)");
    }

    int j;
    for (j=0; j<NUM_CONSUMERS; ++j) {
        thread_args_t* arg = malloc(sizeof(thread_args_t));
        arg->threadId = j;
        arg->numOps = OPS_PER_CONSUMER;

        ret = pthread_create(&consumer[j], NULL, processTransactions, arg);
        if (ret != 0) handle_error_en(ret,"Error in pthread create (consumer)");
    }

    // join on threads
    for (i=0; i<NUM_PRODUCERS; ++i) {
        ret = pthread_join(producer[i], NULL);
        if (ret != 0) handle_error_en(ret,"Error in pthread join (producer)");
    }

    for (j=0; j<NUM_CONSUMERS; ++j) {
        ret = pthread_join(consumer[j], NULL);
        if (ret != 0) handle_error_en(ret,"Error in pthread join (consumer)");
    }



    printf("Final value for deposit: %d\n", deposit);

    if(sem_destroy(&sem_buffer) == -1) handle_error("sem_buffer destroy error");
    if(sem_destroy(&sem_sync) == -1) handle_error("sem_buffer sync error");
    if(sem_destroy(&sem_consumer) == -1) handle_error("sem_buffer consumer error");

    exit(EXIT_SUCCESS);
}
