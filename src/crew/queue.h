#ifndef CREW_QUEUE_INCLUDED
#define CREW_QUEUE_INCLUDED

struct queue;

struct queue * queue_create (void);
void queue_destroy (struct queue ** self);
void * queue_pop (struct queue * self);
void queue_push (struct queue * self, void * item);


#endif
