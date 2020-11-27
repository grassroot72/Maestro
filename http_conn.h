/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_


typedef struct _httpconn httpconn_t;

struct _httpconn {
  int sockfd;
  int epfd;

  PGconn *pgconn;
  list_t *cache;
  list_t *timers;
};


httpconn_t *httpconn_new(int sockfd,
                         int epfd,
                         PGconn *pgconn,
                         list_t *cache,
                         list_t *timers);

void httpconn_task(void *arg);


#endif
