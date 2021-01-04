/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_


/* GET */
void http_get(const int clifd,
              list_t *cache,
              const char *path,
              const httpmsg_t *req,
              const int method);


#endif
