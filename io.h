/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _IO_H_
#define _IO_H_


char *io_read_socket(int sockfd, int *rc);
char *io_fgetc(FILE *fpipe, int *len);
char *io_fread(FILE *fpipe, int len);


#endif
