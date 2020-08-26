/*
 * The original code was developed by pminkov from the following repo
 * https://github.com/pminkov/threadpool
 *
 * I reformatted the code according to my coding style and made some tweeks.
 * pminkov's code was not licensed, but should be acknowledged.
 *
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_


typedef struct _thpool thpool_t;


/* Creates a thread pool and returns a pointer to it */
thpool_t *thpool_init(int max_threads);

/* Return maximum number of threads */
int thpool_get_max_threads(thpool_t *pool);

/*
 * Insert task into thread pool
 * The pool will call work_routine and pass arg as an argument to it
 * This is a similar interface to pthread_create
 */
void thpool_add_task(thpool_t *pool, void (*work_routine)(void *), void *arg);

/* Blocks until the thread pool is done executing its tasks */
void thpool_wait(thpool_t *pool);

/* Cleans up the thread pool, frees memory. Waits until work is done */
void thpool_destroy(thpool_t *pool);


#endif

