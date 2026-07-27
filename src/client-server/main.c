#include "request.h"   // struct request
#include "queue.h"     // queue_*
#include <pthread.h>   // pthread_*
#include <stdlib.h>    // EXIT_SUCCESS, calloc, free, exit
#include <stdio.h>     // fprintf, stderr, stdout, snprintf, fflush
#include <string.h>    // strncpy
#include <unistd.h>    // sleep


struct ctx {
    int id;
    struct queue * queue;
};


static void * client (void * arg);
static struct request * create_print_request (const struct ctx * ctx, const struct request * prompt_request, int iloop, int nloops);
static struct request * create_prompt_request (const struct ctx * ctx);
static void create_threads (int nthreads, pthread_t * threads, struct ctx * ctxs);
static void destroy_request (struct request ** request);
static void init_ctxs (int nthreads, struct ctx * ctxs, struct queue * queue);
static void join_threads (int nthreads, pthread_t * threads);


static void * client (void * arg) {

    // sleep random duration
    sleep(rand() % 30);

    // request the server to prompt the user for input
    struct ctx * ctx = (struct ctx *) arg;
    struct request * prompt_request = create_prompt_request(ctx);
    queue_push(ctx->queue, (void *) prompt_request);

    // wait for the request to be done handling by the server (i.e wait for the user's answer)
    while (!prompt_request->sync.has_answer.pred) {
        pthread_cond_wait(&prompt_request->sync.has_answer.cond, &prompt_request->sync.mutex);
    }

    // send 4 separate requests to the server asking to print the user's answer
    const int nloops = 4;
    for (int i = 0; i < nloops; i++) {
        sleep(4);
        struct request * print_request = create_print_request(ctx, prompt_request, i, nloops);
        queue_push(ctx->queue, (void *) print_request);
        // note: server is responsible for destroying async requests
    }

    // free sync request and its members
    destroy_request(&prompt_request);

    return nullptr;
}


static struct request * create_print_request (const struct ctx * ctx, const struct request * prompt_request, int iloop, int nloops) {
    struct request * print_request = calloc(1, sizeof(struct request));
    if (print_request == nullptr) {
        int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for print request in thread id %d, aborting.\n", code, ctx->id);
        exit(code);
    }
    *print_request = (struct request) {
        .operation = OPERATION_ASYNC,
        .async = {
            .buffers = {
                .cap = 128,
                .print = "",
            },
        },
    };
    snprintf(&print_request->async.buffers.print[0], print_request->sync.buffers.cap - 1, "client %1d, loop %1d / %1d: %s\n", ctx->id, iloop + 1, nloops, prompt_request->sync.buffers.answer);
    return print_request;
}


static struct request * create_prompt_request (const struct ctx * ctx) {
    struct request * request = calloc(1, sizeof(struct request));
    if (request == nullptr) {
        int code = __LINE__;
        fprintf(stderr, "ERROR %d: problem allocating dynamic memory for prompt request in thread id %d, aborting.\n", code, ctx->id);
        exit(code);
    }
    *request = (struct request) {
        .operation = OPERATION_SYNC,
        .sync = {
            .mutex = PTHREAD_MUTEX_INITIALIZER,
            .has_answer = {
                .pred = false,
                .cond = PTHREAD_COND_INITIALIZER,
            },
            .buffers = {
                .cap = 128,
                .prompt = "",
                .answer = ""
            },
        }
    };
    snprintf(&request->sync.buffers.prompt[0], request->async.buffers.cap - 1, "client %d wants to know, what is your message?\n >  ", ctx->id);
    return request;
}


static void create_threads (int nthreads, pthread_t * threads, struct ctx * ctxs) {
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], nullptr, client, (void *) &ctxs[i]);
    }
}


static void destroy_request (struct request ** request) {
    if ((*request)->operation == OPERATION_SYNC) {
        pthread_mutex_destroy(&(*request)->sync.mutex);
        pthread_cond_destroy(&(*request)->sync.has_answer.cond);
    }
    free(*request);
    *request = nullptr;
}


static void init_ctxs (int nthreads, struct ctx * ctxs, struct queue * queue) {
    for (int i = 0; i < nthreads; i++) {
        ctxs[i] = (struct ctx) {
            .id = i + 1,
            .queue = queue,
        };
    }
}


static void join_threads (int nthreads, pthread_t * threads) {
    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], nullptr);
    }
}


int main (void) {

    fprintf(stdout,
            "Use multiple client threads to make both synchronous and\n"
            "asynchronous requests to a server.\n");

    int nthreads = 4;
    pthread_t threads[nthreads];
    struct ctx ctxs[nthreads];

    // create the queue that is shared between the clients and the server
    struct queue * queue = queue_create();

    // initialize the context for each thread
    init_ctxs(nthreads, &ctxs[0], queue);

    // create threads
    create_threads(nthreads, &threads[0], &ctxs[0]);

    // receive requests in a loop
    for (int i = 0; i < nthreads * 5; i++) {
        struct request * request = queue_pop(queue);
        switch (request->operation) {
        case OPERATION_ASYNC:
            fprintf(stdout, "%s", request->async.buffers.print);
            fflush(stdout);
            destroy_request(&request);
            break;
        case OPERATION_SYNC:
            pthread_mutex_lock(&request->sync.mutex);
            fprintf(stdout, "%s", request->sync.buffers.prompt);
            fgets(&request->sync.buffers.answer[0], request->sync.buffers.cap - 1, stdin);
            request->sync.has_answer.pred = true;
            pthread_cond_signal(&request->sync.has_answer.cond);
            pthread_mutex_unlock(&request->sync.mutex);
            break;
        default:
            const int code = __LINE__;
            fprintf(stderr, "ERROR %d: Unreachable code, aborting.\n", code);
            exit(code);
        }
    }

    // join threads
    join_threads(nthreads, &threads[0]);

    // clean up dynamic memory associated with queue
    queue_destroy(&queue);

    return EXIT_SUCCESS;
}
