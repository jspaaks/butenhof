#include <pthread.h>    // pthread_*
#include <stdio.h>      // fprintf, stdout, fflush
#include <stdlib.h>     // EXIT_SUCCESS
#include <unistd.h>     // usleep, sleep


struct incrementer {
    void * (*fun) (void * arg);
    struct incrementer_ctx {
        pthread_barrier_t * barrier;
        char * ch;
        int sleep_duration;
    } ctx;
};


struct monitor {
    void * (*fun) (void * arg);
    struct monitor_ctx {
        char * letters;
    } ctx;
};


void * incrementerfun (void *);
void * monitorfun (void *);


void * incrementerfun (void * arg) {
    // dynamically detach myself so `main` doesn't have to join me later
    pthread_t self = pthread_self();
    pthread_detach(self);
    struct incrementer_ctx * ctx = (struct incrementer_ctx *) arg;
    while (*ctx->ch != 'f') {
        sleep(ctx->sleep_duration);
        (*ctx->ch)++;
        pthread_barrier_wait(ctx->barrier);
    }

    // allow 0.1 s for the monitor to show the updated state
    usleep(100000);

    return (void *) nullptr;
}


int main (void) {

    fprintf(stdout,
            "Concurrently update each character in a string, but use\n"
            "a barrier to synchronize each thread\n");

    // declare an array of NTHREADS threads
    pthread_t threads[5];

    // note, access to `shared` is not protected by a mutex but
    // threads 0-3 update separate positions; position 4 starts
    // out as a \0 but becomes a '\n' at the end of monitorfun;
    // positon 5 is '\0' because shared is a cstring.
    char shared[6] = "aaaa";

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, nullptr, 4);

    // start running the incrementer threads
    struct incrementer incrementers[4] = {};
    for (int i = 0; i < 4; i++) {
        incrementers[i] = (struct incrementer) {
            .fun = incrementerfun,
            .ctx = {
                .barrier = &barrier,
                .ch = &shared[i],
                .sleep_duration = 1 << i,
            },
        };
        pthread_create(&threads[i], nullptr, incrementers[i].fun, (void *) &incrementers[i].ctx);
    }

    // start running the monitor thread
    struct monitor monitor = {
        .fun = monitorfun,
        .ctx = {
            .letters = &shared[0],
        },
    };
    pthread_create(&threads[4], nullptr, monitor.fun, (void *) &monitor.ctx);

    // block main thread execution until monitor thread is done
    pthread_join(threads[4], nullptr);

    // release memory resources associated with barrier
    pthread_barrier_destroy(&barrier);

    return EXIT_SUCCESS;
}


void * monitorfun (void * arg) {
    struct monitor_ctx * ctx = (struct monitor_ctx *) arg;
    do {
        usleep(50000);
        fprintf(stdout, "\r> %s", ctx->letters);
        fflush(stdout);
    } while (ctx->letters[3] != 'f');
    ctx->letters[4] = '\n';
    ctx->letters[5] = '\0';
    fprintf(stdout, "\r> %s", ctx->letters);
    fflush(stdout);
    return nullptr;
}
