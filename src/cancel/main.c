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
    // make the thread temporarily uncancelable until the cleanup has been set up completely
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    // the thread will be deferred-cancelable
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    // define some arbitrary memory resources to illustrate cleaning up
    char * s = calloc(14, sizeof(char));
    if (s == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dyncamic memory, aborting.\n", code);
        exit(code);
    }
    strncpy(s, "hello cleanup", 14);

    // push the cleanup of `s` onto the cleanup handlers stack (LIFO)
    pthread_cleanup_push(cleanup, s);

    // make the thread cancelable
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

    // now for the meat and potatoes of the thread, two nested for loops:
    // outer compute bound loop is interruptable; the inner compute bound
    // loop is not
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
