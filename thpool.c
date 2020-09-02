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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "thpool.h"

#include "debug.h"


const int TASK_QUEUE_MAX = 10000;


struct _taskdata {
  void (*work_routine)(void *);
  void *arg;
};

struct _thpool {
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


int
thpool_get_max_threads(struct _thpool *pool)
{
  return pool->max_threads;
}

static void *
_worker_func(void *pool_arg)
{
  int rc;
  struct _taskdata picked_task;

  DEBS("[W] Starting work thread.");
  struct _thpool *pool = (struct _thpool *)pool_arg;

  do {
    /* lock */
    rc = pthread_mutex_lock(&pool->mutex);
    assert(rc == 0);

    while (pool->queue_head == pool->queue_tail) {
      DEBS("[W] Empty queue. Waiting...");
      rc = pthread_cond_wait(&pool->work_available, &pool->mutex);
      assert(rc == 0);
    }

    assert(pool->queue_head != pool->queue_tail);
    DEBSI("[W] Picked", pool->queue_head);
    picked_task = pool->task_queue[pool->queue_head % TASK_QUEUE_MAX];
    pool->queue_head++;

    /* The task is scheduled */
    pool->scheduled++;

    /* unlock */
    rc = pthread_mutex_unlock(&pool->mutex);
    assert(rc == 0);

    /* Run the task */
    picked_task.work_routine(picked_task.arg);

    /* lock */
    rc = pthread_mutex_lock(&pool->mutex);
    assert(rc == 0);
    pool->scheduled--;

    if (pool->scheduled == 0) {
      rc = pthread_cond_signal(&pool->done);
      assert(rc == 0);
    }

    /* unlock */
    rc = pthread_mutex_unlock(&pool->mutex);
    assert(rc == 0);
  } while (1);

  return 0;
}

void
thpool_add_task(struct _thpool *pool, void (*work_routine)(void *), void *arg)
{
  int rc;
  struct _taskdata task;

  /* lock */
  rc = pthread_mutex_lock(&pool->mutex);
  assert(rc == 0);

  DEBS("[Q] Queueing one item.");
  if (pool->queue_head == pool->queue_tail) {
    rc = pthread_cond_broadcast(&pool->work_available);
    assert(rc == 0);
  }

  task.work_routine = work_routine;
  task.arg = arg;

  pool->task_queue[pool->queue_tail % TASK_QUEUE_MAX] = task;
  pool->queue_tail++;

  /* unlock */
  rc = pthread_mutex_unlock(&pool->mutex);
  assert(rc == 0);
}

void
thpool_wait(struct _thpool *pool)
{
  int rc;

  DEBS("[POOL] Waiting for completion.");
  rc = pthread_mutex_lock(&pool->mutex);
  assert(rc == 0);

  while (pool->scheduled > 0) {
    rc = pthread_cond_wait(&pool->done, &pool->mutex);
    assert(rc == 0);
  }

  rc = pthread_mutex_unlock(&pool->mutex);
  assert(rc == 0);
  DEBS("[POOL] Waiting done.");
}

struct _thpool *
thpool_init(int max_threads)
{
  int rc;
  int i;

  struct _thpool *pool = malloc(sizeof(struct _thpool));

  pool->queue_head = pool->queue_tail = 0;
  pool->scheduled = 0;
  pool->task_queue = malloc(sizeof(struct _taskdata) * TASK_QUEUE_MAX);

  pool->max_threads = max_threads;
  pool->worker_threads = malloc(sizeof(pthread_t) * max_threads);

  rc = pthread_mutex_init(&pool->mutex, NULL);
  assert(rc == 0);
  rc = pthread_cond_init(&pool->work_available, NULL);
  assert(rc == 0);
  rc = pthread_cond_init(&pool->done, NULL);
  assert(rc == 0);

  for (i = 0; i < max_threads; i++) {
    rc = pthread_create(&pool->worker_threads[i], NULL, _worker_func, pool);
    assert(rc == 0);
  }

  return pool;
}

void
thpool_destroy(struct _thpool *pool)
{
  int rc;
  int i;

  for (i = 0; i < pool->max_threads; i++) {
    rc = pthread_detach(pool->worker_threads[i]);
    assert(rc == 0);
  }

  free(pool->worker_threads);
  free(pool->task_queue);

  free(pool);
}
