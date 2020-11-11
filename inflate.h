/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _INFLATE_H_
#define _INFLATE_H_


typedef struct _inflate inflate_t;

struct _inflate {
  int bits, bitcnt;
  unsigned lits[288];
  unsigned dsts[32];
  unsigned lens[19];
  int tlit, tdist, tlen;
};


int inflate(unsigned char* out, const unsigned char* in, int size);


#endif

