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
#include <libpq-fe.h>
#include "util.h"
#include "io.h"
#include "linkedlist.h"
#include "http_msg.h"
#include "http_parser.h"
#include "http_get.h"
#include "http_post.h"
#include "http_conn.h"

//#define DEBUG
#include "debug.h"


httpconn_t *httpconn_new(const int sockfd,
                         const int epfd,
                         PGconn *pgconn,
                         list_t *cache,
                         list_t *timers)
{
  httpconn_t *conn = malloc(sizeof(struct _httpconn));
  conn->sockfd = sockfd;
  conn->epfd = epfd;
  conn->pgconn = pgconn;
  conn->cache = cache;
  conn->timers = timers;

  return conn;
}

void httpconn_task(void *arg)
{
  httpconn_t *conn = (struct _httpconn *)arg;
  long cur_time;
  struct epoll_event event;

  unsigned char *bytes;
  int rc;

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

    /* static GET */
    if (strcmp(req->method, "GET") == 0) {
      http_get(conn->sockfd, conn->cache, req->path, req, METHOD_GET);
    }

    if (strcmp(req->method, "POST") == 0) {
      http_post(conn->sockfd, conn->pgconn, req->path, req);
    }

    /* todo:
    if (strcmp(method, "PUT") == 0)
    if (strcmp(method, "DELETE") == 0)
    */

    if (strcmp(req->method, "HEAD") == 0) {
      http_get(conn->sockfd, conn->cache, req->path, req, METHOD_HEAD);
    }

    /* start timer recording */
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
