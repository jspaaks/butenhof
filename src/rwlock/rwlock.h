#ifndef RWLOCK_INCLUDED
#define RWLOCK_INCLUDED

struct rwlock;

struct rwlock * rwlock_create (void);
int rwlock_destroy (struct rwlock ** self);
void rwlock_lockr (struct rwlock * self);
void rwlock_lockw (struct rwlock * self);
void rwlock_unlockr (struct rwlock * self);
void rwlock_unlockw (struct rwlock * self);

#endif
