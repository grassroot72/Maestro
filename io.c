/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "io.h"

//#define DEBUG
#include "debug.h"


#define BUF_SIZE 1024
#define BUF_MAX_SIZE 8192


unsigned char *
io_read_socket(int sockfd, int *rc)
{
  int n;
  unsigned char buf[BUF_SIZE];
  unsigned char workbuf[BUF_MAX_SIZE];
  unsigned char *last;
  int last_sz;

  unsigned char *bytes = NULL;

  workbuf[0] = '\0';
  last = workbuf;
  last_sz = 0;

  /* use loop to read as much as possible in a task */
  do {
    n = read(sockfd, buf, BUF_SIZE);

    /* the client close the socket: EOF reached */
    if (n == 0) {
      *rc = 0;
      return NULL;
    }
    if (n == -1) {
      /* socket blocked */
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        DEBSI("[CONN] temp error reading socket", sockfd);
        continue;
      }
      else {
        perror("read()");
        *rc = -1;
        return NULL;
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
      *rc = 1;
      return bytes;
    }
  } while (1);
}

void
io_write_socket(int sockfd, unsigned char *bytes, int len)
{
  int n;
  unsigned char *last;
  int done_sz;
  int left_sz;

  last = bytes;
  done_sz = 0;

  /* use loop to write as much as possible in a task */
  do {
    left_sz = len - done_sz;
    if (left_sz == 0) return;

    n = write(sockfd, last, left_sz);
    if (n == -1) continue;
    DEBSI("[IO] write()", n);

    last += n;
    done_sz += n;
  } while (1);
}


unsigned char *
io_fread(FILE *f, int len)
{
  unsigned char *buf;

  buf = malloc(len);
  fread(buf, 1, len, f);
  fclose(f);

  return buf;
}

char *
io_fgetc(FILE *f, int *len)
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
      memcpy(newbuf, buf, capacity);
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
