/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_SVC_H_
#define _HTTP_SVC_H_


void http_rep_get(int clifd, char *path, httpmsg_t *req);
void http_rep_head(int clifd, char *path, httpmsg_t *req);


#endif
