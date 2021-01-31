/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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

static void _set_nonblocking(const int fd)
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
                           const long timeout)
{
  node_t *timer = list_first(timers);
  if (timer) {
    long cur_time = mstime();
    do {
      if (cur_time - timer->stamp >= timeout) {
        httpconn_t *conn = (httpconn_t *)timer->data;

        D_PRINT("[CONN] socket %d closed from server\n", conn->sockfd);
        shutdown(conn->sockfd, SHUT_RDWR);
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
                          const long timeout)
{
  node_t *node = list_first(cache);
  if (node) {
    long cur_time = mstime();
    do {
      if (cur_time - node->stamp >= timeout) {
        cache_data_t *data = (cache_data_t *)node->data;
        http_cache_data_destroy(data);
        D_PRINT("[CACHE] cached data expired!\n");

        list_del(cache, node->stamp);
        node = list_next(cache);
      }
      else
        node = list_next(cache);
    } while (node);
  }
}

static void _receive_conn(const int srvfd,
                          const int epfd,
                          PGconn *pgconn,
                          list_t *cache,
                          list_t *timers)
{
  struct sockaddr cliaddr;
  socklen_t len_cliaddr = sizeof(struct sockaddr);

   /* server socket; accept connections */
  do {
    int clifd = accept(srvfd, &cliaddr, &len_cliaddr);

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

    char *cli_ip = inet_ntoa(((struct sockaddr_in *)&cliaddr)->sin_addr);
    D_PRINT("[CONN] client %s connected on socket %d\n", cli_ip, clifd);

    _set_nonblocking(clifd);
    httpconn_t *cliconn = httpconn_new(clifd, epfd, pgconn, cache, timers);

    /* register timers */
    long cur_time = mstime();
    list_update(timers, cliconn, cur_time);

    struct epoll_event event;
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
  int srvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (srvfd == -1) {
    perror("socket()");
    return -1;
  }

  int opt = 1;
  if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("setsockopt()");
    return -1;
  }
  return srvfd;
}

static int _bind(const int srvfd,
                 const uint16_t port)
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

static int _listen(const int srvfd)
{
  _set_nonblocking(srvfd);
  if (listen(srvfd, SOMAXCONN) < 0) {
    perror("listen()");
    return -1;
  }
  return 1;
}

int main(int argc, char **argv)
{
  /* create a postgresql db connection */
  PGconn *pgconn = pg_connect("dbname = demo", "identity");

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
    D_PRINT("install sigal handler for SIGPIPE failed\n");
    return 0;
  }

  /* ctrl-c handler */
  signal(SIGINT, _svc_stopper);

  /* detect number of cpu cores and use it for thread pool */
  int np = get_nprocs();
  thpool_t *taskpool = thpool_init(np * THREADS_PER_CORE);
  /* list of files cached in the memory */
  list_t *cache = list_new();
  /* list of timers */
  list_t *timers = list_new();
  /* loop time */
  long loop_time = mstime();


  /* create the server socket */
  int srvfd = _create_srv_socket();
  /* bind */
  _bind(srvfd, PORT);
  /* make it nonblocking, and then listen */
  if (_listen(srvfd)) {
    printf("listening on port [%d]\n", PORT);
  }
  else {
    printf("listening on port [%d] failed!!!\n", PORT);
    exit(1);
  }

  /* create the epoll socket */
  int epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1()");
    return -1;
  }

  /* mark the server socket for reading, and become edge-triggered */
  struct epoll_event event;
  memset(&event, 0, sizeof(struct epoll_event));
  httpconn_t *srvconn = httpconn_new(srvfd, epfd, NULL, NULL, NULL);
  event.data.ptr = (void *)srvconn;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, srvfd, &event) == -1) {
    perror("epoll_ctl()");
    return -1;
  }

  struct epoll_event *events = calloc(MAXEVENTS, sizeof(struct epoll_event));

  do {
    int nevents = epoll_wait(epfd, events, MAXEVENTS, EPOLL_TIMEOUT);
    if (nevents == -1) {
      if (errno == EINTR) continue;
      perror("epoll_wait()");
    }

    if ((mstime() - loop_time) >= EPOLL_TIMEOUT) {
      /* expire the timers */
      _expire_timers(timers, HTTP_KEEPALIVE_TIME);
      /* expire the cache */
      _expire_cache(cache, MAX_CACHE_TIME);

      loop_time = mstime();
    }

    /* loop through events */
    int i = 0;
    do {
      httpconn_t *conn = (httpconn_t *)events[i].data.ptr;

      /* error case */
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        if (errno == EAGAIN)
          nsleep(10);
        else {
          perror("[EPOLL ERR|HUP]");
          break;
        }
      }

      if (events[i].events & EPOLLIN) {
        if (conn->sockfd == srvfd)
          _receive_conn(srvfd, epfd, pgconn, cache, timers);
        else {
          /* client socket; read client data and process it */
          thpool_add_task(taskpool, httpconn_task, conn);
        }
      }

      i++;
    } while (i < nevents);
  } while (svc_running);

  thpool_wait(taskpool);
  /*
   * glibc doesn't free thread stacks when threads exit;
   * it caches them for reuse, and only prunes the cache when it gets huge.
   * Thus it always "leaks" some memory.
   *
   * So, don't worry about it.
   */
  thpool_destroy(taskpool);

  list_destroy(timers);
  _expire_cache(cache, 0);
  list_destroy(cache);

  shutdown(srvfd, SHUT_RDWR);
  close(srvfd);
  if (srvconn) free(srvconn);
  close(epfd);
  free(events);

  PQfinish(pgconn);

  D_PRINT("Exit gracefully...\n");
  return 0;
}
