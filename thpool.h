/*
 * The original code was developed by pminkov from the following repo
 * https://github.com/pminkov/threadpool
 *
 * I reformatted the code according to my coding style and made some tweaks.
 * pminkov's code was not licensed, but should be acknowledged.
 *
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_


struct _taskdata {
  void (*work_routine)(void *);
  void *arg;
};


typedef struct _thpool thpool_t;

struct _thpool {
  /*
   * can be used to set the thread PTHREAD_CREATE_DETACHED attribute,
   * or set the stack size of a thread
   * ex. pthread_attr_setstacksize(&attr, SMALL_STACK);
   */
  pthread_attr_t *attr;
  /* N worker threads */
  pthread_t *worker_threads;

  /* A circular queue that holds tasks that are yet to be executed */
  struct _taskdata *task_queue;

  /* Head and tail of the queue */
  int queue_head, queue_tail;

  /* How many worker threads can we have */
  int max_threads;

  /*
   * How many tasks are scheduled for execution
   * We use this so that we can wait for completion
   */
  int scheduled;

  pthread_mutex_t mutex;

  /*
   * A condition that's signaled on when we go:
   * from a state of no work to a state of work available
   */
  pthread_cond_t work_available;

  /* A condition that's signaled on no more tasks scheduled */
  pthread_cond_t done;
};

/* Creates a thread pool and returns a pointer to it */
thpool_t *thpool_init(const int max_threads);

/*
 * Insert task into thread pool
 * The pool will call work_routine and pass arg as an argument to it
 * This is a similar interface to pthread_create
 */
void thpool_add_task(thpool_t *pool,
                     void (*work_routine)(void *),
                     void *arg);

/* Blocks until the thread pool is done executing its tasks */
void thpool_wait(thpool_t *pool);

/* Cleans up the thread pool, frees memory. Waits until work is done */
void thpool_destroy(thpool_t *pool);


#endif
