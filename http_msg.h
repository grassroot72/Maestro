/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_MSG_H_
#define _HTTP_MSG_H_


typedef struct _httpmsg httpmsg_t;


httpmsg_t *msg_new();
unsigned char *msg_body(httpmsg_t *msg);
unsigned char *msg_zipped_body(httpmsg_t *msg);
int msg_body_len(httpmsg_t *msg);
unsigned char *msg_body_start(httpmsg_t *msg);
void msg_set_body_start(httpmsg_t *msg, unsigned char *s);
void msg_add_body(httpmsg_t *msg, unsigned char *body, int len);
void msg_add_zipped_body(httpmsg_t *msg, unsigned char *body_zipped, int len);
void msg_destroy(httpmsg_t *msg, int delbody);

int msg_split_lines(char *line[], int *end, unsigned char *buf);
void msg_lines_destroy(char *line[], int count);

char *msg_method(httpmsg_t *msg);
char *msg_path(httpmsg_t *msg);
void msg_set_req_line(httpmsg_t *msg, char *method, char *path, int major, int minor);
void msg_set_rep_line(httpmsg_t *msg, int major, int minor, int code, char *status);

void msg_add_header(httpmsg_t *msg, char *key, char *value);
char *msg_header_value(httpmsg_t *msg, char *key);
int msg_add_headers(httpmsg_t *msg, char *line[], int end);

char *msg_create_req(httpmsg_t *req, int *len);
char *msg_create_rep(httpmsg_t *rep, int *len);


#endif
