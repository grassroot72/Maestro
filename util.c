/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "util.h"


/*
 * outbuf -  buffer to hold the output number
 * n      -  number to be converted
 * base   -  number base for conversion;  decimal=10,hex=16
 * sign   -  sign bit set in output? ex. '+' or '-', ' ' if not set
 */
void itos(unsigned char *outbuf,
          unsigned long n,
          const int base,
          const char sign)
{
  int i = 12;
  int j = 0;

  do {
    outbuf[i] = "0123456789ABCDEF"[n % base];
    i--;
    n = n/base;
  } while (n > 0);

  if (sign != ' ') {
    outbuf[0] = sign;
    ++j;
  }

  while (++i < 13) {
    outbuf[j++] = outbuf[i];
  }

  outbuf[j] = '\0';
}

char *split_kv(char *kv,
               char delim)
{
  char *p = kv;
  do {
    if (*p == delim) {
      /*
       *  xxxxxxxxxxxx: xxxxxxxxxxxx\0
       *  ^           ^              ^
       *  h           p              p
       */
      *p = '\0';
      p++;
      break;
    }
    p++;
  } while (*p);

  do {
    /* assume that there is no 2nd ':' */
    if (*p != ' ' && *p != '\t') break;
    p++;
  } while (*p);

  return p;  /* return the value */
}

void gmt_date(char *date_gmt,
              const long *tmgmt)
{
  struct tm tm_gmt;
  tm_gmt = *gmtime(tmgmt);
  /*
   * The total length of data with format ("%a, %d %b %Y %H:%M:%S %Z")
   * should be (29 + 1)
   *
   * ex. Sat, 04 Jul 2020 10:29:53 GMT\0
   *                                   ^
   *     |------------29-------------| 1
   */
  strftime(date_gmt, 30, "%a, %d %b %Y %H:%M:%S %Z", &tm_gmt);
}

long mk_etag(char *etag, const char *file)
{
  struct stat sb;
  stat(file, &sb);
  sprintf(etag, "\"%lu-%lu-%ld\"", sb.st_ino, sb.st_size, sb.st_mtime);
  return sb.st_mtime;
}

char *find_ext(const char *file)
{
  char *dot = strrchr(file, '.');
  if(!dot || dot == file) return "";
  return dot + 1;
}

int msleep(const long tms)
{
  struct timespec ts;
  int ret;

  if (tms < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = tms / 1000;
  ts.tv_nsec = (tms % 1000) * 1000000;

  do {
    ret = nanosleep(&ts, &ts);
  } while (ret && errno == EINTR);

  return ret;
}

int nsleep(const long tms)
{
  struct timespec ts;
  int ret;

  if (tms < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = 0;
  ts.tv_nsec = tms;

  do {
    ret = nanosleep(&ts, &ts);
  } while (ret && errno == EINTR);

  return ret;
}

long mstime()
{
  struct timeval tv;
  long msec;

  gettimeofday(&tv, 0);
  msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return msec;
}
