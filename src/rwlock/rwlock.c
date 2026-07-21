#include "rwlock.h"   // rwlock_* 
#include <errno.h>    // EBUSY
#include <pthread.h>  // ptread_*
#include <stdio.h>    // fprintf, stderr
#include <stdlib.h>   // EXIT_SUCCESS, calloc, free, exit


struct rwlock {
    pthread_mutex_t mutex;
    pthread_cond_t cond_can_read;
    pthread_cond_t cond_can_write;
    struct {
        int active;
        int waiting;
    } nreaders;
    struct {
        int active;
        int waiting;
    } nwriters;
};

static void rwlock_cleanupr (void * arg);
static void rwlock_cleanupw (void * arg);


static void rwlock_cleanupr (void * arg) {
    struct rwlock * self = (struct rwlock *) arg;
    self->nreaders.waiting--;
    pthread_mutex_unlock(&self->mutex);
}


static void rwlock_cleanupw (void * arg) {
    struct rwlock * self = (struct rwlock *) arg;
    self->nwriters.waiting--;
    pthread_mutex_unlock(&self->mutex);
}


struct rwlock * rwlock_create (void) {
    struct rwlock * self = calloc(1, sizeof(struct rwlock));
    if (self == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for struct rwlock, aborting\n", code);
        exit(code);
    }
    *self = (struct rwlock) {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond_can_read = PTHREAD_COND_INITIALIZER,
        .cond_can_write = PTHREAD_COND_INITIALIZER,
        .nreaders = {
            .active = 0,
            .waiting = 0,
        },
        .nwriters = {
            .active = 0,
            .waiting = 0,
        },
    };
    return self;
}


int rwlock_destroy (struct rwlock ** self) {

    // wait until the calling thread can  acquire the mutex
    pthread_mutex_lock(&(*self)->mutex);

    // if there are any threads active or waiting for reading or writing, they
    // will fail if I continue destroying the `struct rwlock` here, so check
    // to make sure there aren't any; release the mutex if there are and return
    if (((*self)->nreaders.active > 0) ||
        ((*self)->nreaders.waiting > 0) ||
        ((*self)->nwriters.active > 0) || 
        ((*self)->nwriters.waiting > 0)) {
        pthread_mutex_unlock(&(*self)->mutex);
        return EBUSY;
    }

    // unlock the mutex before destroying
    pthread_mutex_unlock(&(*self)->mutex);
    pthread_mutex_destroy(&(*self)->mutex);

    // destroy the condition variables
    pthread_cond_destroy(&(*self)->cond_can_read);
    pthread_cond_destroy(&(*self)->cond_can_write);

    // free the dynamically allocated memory associated with the `struct rwlock`, and
    // set its pointer to `nullptr`
    free(*self);
    *self = nullptr;

    // assume that if you got this far, it all worked
    return 0;
}


void rwlock_lockr (struct rwlock * self) {
    // lock the mutex while we read and write the members of `self`
    pthread_mutex_lock(&self->mutex);

    // if any threads are currently writing, the calling thread's request to lock for reading will need to wait
    if (self->nwriters.active > 0) {

        // increment the number of waiting threads while the calling thread is waiting
        self->nreaders.waiting++;

        // add a handler for when the calling thread is canceled while waiting
        pthread_cleanup_push(rwlock_cleanupr, (void *) self);

        // put the calling thread to sleep until you receive word that there are no more writers
        while (self->nwriters.active > 0) {
            pthread_cond_wait(&self->cond_can_read, &self->mutex);
        }

        // don't need to allow for calling thread being canceled anymore if we get to this line at all
        pthread_cleanup_pop(0);

        // decrement the number of readers waiting since the calling thread is now no longer waiting
        self->nreaders.waiting--;
    }

    // increment the number of active readers since the calling thread is now active
    self->nreaders.active++;

    // unlock the mutex now that `self` is back in a consistent state
    pthread_mutex_unlock(&self->mutex);
}


void rwlock_lockw (struct rwlock * self) {
    // lock the mutex while we read and write the members of `self`
    pthread_mutex_lock(&self->mutex);

    // if any threads are currently reading or writing, the calling thread's request to lock for writing will need to wait
    if (self->nreaders.active > 0 || self->nwriters.active > 0) {

        // increment the number of waiting threads while the calling thread is waiting
        self->nwriters.waiting++;

        // add a handler for when the calling thread is canceled while waiting
        pthread_cleanup_push(rwlock_cleanupw, (void *) self);

        // put the calling thread to sleep until you receive word that there are no more readers and no more writers
        while (self->nreaders.active > 0 || self->nwriters.active > 0) {
            pthread_cond_wait(&self->cond_can_write, &self->mutex);
        }

        // don't need to allow for calling thread being canceled anymore if we get to this line at all
        pthread_cleanup_pop(0);

        // decrement the number of writers waiting since the calling thread is now no longer waiting
        self->nwriters.waiting--;
    }

    // increment the number of active readers since the calling thread is now active
    self->nwriters.active++;

    // unlock the mutex now that `self` is back in a consistent state
    pthread_mutex_unlock(&self->mutex);
}


void rwlock_unlockr (struct rwlock * self) {
    // lock the mutex while we read and write the members of `self`
    pthread_mutex_lock(&self->mutex);

    // decrement the number of active readers since the calling thread is now no longer active
    self->nreaders.active--;

    // if the above decrement means there are now no longer any active readers, send the corresponding
    // signal to wake up a waiting writer if there are any
    if (self->nreaders.active == 0 && self->nwriters.waiting > 0) {
        pthread_cond_signal(&self->cond_can_write);
    }

    // unlock the mutex now that `self` is back in a consistent state
    pthread_mutex_unlock(&self->mutex);
}


void rwlock_unlockw (struct rwlock * self) {
    // lock the mutex while we read and write the members of `self`
    pthread_mutex_lock(&self->mutex);

    // decrement the number of active writers since the calling thread is now no longer active
    self->nwriters.active--;

    if (self->nreaders.waiting == 0) {
        if (self->nwriters.waiting > 0) {
            // if there's a thread waiting for writing, send it a signal
            pthread_cond_signal(&self->cond_can_write);
        }
    } else if (self->nreaders.waiting == 1) {
        // tell the one thread it can now read
        pthread_cond_signal(&self->cond_can_read);
    } else if (self->nreaders.waiting > 1) {
        // tell all threads they can now read
        pthread_cond_broadcast(&self->cond_can_read);
    }

    // unlock the mutex now that `self` is back in a consistent state
    pthread_mutex_unlock(&self->mutex);
}
