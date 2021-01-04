/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _UTIL_H_
#define _UTIL_H_


void itos(unsigned char *outbuf,
          unsigned long n,
          const int base,
          const char sign);

char *split_kv(char *kv,
               char delim);

void gmt_date(char *date_gmt,
              const long *tmgmt);

long mk_etag(char *etag, const char *file);

char *find_ext(const char *file);

int msleep(const long tms);

int nsleep(const long tms);

long mstime();


#endif
