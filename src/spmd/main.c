#include <pthread.h>    // pthreads_*
#include <stdio.h>      // fprintf, stdout, stderr
#include <stdlib.h>     // EXIT_SUCCESS, srand, rand, calloc, free, exit
#include <time.h>       // time


struct ctx {
    const int * data;
    int n;
};


static int blkdcmp_get_idx_blk_s (int nelems, int nblocks, int iblock);
static int blkdcmp_get_idx_blk_e (int nelems, int nblocks, int iblock);
static void ctxs_init (int nelems, int nthreads, const int * data, struct ctx * ctxs);
static void data_init (int nitems, int * items);
static void * worker (void *);
static int serial_sum (int nelems, int * data);
static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctxs);
static int threads_join (int nthreads, pthread_t * threads);


static int blkdcmp_get_idx_blk_s (int nelems, int nblocks, int iblock) {
    return iblock * nelems / nblocks;
}


static int blkdcmp_get_idx_blk_e (int nelems, int nblocks, int iblock) {
    return ((iblock + 1) * nelems / nblocks) - 1;
}


static void ctxs_init (int nelems, int nthreads, const int * data, struct ctx * ctxs) {
    for (int i = 0; i < nthreads; i++) {
        int s = blkdcmp_get_idx_blk_s(nelems, nthreads, i);
        int e = blkdcmp_get_idx_blk_e(nelems, nthreads, i);
        ctxs[i] = (struct ctx) {
            .data = &data[s],
            .n = e - s + 1,
        };
    }
}


static void data_init (int nelems, int * data) {
    for (int i = 0; i < nelems; i++) {
        data[i] = rand() % 100;
    }
}


int main (void) {

    const int nthreads = 4;
    pthread_t threads[nthreads] = {};
    const int nelems = 100;
    int data[nelems] = {};
    struct ctx ctxs[nthreads] = {};

    fprintf(stdout, "Calculate the sum of an array of random numbers serially and in parallel\n");

    // initialize the pseudorandom number generator
    srand(time(nullptr));

    // initialize the random data
    data_init(nelems, &data[0]);

    // calculate the sum of the array using one thread
    fprintf(stdout, "serial sum  : %d\n", serial_sum(nelems, &data[0]));

    // initialize the thread contexts
    ctxs_init(nelems, nthreads, &data[0], &ctxs[0]);

    // create the threads and pass them their respective contexts
    threads_create(nthreads, &threads[0], &ctxs[0]);

    // wait until the threads are done calculating their thread local sums and collect the results
    int sum = threads_join(nthreads, threads);

    // report the total
    fprintf(stdout, "parallel sum: %d\n", sum);

    return EXIT_SUCCESS;
}


static int serial_sum (int nelems, int * data) {
    int sum = 0;
    for (int i = 0; i < nelems; i++) {
        sum += data[i];
    }
    return sum;
}


static void threads_create (int nthreads, pthread_t * threads, const struct ctx * ctxs) {
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], nullptr, worker, (void *) &ctxs[i]);
    }
}


static int threads_join (int nthreads, pthread_t * threads) {
    int sum = 0;
    for (int i = 0; i < nthreads; i++) {
        int * threadsum = nullptr;
        pthread_join(threads[i], (void **) &threadsum);
        sum += *threadsum;
        free(threadsum);
    }
    return sum;
}


void * worker (void * arg) {
    struct ctx * ctx = (struct ctx *) arg;
    int * sum = calloc(1, sizeof(int));
    if (sum == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for thread sum, aborting\n", code);
        exit(code);
    }
    for (int i = 0; i < ctx->n; i++) {
        *sum += ctx->data[i];
    }
    fprintf(stdout, "thread sum  : %d\n", *sum);
    return (void *) sum;
}
