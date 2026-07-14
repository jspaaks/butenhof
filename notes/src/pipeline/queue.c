#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


struct queue {
    int cap;
    pthread_cond_t cond_has_items;
    pthread_cond_t cond_has_space;
    int head;
    void ** items;
    int len;
    pthread_mutex_t mutex;
};


void queue_append (struct queue * self, void * item) {
    pthread_mutex_lock(&self->mutex);
    while (self->len == self->cap) {
        pthread_cond_wait(&self->cond_has_space, &self->mutex);
    }
    int i = (self->head + self->len) % self->cap;
    self->items[i] = item;
    self->len++;
    pthread_cond_signal(&self->cond_has_items);
    pthread_mutex_unlock(&self->mutex);
}


struct queue * queue_create (int cap) {
    struct queue * self = calloc(1, sizeof(struct queue));
    if (self == nullptr) {
        fprintf(stderr, "Problem allocating dynamic memory for queue, aborting.\n");
        exit(1);
    }
    *self = (struct queue) {
        .cap = cap,
        .cond_has_items = PTHREAD_COND_INITIALIZER,
        .cond_has_space = PTHREAD_COND_INITIALIZER,
        .head = 0,
        .items = calloc(cap, sizeof(void *)),
        .len = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
    };
    if (self->items == nullptr) {
        fprintf(stderr, "Problem allocating dynamic memory for queue items, aborting.\n");
        exit(1);
    }
    return self;
}


void queue_destroy (struct queue ** self) {
    free((*self)->items);
    (*self)->items = nullptr;

    free(*self);
    *self = nullptr;
}


void * queue_pop (struct queue * self) {
    pthread_mutex_lock(&self->mutex);
    while (self->len == 0) {
        pthread_cond_wait(&self->cond_has_items, &self->mutex);
    }
    void * item = self->items[self->head];
    self->head = (self->head + 1) % self->cap;
    self->len--;
    pthread_cond_signal(&self->cond_has_space);
    pthread_mutex_unlock(&self->mutex);
    return item;
}
