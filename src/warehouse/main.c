#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "stack.h"


void * consumer (void * arg);
void * producer (void * arg);
void * monitor (void * arg);


// thread monitoring state
static bool keep_monitoring = true;


void * consumer (void * arg) {
    int n = 10;
    struct stack * stack = (struct stack *) arg;
    for (int i = 0; i < n; i++) {
        sleep(1 + rand() % 15);
        fprintf(stdout, "consumer wants to pop item\n");
        int item_id = stack_pop_item(stack);
        fprintf(stdout, "consumer popped item with id %d\n", item_id);
    }
    return nullptr;
}


void * producer (void * arg) {
    struct stack * stack = (struct stack *) arg;
    int n = 10; 
    for (int i = 0; i < n; i++) {
        sleep(1 + rand() % 10);
        int item_id = 100 + i;
        fprintf(stdout, "producer wants to add item with id %d\n", item_id);
        stack_add_item(stack, item_id);
        fprintf(stdout, "producer added item with id %d\n", item_id);
    }
    return nullptr;
}


int main (void) {

    // initialize the pseudorandom number generator
    srand(time(nullptr));

    // there are going to be 3 threads
    const int nthreads = 3;
    pthread_t threads[nthreads];

    struct stack * stack = stack_create(3);

    pthread_create(&threads[0], nullptr, consumer, (void *) stack);
    pthread_create(&threads[1], nullptr, producer, (void *) stack);
    pthread_create(&threads[2], nullptr, monitor, (void *) stack);

    pthread_join(threads[0], nullptr);
    pthread_join(threads[1], nullptr);
    keep_monitoring = false;
    pthread_join(threads[2], nullptr);

    stack_destroy(&stack);

    return EXIT_SUCCESS;
}


void * monitor (void * arg) {
    struct stack * stack = (struct stack *) arg;
    while (keep_monitoring) {
        stack_print(stack);
        sleep(1);
    }
    return nullptr;
}
