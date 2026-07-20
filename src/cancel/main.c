#include <limits.h>     // INT_MAX
#include <pthread.h>    // pthread_*
#include <stdio.h>      // fprintf, stdout, fflush
#include <stdlib.h>     // EXIT_SUCCESS
#include <string.h>     // strncpy
#include <unistd.h>     // sleep


void cleanup (void * arg);
void * compute_bound_loop (void * arg);


void cleanup (void * arg) {
    char * s = (char *) arg;
    fprintf(stdout, "Starting the cleanup...\n");
    free(s);
    fprintf(stdout, "Done with the cleanup\n");
    fflush(stdout);
}


void * compute_bound_loop (void *) {
    char * s = calloc(14, sizeof(char));
    if (s == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dyncamic memory, aborting.\n", code);
        exit(code);
    }
    strncpy(s, "hello cleanup", 14);
    pthread_cleanup_push(cleanup, s);


    // outer compute bound loop is interruptable
    // inner compute bound loop is uninterruptable
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    for (int i = 0; i < INT_MAX; i++) {
        for (int j = 0; j < INT_MAX; j++) {
            (void) 0;  // do nothing but make sure the compiler retains the loop
        }
        {
            // avoid fprintf serving as a cancellation point
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
            fprintf(stdout, "%i\n", i);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
        }
        pthread_testcancel();
    }

    // need pop whenever you use push, due to its macro implementation
    pthread_cleanup_pop(1);

    return nullptr;
}


int main (void) {

    fprintf(stdout, "Making a compute bound loop cancelable, with cleanup of resources\n");

    pthread_t thread;

    pthread_create(&thread, nullptr, compute_bound_loop, nullptr);

    sleep(10);

    fprintf(stdout, "Canceling compute-bound thread now...\n");
    fflush(stdout);
    pthread_cancel(thread);

    void * result;
    pthread_join(thread, &result);

    if (result != PTHREAD_CANCELED) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: Compute-bound thread terminated normally.\n", code);
        exit(code);
    }

    fprintf(stdout, "Compute-bound thread was canceled as expected.\n");

    return EXIT_SUCCESS;
}
