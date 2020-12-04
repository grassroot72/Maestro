/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libpq-fe.h>
#include "pg_conn.h"
#include "util.h"
#include "linkedlist.h"
#include "thpool.h"
#include "http_cache.h"
#include "http_conn.h"

#define DEBUG
#include "debug.h"


#define THREADS_PER_CORE 128
#define MAXEVENTS 2048

#define EPOLL_TIMEOUT 1000         /* 1 second */
#define HTTP_KEEPALIVE_TIME 72000  /* 72 seconds */
#define PORT 9000

#define MAX_CACHE_TIME 86400000    /* 24 x 60 x 60 = 1 day */


static volatile int svc_running = 1;


static void _svc_stopper(int dummy)
{
  svc_running = 0;
}

static void _set_nonblocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl()");
    return;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    perror("fcntl()");
}

static void _expire_timers(list_t *timers,
                           long timeout)
{
  httpconn_t *conn;
  node_t *timer;
  long cur_time;

  timer = list_first(timers);
  if (timer) {
    cur_time = mstime();
    do {
      if (cur_time - timer->stamp >= timeout) {
        conn = (httpconn_t *)timer->data;

        DEBSI("[CONN] socket closed from server", conn->sockfd);
        close(conn->sockfd);
        free(conn);

        list_del(timers, timer->stamp);
        timer = list_next(timers);
      }
      else
        timer = list_next(timers);
    } while (timer);
  }
}

static void _expire_cache(list_t *cache,
                          long timeout)
{
  cache_data_t *data;
  node_t *node;
  long cur_time;

  node = list_first(cache);
  if (node) {
    cur_time = mstime();
    do {
      if (cur_time - node->stamp >= timeout) {
        data = (cache_data_t *)node->data;
        http_cache_data_destroy(data);
        DEBS("[CACHE] cached data expired");

        list_del(cache, node->stamp);
        node = list_next(cache);
      }
      else
        node = list_next(cache);
    } while (node);
  }
}

static void _receive_conn(int srvfd,
                          int epfd,
                          PGconn *pgconn,
                          list_t *cache,
                          list_t *timers)
{
  int clifd;
  struct sockaddr cliaddr;
  socklen_t len_cliaddr = sizeof(struct sockaddr);
  char *cli_ip;

  httpconn_t *cliconn;
  struct epoll_event event;

  /* server socket; accept connections */
  do {
    clifd = accept(srvfd, &cliaddr, &len_cliaddr);

    if (clifd == -1) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        /* we processed all of the connections */
        break;
      }
      perror("accept()");
      close(clifd);
      break;
    }

    cli_ip = inet_ntoa(((struct sockaddr_in *)&cliaddr)->sin_addr);
    DEBSS("[CONN] client connected", cli_ip);
    DEBSI("[CONN] on socket", clifd);

    _set_nonblocking(clifd);
    cliconn = httpconn_new(clifd, epfd, pgconn, cache, timers);
    event.data.ptr = (void *)cliconn;
    /*
     * With the use of EPOLLONESHOT, it is guaranteed that
     * a client file descriptor is only used by one thread
     * at a time
     */
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clifd, &event) == -1) {
      perror("epoll_ctl()");
      return;
    }
  } while (1);
}

static int _create_srv_socket()
{
  int srvfd;
  int opt = 1;

  srvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (srvfd == -1) {
    perror("socket()");
    return -1;
  }

  if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("setsockopt()");
    return -1;
  }

  return srvfd;
}

static int _bind(int srvfd,
                 uint16_t port)
{
  struct sockaddr_in srvaddr;

  memset(&srvaddr, 0, sizeof(struct sockaddr_in));
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  srvaddr.sin_port = htons(port);

  if (bind(srvfd, (struct sockaddr*)&srvaddr, sizeof(struct sockaddr_in)) < 0) {
    perror("bind()");
    return -1;
  }

  return 0;
}

static int _listen(int srvfd)
{
  _set_nonblocking(srvfd);
  if (listen(srvfd, SOMAXCONN) < 0) {
    perror("listen()");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  PGconn *pgconn;

  int srvfd;
  int i;

  int epfd;
  int nevents;
  struct epoll_event event;
  struct epoll_event *events;

  httpconn_t *srvconn;
  httpconn_t *conn;

  int np;
  thpool_t *taskpool;

  list_t *cache;
  list_t *timers;
  long loop_time;

  /* create a postgresql db connection */
  pgconn = pg_connect("dbname = demo", "identity");


  /*
   * install signal handle for SIGPIPE
   * when a fd is closed by remote, writing to this fd will cause system
   * send SIGPIPE to this process, which exit the program
   */
  struct sigaction sa;
  memset(&sa, '\0', sizeof(struct sigaction));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, 0)) {
    DEBS("install sigal handler for SIGPIPE failed");
    return 0;
  }

  /* ctrl-c handler */
  signal(SIGINT, _svc_stopper);

  /* detect number of cpu cores and use it for thread pool */
  np = get_nprocs();
  taskpool = thpool_init(np * THREADS_PER_CORE);
  /* list of files cached in the memory */
  cache = list_new();
  /* list of timers */
  timers = list_new();
  /* loop time */
  loop_time = mstime();


  /* create the server socket */
  srvfd = _create_srv_socket();
  /* bind */
  _bind(srvfd, PORT);
  /* make it nonblocking, and then listen */
  _listen(srvfd);
  printf("listening on port [%d]\n", PORT);


  /* create the epoll socket */
  epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1()");
    return -1;
  }

  /* mark the server socket for reading, and become edge-triggered */
  memset(&event, 0, sizeof(struct epoll_event));
  srvconn = httpconn_new(srvfd, epfd, NULL, NULL, NULL);
  event.data.ptr = (void *)srvconn;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, srvfd, &event) == -1) {
    perror("epoll_ctl()");
    return -1;
  }

  events = calloc(MAXEVENTS, sizeof(struct epoll_event));

  do {
    nevents = epoll_wait(epfd, events, MAXEVENTS, EPOLL_TIMEOUT);
    if (nevents == -1) perror("epoll_wait()");

    if ((mstime() - loop_time) >= EPOLL_TIMEOUT) {
      /* expire the timers */
      _expire_timers(timers, HTTP_KEEPALIVE_TIME);
      /* expire the cache */
      _expire_cache(cache, MAX_CACHE_TIME);

      loop_time = mstime();
    }

    /* loop through events */
    for (i = 0; i < nevents; i++) {
      conn = (httpconn_t *)events[i].data.ptr;

      /* error case */
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
          (!(events[i].events & EPOLLIN))) {
        if (errno != EAGAIN) {
          perror("[EPOLL] ERR|HUP");
        }
        list_update(timers, conn, mstime());
        break;
      }

      else if (conn->sockfd == srvfd) {
        _receive_conn(srvfd, epfd, pgconn, cache, timers);
      }

      else {
        /* client socket; read client data and process it */
        thpool_add_task(taskpool, httpconn_task, conn);
      }
    }
  } while (svc_running);

  thpool_wait(taskpool);
  thpool_destroy(taskpool);

  list_destroy(timers);
  _expire_cache(cache, 0);
  list_destroy(cache);

  free(srvconn);
  close(epfd);
  free(events);

  puts("Exit gracefully...");

  return 0;
}
