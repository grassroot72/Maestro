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
    const uint32_t i = (uint32_t)((value % 100) << 1);
    value /= 100;
    dst[next] = digits[i + 1];
    dst[next - 1] = digits[i];
    next -= 2;
  }

  /* Handle last 1-2 digits */
  if (value < 10) {
    dst[next] = (char)('0' + (uint32_t) value);
    *len = length - next - 1;
    return dst + next;
  }

  uint32_t i = (uint32_t)(value << 1);
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
    p = uitos((size_t) - value, dst, len);
    *--p = '-';
    ++*len;
    return p;
  }
  return uitos((size_t)value, dst, len);
}

void
u64tohex(char *s, uint64_t num, int lower_alpha)
{
  uint64_t x = num;

  /*
   * use bitwise-ANDs and bit-shifts to isolate
   * each nibble into its own byte;
   * also need to position relevant nibble/byte into
   * proper location for little-endian copy.
   */
  x = ((x & 0xFFFF) << 32) | ((x & 0xFFFF0000) >> 16);
  x = ((x & 0x0000FF000000FF00) >> 8) | (x & 0x000000FF000000FF) << 16;
  x = ((x & 0x00F000F000F000F0) >> 4) | (x & 0x000F000F000F000F) << 8;

  /*
   * Now isolated hex digit in each byte
   * Ex: 0x1234FACE => 0x0E0C0A0F04030201
   */

  /*
   * Create bitmask of bytes containing alpha hex digits
   * - add 6 to each digit
   * - if the digit is a high alpha hex digit, then the addition
   *   will overflow to the high nibble of the byte
   * - shift the high nibble down to the low nibble and mask
   *   to create the relevant bitmask
   *
   * Using above example:
   * 0x0E0C0A0F04030201 + 0x0606060606060606 = 0x141210150a090807
   * >> 4 == 0x0141210150a09080 & 0x0101010101010101
   * == 0x0101010100000000
   */
  uint64_t mask = ((x + 0x0606060606060606) >> 4) & 0x0101010101010101;

  /* convert to ASCII numeral characters */
  x |= 0x3030303030303030;

  /*
   * if there are high hexadecimal characters, need to adjust
   * for uppercase alpha hex digits:
   *   need to add 0x07 to move 0x3A-0x3F to 0x41-0x46 (A-F)
   * for lowercase alpha hex digits:
   *   need to add 0x27 to move 0x3A-0x3F to 0x61-0x66 (a-f)
   * it's actually more expensive to test if mask non-null
   * and then run the following stmt
   */
  x += ((lower_alpha) ? 0x27 : 0x07) * mask;

  /* copy string to output buffer */
  *(uint64_t *)s = x;
}

void
u64tohex2(char *s, uint64_t num, int lower_alpha)
{
  static const char digits[513] =
    "000102030405060708090A0B0C0D0E0F"
    "101112131415161718191A1B1C1D1E1F"
    "202122232425262728292A2B2C2D2E2F"
    "303132333435363738393A3B3C3D3E3F"
    "404142434445464748494A4B4C4D4E4F"
    "505152535455565758595A5B5C5D5E5F"
    "606162636465666768696A6B6C6D6E6F"
    "707172737475767778797A7B7C7D7E7F"
    "808182838485868788898A8B8C8D8E8F"
    "909192939495969798999A9B9C9D9E9F"
    "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
    "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
    "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
    "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
    "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
    "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
  static const char digits_lower_alpha[513] =
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

  int pos;
  char ch;

  uint32_t x = (uint32_t) num;
  int i = 3;
  char *lut = (char*)((lower_alpha) ? digits_lower_alpha : digits);
  while (i >= 0) {
    pos = (x & 0xFF) << 1;
    ch = lut[pos];
    s[i << 1] = ch;

    ch = lut[pos + 1];
    s[(i << 1) + 1] = ch;

    x >>= 8;
    i -= 1;
  }
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
   * The total length of data with format ("%a, %d %b %Y %H:%M:%S %Z") should
   * be (29 + 1)
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
  sprintf(etag, "\"%ld-%ld-%ld\"", sb.st_ino, sb.st_size, sb.st_mtime);
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

long
mstime()
{
  struct timeval tv;
  long msec;

  gettimeofday(&tv, 0);
  msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return msec;
}
