/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */
#ifndef _HTTP_MIME_H_
#define _HTTP_MIME_H_


#define MIME_BIN 0   /* don't zip this type of data */
#define MIME_TXT 1


int mime_set_content_type(char *ctype,
                          const char *ext);


#endif
