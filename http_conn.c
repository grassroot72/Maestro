/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include "util.h"
#include "io.h"
#include "linkedlist.h"
#include "http_msg.h"
#include "http_parser.h"
#include "http_svc.h"
#include "http_conn.h"

#define DEBUG
#include "debug.h"


struct _httpconn {
  int sockfd;
  int epfd;

  list_t *cache;
  list_t *timers;
};


httpconn_t *
httpconn_new(int sockfd, int epfd, void *cache, void *timers)
{
  httpconn_t *conn = malloc(sizeof(struct _httpconn));
  conn->sockfd = sockfd;
  conn->epfd = epfd;
  conn->cache = (list_t *)cache;
  conn->timers = (list_t *)timers;

  return conn;
}

int
httpconn_sockfd(httpconn_t *conn)
{
  return conn->sockfd;
}

void
httpconn_task(void *arg)
{
  httpconn_t *conn = (struct _httpconn *)arg;
  long cur_time;
  struct epoll_event event;

  char *bytes;
  int rc;

  char *method;
  char *path;

  httpmsg_t *req;

  bytes = io_read_socket(conn->sockfd, &rc);

  /* rc = 0:  the client has closed the connection */
  if (rc == 0) {
    DEBSI("[CONN] client disconnected", conn->sockfd);
    return;
  }

  /* rc = -1: something wrong with the socket */
  if (rc == -1) {
    DEBSI("[CONN] sock error", conn->sockfd);
    return;
  }

  if (rc == 1) {
    req = http_parse_req(bytes);
    if (!req) return;

    method = msg_method(req);
    path = msg_path(req);

    /* static GET */
    if (strcmp(method, "GET") == 0) {
      http_rep_get(conn->sockfd, conn->cache, path, req);
    }

    /* todo:
    if (strcmp(method, "POST") == 0)
    if (strcmp(method, "PUT") == 0)
    if (strcmp(method, "DELETE") == 0)
    */

    if (strcmp(method, "HEAD") == 0) {
      http_rep_head(conn->sockfd, conn->cache, path, req);
    }

    /* start timer recoding */
    cur_time = mstime();
    list_update(conn->timers, conn, cur_time);
    msg_destroy(req, 1);
    free(bytes);

    /* put the event back */
    event.data.ptr = (void *)conn;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    rc = epoll_ctl(conn->epfd, EPOLL_CTL_MOD, conn->sockfd, &event);
    if (rc == -1) perror("epoll_ctl()...");
    return;
  }
}
