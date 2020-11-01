/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
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


char *
uitos(size_t value, char dst[I2S_SIZE], size_t *len)
{
  /*
   * Based on routine by A. Alexandrescu, licensed under CC0
   * https://creativecommons.org/publicdomain/zero/1.0/legalcode
   */
  static const size_t length = I2S_SIZE;
  size_t next = length - 1;

  static const char digits[201] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

  dst[next--] = '\0';
  while (value >= 100) {
    const size_t i = (size_t)((value % 100) << 1);
    value /= 100;
    dst[next] = digits[i + 1];
    dst[next - 1] = digits[i];
    next -= 2;
  }

  /* Handle last 1-2 digits */
  if (value < 10) {
    dst[next] = (char)('0' + (size_t)value);
    *len = length - next - 1;
    return dst + next;
  }

  size_t i = (size_t)(value << 1);
  dst[next] = digits[i + 1];
  dst[next - 1] = digits[i];
  *len = length - next;

  return dst + next - 1;
}

char *
itos(ssize_t value, char dst[I2S_SIZE], size_t *len)
{
  char *p;
  if (value < 0) {
    p = uitos((size_t)-value, dst, len);
    *--p = '-';
    ++*len;
    return p;
  }
  return uitos((size_t)value, dst, len);
}

char *
split_kv(char *kv, char delim)
{
  char *p = kv;
  do {
    if (*p == delim) {
      /*
       *  xxxxxxxxxxxx: xxxxxxxxxxxx\0
       *  ^           ^              ^
       *  h           p              p
       */
      *p = 0;
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

void
gmt_date(char *date_gmt, long *tmgmt)
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

long
mk_etag(char *etag, char *file)
{
  struct stat sb;
  stat(file, &sb);
  sprintf(etag, "\"%lu-%lu-%ld\"", sb.st_ino, sb.st_size, sb.st_mtime);
  return sb.st_mtime;
}

char *
find_ext(char *file)
{
  char *dot = strrchr(file, '.');
  if(!dot || dot == file) return "";
  return dot + 1;
}

int
msleep(long tms)
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

int
nsleep(long tms)
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

long
mstime()
{
  struct timeval tv;
  long msec;

  gettimeofday(&tv, 0);
  msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return msec;
}

/*
 * n      -  number to be printed
 * base   -  number base for conversion;  decimal=10,hex=16
 * sign   -  signed or unsigned output
 * outbuf -  buffer to hold the output number
 */
void
itohex(unsigned long n, int base, char sign, unsigned char *outbuf)
{
  int i = 12;
  int j = 0;

  do {
    outbuf[i] = "0123456789ABCDEF"[n % base];
    i--;
    n = n/base;
  } while(n > 0);

  if (sign != ' ') {
    outbuf[0] = sign;
    ++j;
  }

  while (++i < 13) {
    outbuf[j++] = outbuf[i];
  }

  outbuf[j] = '\0';
}
