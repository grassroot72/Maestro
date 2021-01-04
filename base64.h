/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _BASE64_H_
#define _BASE64_H_


char *b64_encode(size_t *len_out,
                 const size_t len_in,
                 const unsigned char *data);

unsigned char *b64_decode(size_t *len_out,
                          const size_t len_in,
                          const char *data);

void b64_cleanup();


#endif
