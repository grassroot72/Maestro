/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "util.h"
#include "linkedlist.h"
#include "thpool.h"
#include "http_msg.h"
#include "http_parser.h"
#include "http_svc.h"
#include "http_conn.h"
#include "debug.h"


#define BUF_SIZE 256
#define BUF_MAX_SIZE 8192


struct _httpconn {
  int sockfd;
  int epfd;
  char *bytes;

  list_t *timers;
};


httpconn_t *
httpconn_new(int sockfd, int epfd, void *timers)
{
  httpconn_t *conn = malloc(sizeof(struct _httpconn));
  conn->sockfd = sockfd;
  conn->epfd = epfd;
  conn->timers = (list_t *)timers;

  return conn;
}

int
httpconn_sockfd(httpconn_t *conn)
{
  return conn->sockfd;
}

static int
_read_socket(httpconn_t *conn)
{
  int n;
  char buf[BUF_SIZE];
  char workbuf[BUF_MAX_SIZE];
  char *last;
  int last_sz;

  char *bytes = NULL;

  workbuf[0] = '\0';
  last = workbuf;
  last_sz = 0;

  /* use loop to read as much as possible in a task */
  do {
    n = read(conn->sockfd, buf, BUF_SIZE);

    if (n == 0) return 0;  /* the client stop sending data: EOF reached */
    if (n == -1) {
      /* socket blocked */
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        DEBSI("[CONN] temp error reading socket", conn->sockfd);
        continue;
      }
      else {
        perror("read()");
        break;
      }
    }

    memcpy(last, buf, n);
    last += n;
    last_sz += n;

    if (n < BUF_SIZE) {
      *last = '\0';
      last_sz++;
      bytes = malloc(last_sz);
      memcpy(bytes, workbuf, last_sz);
      conn->bytes = bytes;
      return 1;
    }
  } while (1);

  return -1;
}

void
httpconn_task(void *arg)
{
  httpconn_t *conn = (struct _httpconn *)arg;
  list_t *timers = conn->timers;
  long cur_time;
  struct epoll_event event;
  int rc;

  char *method;
  char *path;

  httpmsg_t *req;

  rc = _read_socket(conn);

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
    req = http_parse_req(conn->bytes);
    if (!req) return;

    method = msg_method(req);
    path = msg_path(req);

    if (strcmp(method, "GET") == 0) {
      if (strstr(path, "?")) {
        /* dynamic content */
        /* todo: _rep_get_dynamic() */
      }
      else {
        http_rep_get(conn->sockfd, path, req, 0);
        cur_time = mstime();
        list_update(timers, conn, cur_time);
      }
    }

    /* todo:
    if (strcmp(method, "POST") == 0)
    if (strcmp(method, "PUT") == 0)
    if (strcmp(method, "DELETE") == 0)
    */

    if (strcmp(method, "HEAD") == 0)
      http_rep_get(conn->sockfd, path, req, 1);

    msg_destroy(req);
    free(conn->bytes);

    /* put the event back */
    event.data.ptr = (void *)conn;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    rc = epoll_ctl(conn->epfd, EPOLL_CTL_MOD, conn->sockfd, &event);
    if (rc == -1) perror("epoll_ctl()...");
    return;
  }
}
