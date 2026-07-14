#include <stdlib.h>     // EXIT_SUCCESS
#include <stdio.h>      // fprintf, stdout, stderr 
#include <pthread.h>    // pthread_*


#define NTHREADS 4


struct account {
    int balance;
    pthread_mutex_t mutex;
};


void * incrementer (void *);


int main (void) {

    fprintf(stdout, "Concurrently increment a shared variable a=0 one million times in each of %d threads\n", NTHREADS);

    // initialize the account balance and its mutex
    struct account account = {.balance = 0};
    pthread_mutex_init(&account.mutex, NULL);

    // declare an array of NTHREADS threads
    pthread_t threads[NTHREADS];

    // start running the threads
    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&threads[i], nullptr, incrementer, (void *) &account);
    }

    // wait for the threads to be done
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    // print the account balance
    fprintf(stdout, "a=%d\n", account.balance);

    // clean up memory resources
    pthread_mutex_destroy(&account.mutex);

    return EXIT_SUCCESS;
}


void * incrementer (void * arg) {
    struct account * account = (struct account *) arg;
    pthread_mutex_lock(&account->mutex);
    for (int i = 0; i < 1000000; i++) {
        account->balance++;
    }
    pthread_mutex_unlock(&account->mutex);
    return (void *) nullptr;
}
