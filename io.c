/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include "memcpy_sse2.h"
#include "util.h"
#include "io.h"

//#define DEBUG
#include "debug.h"


#define CHUNK_SIZE 1024
#define BUF_MAX_SIZE 8192


unsigned char *io_socket_read(const int sockfd,
                              int *rc)
{
  int n;
  unsigned char buf[CHUNK_SIZE];
  unsigned char workbuf[BUF_MAX_SIZE];
  unsigned char *last;
  int last_sz;

  unsigned char *bytes = NULL;

  workbuf[0] = '\0';
  last = workbuf;
  last_sz = 0;

  /* use loop to read as much as possible in a task */
  do {
    n = recv(sockfd, buf, CHUNK_SIZE, 0);

    /* the client close the socket: EOF reached */
    if (n == 0) {
      *rc = 0;
      return NULL;
    }
    if (n == -1) {
      /* perror("[IO] read()"); */
      /* normally errno = EAGAIN (Resource busy) */
      *rc = -1;
      return NULL;
    }

    memcpy_fast(last, buf, n);
    last += n;
    last_sz += n;

    if (n < CHUNK_SIZE) {
      *last = '\0';
      last_sz++;
      bytes = malloc(last_sz);
      memcpy_fast(bytes, workbuf, last_sz);
      *rc = 1;
      return bytes;
    }

    nsleep(10);
  } while (1);
}

void io_socket_write(const int sockfd,
                     const unsigned char *bytes,
                     const size_t len)
{
  int n;
  const unsigned char *last;
  size_t done_sz;
  size_t left_sz;

  last = bytes;
  done_sz = 0;

  /* use loop to write as much as possible in a task */
  do {
    left_sz = len - done_sz;
    if (left_sz <= 0) return;

    n = send(sockfd, last, left_sz, 0);
    if (n == -1) {
      /* perror("write()") */
      if (errno == EPIPE) return;
      nsleep(10);
      continue;
    }

    last += n;
    done_sz += n;
  } while (1);
}

unsigned char *io_fread(const char *fname,
                        const size_t len)
{
  FILE *f;
  unsigned char *buf;

  buf = malloc(len);
  f = fopen(fname, "r");
  fread(buf, 1, len, f);
  fclose(f);

  return buf;
}

unsigned char *io_fread_pipe(FILE *f,
                             const size_t len)
{
  unsigned char *buf;

  buf = malloc(len);
  fread(buf, 1, len, f);
  fclose(f);

  return buf;
}

char *io_fgetc(FILE *f,
               int *len)
{
  int capacity = 10;
  int index = 0;
  char *buf = malloc(capacity);

  char *newbuf;
  int c;
  while ((c = fgetc(f)) != EOF) {
    assert(index < capacity);
    buf[index++] = c;

    if (index == capacity) {
      newbuf = malloc(capacity << 1);
      memcpy_fast(newbuf, buf, capacity);
      free(buf);
      buf = newbuf;
      capacity *= 2;
    }
  }

  buf[index] = '\0';
  *len = index;
  fclose(f);

  return buf;
}

void io_send_chunk(const int clifd,
                   const char *chunk)
{
  int len_chunk;
  int len;
  unsigned char hex_len[16];

  len_chunk = strlen(chunk);
  len = itos(hex_len, len_chunk, 16, ' ');
  /* chunked length in Hex */
  DEBSI("[IO] Sending chunked length...", clifd);
  io_socket_write(clifd, hex_len, len);
  io_socket_write(clifd, (unsigned char *)"\r\n", 2);
  /* chunk */
  DEBSI("[IO] Sending chunked...", clifd);
  io_socket_write(clifd, (unsigned char *)chunk, len_chunk);
  io_socket_write(clifd, (unsigned char *)"\r\n", 2);
}
