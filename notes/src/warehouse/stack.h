#ifndef WAREHOUSE_STACK_INCLUDED
#define  WAREHOUSE_STACK_INCLUDED

struct stack;

void stack_add_item (struct stack * self, int item_id);
struct stack * stack_create (int cap);
void stack_destroy (struct stack ** self);
int stack_pop_item (struct stack * self);
void stack_print (struct stack * self);

#endif

