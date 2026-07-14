#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"


void * install_body (void * arg);
void * install_engine (void * arg);
void * install_frame (void * arg);
void * install_wheels (void * arg);
void * sink (void * arg);
void * source (void * arg);

enum {
    CAR_EMPTY = 0,
    CAR_HAS_FRAME = 1 << 0,
    CAR_HAS_ENGINE = 1 << 1, 
    CAR_HAS_BODY = 1 << 2,
    CAR_HAS_WHEELS = 1 << 3,
} car;


struct car {
    int vin;
    int state;
};


struct queue_pair {
    struct queue * src;
    struct queue * tgt;
};


void * install_body (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;
    while (true) {
        struct car * car = (struct car *) queue_pop(queues->src);
        if (car == nullptr) break;
        fprintf(stdout, "vin %d: start installing body\n", car->vin);
        sleep(5 + rand() % 10);
        car->state |= CAR_HAS_BODY;
        queue_append(queues->tgt, (void *) car);
        fprintf(stdout, "vin %d: completed installing body\n", car->vin);
    }
    queue_append(queues->tgt, nullptr);
    fprintf(stdout, "thread 'install_body' complete\n");
    return nullptr;
}


void * install_engine (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;
    while (true) {
        struct car * car = (struct car *) queue_pop(queues->src);
        if (car == nullptr) break;
        fprintf(stdout, "vin %d: start installing engine\n", car->vin);
        sleep(5 + rand() % 10);
        car->state |= CAR_HAS_ENGINE;
        queue_append(queues->tgt, (void *) car);
        fprintf(stdout, "vin %d: completed installing engine\n", car->vin);
    }
    queue_append(queues->tgt, nullptr);
    fprintf(stdout, "thread 'install_engine' complete\n");
    return nullptr;
}


void * install_frame (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;
    while (true) {
        struct car * car = (struct car *) queue_pop(queues->src);
        if (car == nullptr) break;
        fprintf(stdout, "vin %d: start installing frame\n", car->vin);
        sleep(5 + rand() % 10);
        car->state |= CAR_HAS_FRAME;
        queue_append(queues->tgt, (void *) car);
        fprintf(stdout, "vin %d: completed installing frame\n", car->vin);
    }
    queue_append(queues->tgt, nullptr);
    fprintf(stdout, "thread 'install_frame' complete\n");
    return nullptr;
}


void * install_wheels (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;
    while (true) {
        struct car * car = (struct car *) queue_pop(queues->src);
        if (car == nullptr) break;
        fprintf(stdout, "vin %d: start installing wheels\n", car->vin);
        sleep(5 + rand() % 10);
        car->state |= CAR_HAS_WHEELS;
        queue_append(queues->tgt, (void *) car);
        fprintf(stdout, "vin %d: completed installing wheels\n", car->vin);
    }
    queue_append(queues->tgt, nullptr);
    fprintf(stdout, "thread 'install_wheels' complete\n");
    return nullptr;
}


int main (void) {

    // initialize the pseudorandom number generator
    srand(time(nullptr));

    // allocate the treads and the queues in between each stage
    const int nthreads = 6;
    pthread_t threads[nthreads];
    struct queue * queues[nthreads + 1] = {};

    // initialize the queues
    const int cap = 5;
    queues[0] = nullptr;
    queues[1] = queue_create(cap);
    queues[2] = queue_create(cap);
    queues[3] = queue_create(cap);
    queues[4] = queue_create(cap);
    queues[5] = queue_create(cap);
    queues[nthreads] = nullptr;

    // initialize the threads
    pthread_create(&threads[0], nullptr, source, (void *) &(struct queue_pair){.src = queues[0], .tgt = queues[1]});
    pthread_create(&threads[1], nullptr, install_frame, (void *) &(struct queue_pair){.src = queues[1], .tgt = queues[2]});
    pthread_create(&threads[2], nullptr, install_engine, (void *) &(struct queue_pair){.src = queues[2], .tgt = queues[3]});
    pthread_create(&threads[3], nullptr, install_body, (void *) &(struct queue_pair){.src = queues[3], .tgt = queues[4]});
    pthread_create(&threads[4], nullptr, install_wheels, (void *) &(struct queue_pair){.src = queues[4], .tgt = queues[5]});
    pthread_create(&threads[5], nullptr, sink, (void *) &(struct queue_pair){.src = queues[5], .tgt = queues[nthreads]});

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], nullptr);
    }

    queue_destroy(&queues[1]);
    queue_destroy(&queues[2]);
    queue_destroy(&queues[3]);
    queue_destroy(&queues[4]);
    queue_destroy(&queues[5]);

    return EXIT_SUCCESS;
}


void * sink (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;
    while (true) {
        struct car * car = (struct car *) queue_pop(queues->src);
        if (car == nullptr) break;
        fprintf(stdout, "vin %d: complete\n", car->vin);
        free(car);
    }
    fprintf(stdout, "thread 'sink' complete\n");
    return nullptr;
}


void * source (void * arg) {
    struct queue_pair * queues = (struct queue_pair *) arg;

    // specify how many cars you want to produce
    const int ncars = 10;

    for (int i = 0; i < ncars; i++) {
        sleep(1 + rand() % 2);
        struct car * car = calloc(1, sizeof(struct car));
        if (car == nullptr) {
            fprintf(stderr, "Problem allocating dynamic memory for struct car, aborting.\n");
            exit(1);
        }
        *car = (struct car) {
            .vin = i,
            .state = CAR_EMPTY,
        };
        queue_append(queues->tgt, (void *) car);
        fprintf(stdout, "vin %d: spawned\n", car->vin);
    }
    queue_append(queues->tgt, (void *) nullptr);
    fprintf(stdout, "thread 'source' complete\n");


    return nullptr;
}
