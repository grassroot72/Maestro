/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_SVC_H_
#define _HTTP_SVC_H_


#define METHOD_GET 1
#define METHOD_HEAD 0

typedef struct _cached_body cached_body_t;


void http_del_cached_body(cached_body_t *data);
void http_rep_static(int clifd, void *cache, char *path, void *req, int method);


#endif
