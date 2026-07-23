#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


struct elem {
    void * payload;
    struct elem * next;
};


struct queue {
    pthread_mutex_t mutex;
    pthread_cond_t cond_has_work;
    struct elem * head;
    struct elem * tail;
    int len;
};


struct queue * queue_create (void) {
    struct queue * self = calloc(1, sizeof(struct queue));
    if (self == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for queue, aborting.\n", code);
        exit(code);
    }
    *self = (struct queue) {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond_has_work = PTHREAD_COND_INITIALIZER,
        .head = nullptr,
        .tail = nullptr,
        .len = 0,
    };
    return self;
}


void queue_destroy (struct queue ** self) {
    if ((*self)->len > 0) {
        fprintf(stderr, "Warning: destroying an nonempty queue may result in memory leaks\n");
    }
    struct elem * this = (*self)->head;
    while (this != nullptr) {
        struct elem * next = this->next;
        free(this);
        this = next;
    }
    pthread_mutex_destroy(&(*self)->mutex);
    pthread_cond_destroy(&(*self)->cond_has_work);
    free(*self);
}


void * queue_pop (struct queue * self) {
    pthread_mutex_lock(&self->mutex);
    while (self->len == 0) {
        pthread_cond_wait(&self->cond_has_work, &self->mutex);
    }
    void * rv = self->head->payload;
    struct elem * next = self->head->next;
    free(self->head);
    self->head = next;
    self->len--;
    pthread_mutex_unlock(&self->mutex);
    return rv;
}


void queue_push (struct queue * self, void * item) {
    pthread_mutex_lock(&self->mutex);
    struct elem * elem = calloc(1, sizeof(struct elem));
    if (elem == nullptr) {
        const int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for queue element, aborting.\n", code);
        exit(code);
    }
    *elem = (struct elem) {
        .payload = item,
        .next = nullptr,
    };
    if (self->len == 0) {
        self->head = elem;
    } else {
        self->tail->next = elem;
    }
    self->tail = elem;
    self->len++;
    pthread_cond_signal(&self->cond_has_work);
    pthread_mutex_unlock(&self->mutex);
}
