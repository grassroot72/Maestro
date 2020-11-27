/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_SVC_H_
#define _HTTP_SVC_H_


#define SVC_VERSION "Maestro/1.0"

#define METHOD_GET 1
#define METHOD_HEAD 0


/* GET */
void http_rep_static(int clifd,
                     list_t *cache,
                     char *path,
                     httpmsg_t *req,
                     int method);
/* POST */
void http_post(int clifd,
               char *path,
               httpmsg_t *req);


#endif
