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


typedef struct _deflate deflate_t;


deflate_t *deflate_new();
void deflate_destroy(deflate_t *s);

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
