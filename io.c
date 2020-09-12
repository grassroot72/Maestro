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

#include "debug.h"


#define BUF_SIZE 255
#define BUF_MAX_SIZE 8192


char *
io_read_socket(int sockfd, int *rc)
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
io_write_socket(int sockfd, char *bytes, int len)
{
  int n;
  char *last;
  int done_sz;
  int left_sz;

  last = bytes;
  done_sz = 0;

  /* use loop to write as much as possible in a task */
  do {
    left_sz = len - done_sz;
    if (!left_sz) return;

    n = write(sockfd, last, left_sz);

    last += n;
    done_sz += n;
  } while (1);
}


char *
io_fread(FILE *f, int len)
{
  char *buf;

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
