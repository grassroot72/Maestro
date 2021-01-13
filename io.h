/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _IO_H_
#define _IO_H_


unsigned char *io_socket_read(const int sockfd,
                              int *rc);

void io_socket_write(const int sockfd,
                     const unsigned char *bytes,
                     const size_t len);

unsigned char *io_fread(const char *fname,
                        const size_t len);

unsigned char *io_fread_pipe(FILE *f,
                             const size_t len);

char *io_fgetc(FILE *fpipe,
               int *len);

void io_send_chunk(const int clifd,
                   const char *chunk);


#endif
