#include "thread_pool.h"

void thread_pool_init(pthread_t pool[] ,int pool_limit, void *pfunc(void *), void *parg)
{
    for (int i = 0; i < pool_limit; i++)
    {
        if (pthread_create(&pool[i], NULL, pfunc, parg) != 0)
        {
            fprintf(stderr, "[ERR] thread_pool_init: \
                    unable to create thread\n") ;
        }
    }
}

void thread_pool_join(pthread_t pool[], int pool_limit)
{
    for (int i = 0; i < pool_limit; i++)
    {
        if (pthread_join(pool[i], NULL) != 0)
        {
            fprintf(stderr, "[ERR] thread_pool_join:\
                    unable to join thread\n");
        }
    }
}
