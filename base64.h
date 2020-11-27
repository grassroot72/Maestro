/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _BASE64_H_
#define _BASE64_H_


char *b64_encode(const unsigned char *data,
                 size_t len_in,
                 size_t *len_out);

unsigned char *b64_decode(const char *data,
                          size_t len_in,
                          size_t *len_out);

void b64_cleanup();


#endif
