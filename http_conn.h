/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_


typedef struct _httpconn httpconn_t;

struct _httpconn {
  int sockfd;
  int epfd;
  long stamp;  /* timestamp */

  PGconn *pgconn;
  list_t *cache;
  rbtree_t *timers;
};


httpconn_t *httpconn_new(const int sockfd,
                         const int epfd,
                         PGconn *pgconn,
                         list_t *cache,
                         rbtree_t *timers);

int httpconn_compare(const void *conn1,
                     const void *conn2);

void httpconn_destroy(void *conn);

void httpconn_task(void *arg);


#endif
