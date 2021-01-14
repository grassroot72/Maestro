/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"
#include "http_cache.h"

//#define DEBUG
#include "debug.h"


cache_data_t *http_cache_data_new()
{
  cache_data_t *data = malloc(sizeof(struct _cache_data));
  return data;
}

void http_set_cache_data(cache_data_t *data,
                         char *path,
                         char *etag,
                         char *modified,
                         unsigned char *body,
                         const size_t len_body,
                         unsigned char *body_zipped,
                         const size_t len_zipped)
{
  data->path = path;
  data->etag = etag;
  data->last_modified = modified;
  data->body = body;
  data->len_body = len_body;
  data->body_zipped = body_zipped;
  data->len_zipped = len_zipped;
}

void http_cache_data_destroy(cache_data_t *data)
{
  if (data) {
    if (data->path) free(data->path);
    if (data->etag) free(data->etag);
    if (data->last_modified) free(data->last_modified);
    if (data->body) free(data->body);
    if (data->body_zipped) free(data->body_zipped);
    free(data);
  }
}

cache_data_t *http_cache_data(list_t *cache,
                              const char *path)
{
  node_t *node = list_first(cache);

  if (node) {
    do {
      cache_data_t *data = node->data;
      if (strcmp(path, data->path) == 0) {
        return data;
      }
      node = list_next(cache);
    } while (node);
  }

  return NULL;
}
