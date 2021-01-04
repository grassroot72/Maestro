/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_


/* POST */
void http_post(const int clifd,
               PGconn *pgconn,
               const char *path,
               const httpmsg_t *req);


#endif
