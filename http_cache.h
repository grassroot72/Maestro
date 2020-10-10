/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_CACHE_H_
#define _HTTP_CACHE_H_


typedef struct _cache_data cache_data_t;

cache_data_t *http_cache_new();
void http_set_cache_body(cache_data_t *data,
                         char *path, char *etag, char *modified,
                         unsigned char *body, size_t len,
                         unsigned char *body_zipped, size_t len_zipped);
void http_cache_destroy(cache_data_t *data);
cache_data_t *http_cache_data(void *cache, char *path);

char *http_cache_etag(cache_data_t *data);
char *http_cache_last_modified(cache_data_t *data);
unsigned char *http_cache_body(cache_data_t *data);
size_t http_cache_len_body(cache_data_t *data);
unsigned char *http_cache_body_zipped(cache_data_t *data);
size_t http_cache_len_zipped(cache_data_t *data);


#endif
