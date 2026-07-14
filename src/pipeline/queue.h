#ifndef PIPELINE_QUEUE_INCLUDED
#define  PIPELINE_QUEUE_INCLUDED

struct queue;

void queue_append (struct queue * self, void * item);
struct queue * queue_create (int cap);
void queue_destroy (struct queue ** self);
void * queue_pop (struct queue * self);

#endif
