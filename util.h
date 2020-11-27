/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _UTIL_H_
#define _UTIL_H_


#define I2S_SIZE 64


char *uitos(size_t value,
            char dst[I2S_SIZE],
            size_t *len);

char *itos(ssize_t value,
           char dst[I2S_SIZE],
           size_t *len);

char *split_kv(char *kv,
               char delim);

void gmt_date(char *date_gmt,
              long *tmgmt);

long mk_etag(char *etag,
             char *file);

char *find_ext(char *file);

int msleep(long tms);

int nsleep(long tms);

long mstime();

void itohex(unsigned long n,
            int base,
            char sign,
            unsigned char *outbuf);


#endif
