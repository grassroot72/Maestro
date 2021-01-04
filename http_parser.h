/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_


httpmsg_t *http_parse_req(const unsigned char *buf);
httpmsg_t *http_parse_rep(const unsigned char *buf);
httpmsg_t *http_parse_headers(const unsigned char *buf);


#endif
