/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_


httpmsg_t *http_parse_req(unsigned char *buf);
httpmsg_t *http_parse_rep(unsigned char *buf);
httpmsg_t *http_parse_headers(unsigned char *buf);


#endif
