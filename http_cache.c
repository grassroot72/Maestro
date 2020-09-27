/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"
#include "http_cache.h"

#define DEBUG
#include "debug.h"


struct _cached_data {
  char *path;
  char *etag;
  char *last_modified;
  unsigned char *body;
  size_t len;
};


cached_data_t *
http_cached_new()
{
  cached_data_t *data;
  data = (cached_data_t *)malloc(sizeof(struct _cached_data));
  return data;
}

void
http_set_cached_body(cached_data_t *data, char* path, char *etag, char *modified,
                     unsigned char *body, size_t len)
{
  data->path = path;
  data->etag = etag;
  data->last_modified = modified;
  data->body = body;
  data->len = len;
}

void
http_cached_destroy(cached_data_t *data)
{
  if (data) {
    if (data->path) free(data->path);
    if (data->etag) free(data->etag);
    if (data->last_modified) free(data->last_modified);
    free(data->body);
  }
  free(data);
}

cached_data_t *
http_cached_data(void *cache, char *path)
{
  node_t *node;
  cached_data_t *data;

  node = list_first(cache);
  if (node) {
    do {
      data = (cached_data_t *)list_node_data(node);
      if (strcmp(path, data->path) == 0) {
        return data;
      }
      node = list_next(cache);
    } while (node);
  }

  return NULL;
}

char *
http_cached_etag(cached_data_t *data)
{
  return data->etag;
}

char *
http_cached_last_modified(cached_data_t *data)
{
  return data->last_modified;
}

unsigned char *
http_cached_body(cached_data_t *data)
{
  return data->body;
}

size_t
http_cached_len_body(cached_data_t *data)
{
  return data->len;
}
