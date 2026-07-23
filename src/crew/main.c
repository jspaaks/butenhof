#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


struct ctx {
    int id;
    struct queue * queue;
    bool * terminate;
};


static void ctxs_init (int nthreads, struct ctx * ctxs, const struct queue * queue, bool * terminate);
static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctx);
static void threads_join (int nthreads, pthread_t * threads);
static void * worker (void *);


static void ctxs_init (int nthreads, struct ctx * ctxs, const struct queue * queue, bool * terminate) {
    for (int i = 0; i < nthreads; i++) {
        ctxs[i] = (struct ctx) {
            .id = i + 1,
            .terminate = terminate,
            .queue = (struct queue *) queue,
        };
    }
}


int main (void) {
    fprintf(stdout, "hello from main\n");

    // create the work items queue
    struct queue * queue = queue_create();

    // statically allocate the thread termination flag
    bool terminate = false;

    // define the number of threads
    const int nthreads = 4;

    // initialize the thread contexts
    struct ctx ctxs[nthreads] = {};
    ctxs_init(nthreads, ctxs, queue, &terminate);

    // create the threads
    pthread_t threads[nthreads] = {};
    threads_create(nthreads, &threads[0], &ctxs[0]);

    // let her rip for 10 s
    sleep(10);
    terminate = true;

    // waiting until the treads return
    threads_join(nthreads, &threads[0]);

    // release dynamic memory associated with the queue
    queue_destroy(&queue);

    fprintf(stdout, "bye from main\n");

    return EXIT_SUCCESS;
}


static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctxs) {
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], nullptr, worker, (void *) &ctxs[i]);
    }
}


static void threads_join (int nthreads, pthread_t * threads) {
    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], nullptr);
    }
}


static void * worker (void * arg) {
    struct ctx * ctx = (void *) arg;
    fprintf(stdout, "hello from thread %d\n", ctx->id);
    while (*ctx->terminate != true) {
        int * pushed = calloc(1, sizeof(int));
        if (pushed == nullptr) {
            const int code = __LINE__;
            fprintf(stderr, "ERROR %d: problem allocating dynamic memory for `pushed`, aborting", code);
            exit(code);
        }
        *pushed = ctx->id * 100 + rand() % 100;
        queue_push(ctx->queue, (void *) pushed);
        fprintf(stdout, "thread %2d pushed        %3d\n", ctx->id, *pushed);
        sleep(ctx->id);
        int * popped = (int *) queue_pop(ctx->queue);
        fprintf(stdout, "thread %2d        popped %3d\n", ctx->id, *popped);
        free(popped);
    }
    fprintf(stdout, "bye from thread %d\n", ctx->id);
    fflush(stdout);

    return nullptr;
}
