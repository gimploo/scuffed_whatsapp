#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#include <pthread.h>
#include <stdio.h>

#define SIZEOF(arr) (sizeof(arr)/sizeof(*arr))

// Inititalizes the thread pool
void thread_pool_init(pthread_t pool[] ,int pool_limit, void *pfunc(void *), void *parg);

// Waits for thread completion
void thread_pool_join(pthread_t pool[], int pool_limit);

#endif
