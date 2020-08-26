/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "io.h"


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

char *
io_fread(FILE *f, int len)
{
  char *buf;

  buf = malloc(len);
  fread(buf, 1, len, f);
  fclose(f);

  return buf;
}
