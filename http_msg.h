/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _HTTP_MSG_H_
#define _HTTP_MSG_H_


struct _httphdr {
  char *key;
  char *value;
};

typedef struct _httpmsg httpmsg_t;

struct _httpmsg {
  char *method;
  char *path;
  int ver_major;
  int ver_minor;
  int code;      /* status code */
  char *status;  /* status text */

  struct _httphdr *headers;
  int num_headers;

  int len_startline;
  int len_headers;

  unsigned char *body;    /* point to the body, raw or compressed */
  unsigned char *body_zipped;
  unsigned char *body_s;  /* point to the range start of the body */
  size_t len_body;
};


httpmsg_t *msg_new();
void msg_set_body_start(httpmsg_t *msg, unsigned char *s);
void msg_add_body(httpmsg_t *msg, unsigned char *body, size_t len);
void msg_add_zipped_body(httpmsg_t *msg, unsigned char *body_zipped, size_t len);
void msg_destroy(httpmsg_t *msg, int delbody);

int msg_split(unsigned char *line[], int *end, int *len_body, unsigned char *buf);
void msg_lines_destroy(unsigned char *line[], int count);

void msg_set_req_line(httpmsg_t *msg, char *method, char *path, int major, int minor);
void msg_set_rep_line(httpmsg_t *msg, int major, int minor, int code, char *status);

void msg_add_header(httpmsg_t *msg, char *key, char *value);
char *msg_header_value(httpmsg_t *msg, char *key);
int msg_add_headers(httpmsg_t *msg, unsigned char *line[], int end);

char *msg_create_req(httpmsg_t *req, int *len);
char *msg_create_rep(httpmsg_t *rep, int *len);


#endif
