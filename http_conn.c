/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <libpq-fe.h>
#include "util.h"
#include "io.h"
#include "rbtree.h"
#include "linkedlist.h"
#include "http_msg.h"
#include "http_parser.h"
#include "http_get.h"
#include "http_post.h"
#include "http_conn.h"

#define DEBUG
#include "debug.h"


httpconn_t *httpconn_new(const int sockfd,
                         const int epfd,
                         PGconn *pgconn,
                         list_t *cache,
                         rbtree_t *timers)
{
  httpconn_t *conn = malloc(sizeof(struct _httpconn));
  conn->sockfd = sockfd;
  conn->epfd = epfd;
  conn->pgconn = pgconn;
  conn->cache = cache;
  conn->timers = timers;

  return conn;
}

int httpconn_compare(const void *conn1,
                     const void *conn2)
{
  httpconn_t *p1, *p2;

  p1 = (httpconn_t *)conn1;  // data to be compared
  p2 = (httpconn_t *)conn2;  // current data

  if (p1->sockfd > 32720 || p1->sockfd < 0) return 0;

  if (p1 != p2) {
    if (p1->stamp >= p2->stamp) {
      D_PRINT("[CONN] new insert GT %d %ld\n", p1->sockfd, p1->stamp);
      return 1;
    }
    else {
      D_PRINT("[CONN] new insert LE %d %ld\n", p1->sockfd, p1->stamp);
      return -1;
    }
  }
  else {
    if (p1->stamp > p2->stamp) {
      p2->stamp = p1->stamp;
      D_PRINT("[CONN] updated %d\n", p2->sockfd);
      return 0;
    }
    else {
      D_PRINT("[CONN] same stamp %d\n", p2->sockfd);
      return 0;
    }
  }
}

void httpconn_destroy(void *conn)
{
  if (!conn) return;

  httpconn_t *p;
  p = (httpconn_t *)conn;
  if (p->sockfd < 32720 && p->sockfd >= 0) {
    printf("[CONN] socket %d closed from server\n", p->sockfd);
    shutdown(p->sockfd, SHUT_RDWR);
    close(p->sockfd);
    p->sockfd = -1;
    free(p);
  }
}


void httpconn_task(void *arg)
{
  httpconn_t *conn = (struct _httpconn *)arg;
  int rc;
  unsigned char *bytes = io_socket_read(conn->sockfd, &rc);

  /* rc = 0:  the client has closed the connection */
  if (rc == 0) {
    D_PRINT("[CONN] client disconnected: %d\n", conn->sockfd);
    return;
  }

  /* rc = -1: EAGAIN (Resource busy) */
  if (rc == -1) {
    /* D_PRINT("[CONN] sock error: %d\n", conn->sockfd); */
    return;
  }

  if (rc == 1) {
    /* D_PRINT("[CONN] raw bytes: %s\n", bytes); */
    httpmsg_t *req = http_parse_req(bytes);

    if (!req) return;

    /* static GET */
    if (req->method == METHOD_GET || req->method == METHOD_HEAD) {
      http_get(conn->sockfd, conn->cache, req->path, req);
    }

    if (req->method == METHOD_POST) {
      http_post(conn->sockfd, conn->pgconn, req->path, req);
    }

    /* start timer recording */
    conn->stamp = mstime();
    rbt_insert(conn->timers, conn);
    msg_destroy(req, 1);
    free(bytes);

    /* put the event back */
    struct epoll_event event;
    event.data.ptr = (void *)conn;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    rc = epoll_ctl(conn->epfd, EPOLL_CTL_MOD, conn->sockfd, &event);
    if (rc == -1) perror("epoll_ctl()...");
    return;
  }
}
