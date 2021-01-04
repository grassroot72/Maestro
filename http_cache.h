/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_CACHE_H_
#define _HTTP_CACHE_H_


typedef struct _cache_data cache_data_t;

struct _cache_data {
  char *path;
  char *etag;
  char *last_modified;
  unsigned char *body;
  unsigned char *body_zipped;
  size_t len_body;
  size_t len_zipped;
};


cache_data_t *http_cache_data_new();

void http_set_cache_data(cache_data_t *data,
                         char *path,
                         char *etag,
                         char *modified,
                         unsigned char *body,
                         const size_t len_body,
                         unsigned char *body_zipped,
                         const size_t len_zipped);

void http_cache_data_destroy(cache_data_t *data);

cache_data_t *http_cache_data(list_t *cache,
                              const char *path);


#endif
