/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_CACHE_H_
#define _HTTP_CACHE_H_


typedef struct _cached_data cached_data_t;

cached_data_t *http_cached_new();
void http_set_cached_body(cached_data_t *data, char* path, char *etag, char *modified, unsigned char *body, size_t len);
void http_cached_destroy(cached_data_t *data);
cached_data_t *http_cached_data(void *cache, char *path);

char *http_cached_etag(cached_data_t *data);
char *http_cached_last_modified(cached_data_t *data);
unsigned char *http_cached_body(cached_data_t *data);
size_t http_cached_len_body(cached_data_t *data);


#endif
