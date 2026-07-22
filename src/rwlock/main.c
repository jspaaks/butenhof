#include "rwlock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


struct ctx {
    struct rwlock * rwlock;
    int id;
    int pct_write;
    int niterations;
    int * val;
};


static void ctxs_destroy (struct ctx * ctx);
static void ctxs_init (int n, struct ctx * ctx);
static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctxs);
static void threads_join (int nthreads, pthread_t * threads);
static void * worker (void * arg);


static void ctxs_init (int n, struct ctx * ctx) {
    struct rwlock * rwlock = rwlock_create();
    int * val = calloc(1, sizeof(int));
    if (val == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for `struct ctx` member `val`, aborting.\n", code);
        exit(code);
    }
    for (int i = 0; i < n; i++) {
        ctx[i] = (struct ctx) {
            .rwlock = rwlock,
            .id = i,
            .pct_write = 5,
            .niterations = 20,
            .val = val,
        };
    }
}


static void ctxs_destroy (struct ctx * ctx) {
    rwlock_destroy(&ctx->rwlock);
    free(ctx->val);
}


int main (void) {

    int nthreads = 10;
    pthread_t threads[nthreads];
    struct ctx ctxs[nthreads];

    fprintf(stdout,
            "Reading and writing a rwlock protected value using multiple threads\n"
            "thread_id|reading|writing|done|value\n"
            "---------|-------|-------|----|-----\n");

    // initialize the pseudorandom number generator using the current time
    srand(time(nullptr));

    // initialize the thread contexts
    ctxs_init(nthreads, &ctxs[0]);

    // initialize the threads
    threads_create(nthreads, &threads[0], &ctxs[0]);

    // wait until all threads are done
    threads_join(nthreads, &threads[0]);

    // release dynamic memory associated with `ctxs`
    ctxs_destroy(&ctxs[0]);

    return EXIT_SUCCESS;
}


static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctxs) {
    for (int i = 0 ; i < nthreads; i++) {
        pthread_create(&threads[i], nullptr, worker, (void *) &ctxs[i]);
    }
}


static void threads_join (int nthreads, pthread_t * threads) {
    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], nullptr);
    }
}


static void * worker (void * arg) {
    struct ctx * ctx = (struct ctx *) arg;
    enum state {STATE_READING, STATE_WRITING} state = STATE_READING;
    int millisecond = 1000;
    for (int i = 0; i < ctx->niterations; i++) {
        if (rand() % 100 < ctx->pct_write) {
            state = STATE_WRITING;
        } else {
            state = STATE_READING;
        }
        switch (state) {
        case STATE_READING:
            rwlock_lockr(ctx->rwlock);
            fprintf(stdout, "%9d|reading|       |    |%5d\n", ctx->id, *ctx->val);
            usleep(50 * millisecond);      // simulate duration of reading
            rwlock_unlockr(ctx->rwlock);
            break;
        case STATE_WRITING:
            rwlock_lockw(ctx->rwlock);
            (*ctx->val)++;
            fprintf(stdout, "%9d|       |writing|    |%5d\n", ctx->id, *ctx->val);
            usleep(500 * millisecond);     // simulate duration of writing
            rwlock_unlockw(ctx->rwlock);
            break;
        default:
            const int code = __LINE__;
            fprintf(stderr, "ERROR %d reached unreachable code, aborting.\n", code);
            exit(code);
        }
        usleep(100 * millisecond);         // let the loop spend some time in unlocked
                                           // state to promote thread context switching
    }
    fprintf(stdout, "%9d|       |       |done\n", ctx->id);
    return nullptr;
}
