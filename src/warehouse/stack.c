#include "stack.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


struct stack {
    int * storage;
    int cap;
    int len;
    pthread_mutex_t mutex;
    pthread_cond_t cond_has_space;
    pthread_cond_t cond_has_items;
};


void stack_add_item (struct stack * self, int item_id) {
    pthread_mutex_lock(&self->mutex);
    while (self->len == self->cap) {
        // wait for conditions to improve
        fprintf(stdout, "out of space, waiting for items to be popped...\n");
        pthread_cond_wait(&self->cond_has_space, &self->mutex);
    }
    // there is space now, add item
    self->storage[self->len] = item_id;
    self->len++;
    pthread_cond_signal(&self->cond_has_items);
    pthread_mutex_unlock(&self->mutex);
    return;
}


struct stack * stack_create (int cap) {
    struct stack * self = calloc(1, sizeof(struct stack));
    if (self == nullptr) {
        fprintf(stderr, "Error allocating dynamic memory for stack, aborting.\n");
        exit(1);
    }
    self->storage = calloc(cap, sizeof(int));
    if (self->storage == nullptr) {
        fprintf(stderr, "Error allocating dynamic memory for stack storage, aborting.\n");
        exit(2);
    }
    self->cap = cap;
    self->len = 0;
    pthread_mutex_init(&self->mutex, nullptr);
    pthread_cond_init(&self->cond_has_items, nullptr);
    pthread_cond_init(&self->cond_has_space, nullptr);
    return self;
}


void stack_destroy (struct stack ** self) {
    free((*self)->storage);
    (*self)->storage  = nullptr;
    pthread_mutex_destroy(&(*self)->mutex);
    pthread_cond_destroy(&(*self)->cond_has_items);
    pthread_cond_destroy(&(*self)->cond_has_space);
    free(*self);
    *self = nullptr;
}


int stack_pop_item (struct stack * self) {
    pthread_mutex_lock(&self->mutex);
    while (self->len == 0) {
        // wait for conditions to improve
        fprintf(stdout, "out of stock, waiting for items to be added...\n");
        pthread_cond_wait(&self->cond_has_items, &self->mutex);
    }
    int item_id = self->storage[self->len - 1];
    self->len--;
    pthread_cond_signal(&self->cond_has_space);
    pthread_mutex_unlock(&self->mutex);
    return item_id;
}


void stack_print (struct stack * self) {
    pthread_mutex_lock(&self->mutex);
    for (int i = 0; i < self->len; i++) {
        fprintf(stdout, "%3d%c", self->storage[i], i == self->cap - 1 ? '\n' : ' ');
    }
    for (int i = self->len; i < self->cap; i++) {
        fprintf(stdout, "___%c", i == self->cap - 1 ? '\n' : ' ');
    }
    pthread_mutex_unlock(&self->mutex);
}
