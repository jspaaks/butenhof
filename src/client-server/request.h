#ifndef CLIENT_SERVER_REQUEST_INCLUDED
#define CLIENT_SERVER_REQUEST_INCLUDED
#include <pthread.h>

// define request as a union betweem the properties of a synchronous request and an asynchronous
// request; use the member `operation` to know which one you're dealing with at any given time

struct request {
    enum {OPERATION_SYNC, OPERATION_ASYNC} operation;
    union {
        struct {
            pthread_mutex_t mutex;
            struct {
                pthread_cond_t cond;
                bool pred;
            } has_answer;
            struct {
                char prompt[128];
                char answer[128];
                int cap;
            } buffers;
        } sync;
        struct {
            struct {
                char print[128];
                int cap;
            } buffers;
        } async;
    };
};

#endif
