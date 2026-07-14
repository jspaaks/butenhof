#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct ctx {
    int id;
};

struct thread {
    struct ctx ctx;
    pthread_t thread;
};


void * say_hello_thread (void *);


int main (void) {
    // there are going to be 4 threads
    const int nthreads = 4;
    struct thread threads[nthreads];

    // give the threads identifiers
    for (int i = 0; i < nthreads; i++) {
        threads[i] = (struct thread) {
            .ctx = {
                .id = i,
            },
            .thread = 0,
        };
    }

    fprintf(stdout, "hello from main\n");

    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i].thread, nullptr, say_hello_thread, (void *) &threads[i].ctx);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i].thread, nullptr);
    }

    fprintf(stdout, "goodbye from main\n");
    return EXIT_SUCCESS;
}


void * say_hello_thread (void * arg) {
    struct ctx ctx = *(struct ctx *) arg;
    fprintf(stdout, "hello from thread %d\n", ctx.id);
    sleep(1);
    fprintf(stdout, "goodbye from thread %d\n", ctx.id);
    return nullptr;
}
