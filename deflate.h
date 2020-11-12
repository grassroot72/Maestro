/*
 * Copyright (c) 2020 Micha Mettke
 * https://github.com/vurtun/sdefl
 *
 * I reformatted the code according to my coding style, I added some functions
 *
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _DEFLATE_H_
#define _DEFLATE_H_


#define DEFLATE_MAX_OFF (1 << 15)
#define DEFLATE_WIN_SIZE DEFLATE_MAX_OFF
#define DEFLATE_WIN_MASK (DEFLATE_WIN_SIZE - 1)

#define DEFLATE_MIN_MATCH 4
#define DEFLATE_MAX_MATCH 258

#define DEFLATE_HASH_BITS 15
#define DEFLATE_HASH_SIZE (1 << DEFLATE_HASH_BITS)
#define DEFLATE_HASH_MASK (DEFLATE_HASH_SIZE - 1)
#define DEFLATE_NIL (-1)

#define DEFLATE_LVL_MIN 0
#define DEFLATE_LVL_DEF 5
#define DEFLATE_LVL_MAX 8


#define DEFLATE_ZLIB_HDR (0x01)

typedef struct _deflate deflate_t;

struct _deflate {
  int bits, cnt;
  int tbl[DEFLATE_HASH_SIZE];
  int prv[DEFLATE_WIN_SIZE ];
};


int deflate(deflate_t *s,
            unsigned char *out,
            const unsigned char *in,
            int in_len,
            int lvl);

int zdeflate(deflate_t *s,
             unsigned char *out,
             const unsigned char *in,
             int in_len,
             int lvl);

int deflate_bound(int in_len);


#endif
