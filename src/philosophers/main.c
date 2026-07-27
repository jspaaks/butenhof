#include <pthread.h>   // pthread_*
#include <stdio.h>     // fprintf, stderr 
#include <stdlib.h>    // calloc, free, EXIT_SUCCESS, rand
#include <time.h>      // time
#include <unistd.h>    // sleep


struct philosopher {
    int id;
    int nbites;
    pthread_mutex_t * forks[2];
    enum {STATE_EATING, STATE_THINKING} state;
};


static struct philosopher * create_philosophers (int n);
static void create_threads (int n, struct philosopher * philosophers, pthread_t * threads);
static void destroy_philosophers (int n, struct philosopher ** philosophers);
static void * dining (void * arg);
static void join_threads (int n, pthread_t * threads);


static struct philosopher * create_philosophers (int n) {
    pthread_mutex_t * forks = calloc(n, sizeof(pthread_mutex_t));
    if (forks == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for array of forks, aborting.\n", code);
        exit(code);
    }
    struct philosopher * philosophers = calloc(n, sizeof(struct philosopher));
    if (philosophers == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for array of philosophers, aborting.\n", code);
        exit(code);
    }
    for (int i = 0; i < n; i++) {
        int left = i;
        int right = (i + 1) % n;
        bool iseven = i % 2 == 0;
        philosophers[i] = (struct philosopher) {
            .id = i,
            .nbites = 2 + rand() % 5,
            .forks = {&forks[iseven ? left : right], &forks[iseven ? right : left]},
            .state = STATE_THINKING,
        };
    }
    return philosophers;
}


static void create_threads (int n, struct philosopher * philosophers, pthread_t * threads) {
    for (int i = 0; i < n; i++) {
        pthread_create(&threads[i], nullptr, dining, (void *) &philosophers[i]);
    }
}


static void destroy_philosophers (int n, struct philosopher ** philosophers) {
    // release dynamic memory associated with the contents of each mutex
    for (int i = 0; i < n; i += 2) {
        pthread_mutex_destroy((*philosophers)[i].forks[0]);
        pthread_mutex_destroy((*philosophers)[i].forks[1]);
    }

    // release dynamic memory associated with array of n mutexes
    free((*philosophers)[0].forks[0]);
    (*philosophers)[0].forks[0] = nullptr;

    // release dynamic memory associated with the array of philosophers
    free(*philosophers);
    *philosophers = nullptr;
}


static void * dining (void * arg) {
    struct philosopher * philosopher = (struct philosopher *) arg;
    for (int i = 0; i < philosopher->nbites; i++) {
        pthread_mutex_lock(philosopher->forks[0]);
        fprintf(stdout, "philosopher %d has acquired the first fork    (id = %p)\n", philosopher->id, (void *) philosopher->forks[0]);
        pthread_mutex_lock(philosopher->forks[1]);
        fprintf(stdout, "philosopher %d has acquired the second fork   (id = %p)\n", philosopher->id, (void *) philosopher->forks[1]);
        fprintf(stdout, "philosopher %d starts eating bite %d of %d\n", philosopher->id, i + 1, philosopher->nbites);
        philosopher->state = STATE_EATING;
        sleep(1 + rand() % 10);
        fprintf(stdout, "philosopher %d done eating bite %d of %d\n", philosopher->id, i + 1, philosopher->nbites);
        pthread_mutex_unlock(philosopher->forks[1]);
        fprintf(stdout, "philosopher %d has released their second fork (id = %p)\n", philosopher->id, (void *) philosopher->forks[1]);
        pthread_mutex_unlock(philosopher->forks[0]);
        fprintf(stdout, "philosopher %d has released their first fork  (id = %p)\n", philosopher->id, (void *) philosopher->forks[0]);
        philosopher->state = STATE_THINKING;
        fprintf(stdout, "philosopher %d is thinking\n", philosopher->id);
        sleep(1 + rand() % 10);
    }
    fprintf(stdout, "philosopher %d is done\n", philosopher->id);
    return nullptr;
}


static void join_threads (int n, pthread_t * threads) {
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], nullptr);
    }
}


int main (void) {

    // initialize the pseudorandom number generator with the current time
    srand(time(nullptr));

    const int n = 5;
    pthread_t threads[n];
    struct philosopher * philosophers = create_philosophers(n);
    create_threads (n, &philosophers[0], &threads[0]);
    join_threads(n, &threads[0]);
    destroy_philosophers(n, &philosophers);

    return EXIT_SUCCESS;
}
