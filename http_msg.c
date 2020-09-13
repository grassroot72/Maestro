/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "http_msg.h"

#include "debug.h"


#define MAX_NUM_HEADERS 32
#define LF '\n'
#define CR '\r'


struct _httphdr {
  char *key;
  char *value;
};

struct _httpmsg {
  char *method;
  char *path;
  int ver_major;
  int ver_minor;
  int code;      /* status code */
  char *status;  /* status text */
  struct _httphdr headers[MAX_NUM_HEADERS];

  int len_startline;
  int len_headers;
  int num_headers;

  unsigned char *body;    /* point to the body, raw or compressed */
  unsigned char *body_zipped;
  unsigned char *body_s;  /* point to the range start of the body */
  int len_body;
};


httpmsg_t *
msg_new()
{
  httpmsg_t *msg = malloc(sizeof(httpmsg_t));
  msg->len_startline = 0;
  msg->len_headers = 0;
  msg->num_headers = 0;

  msg->method = NULL;
  msg->path = NULL;
  msg->status = NULL;

  msg->body = NULL;
  msg->body_zipped = NULL;
  return msg;
}

unsigned char *
msg_body(httpmsg_t *msg)
{
  return msg->body;
}

unsigned char *
msg_zipped_body(httpmsg_t *msg)
{
  return msg->body_zipped;
}

int
msg_body_len(httpmsg_t *msg)
{
  return msg->len_body;
}

unsigned char *
msg_body_start(httpmsg_t *msg)
{
  return msg->body_s;
}

void
msg_set_body_start(httpmsg_t *msg, unsigned char *s)
{
  msg->body_s = s;
}

void
msg_add_body(httpmsg_t *msg, unsigned char *body, int len)
{
  msg->body = body;
  msg->len_body = len;
}

void
msg_add_zipped_body(httpmsg_t *msg, unsigned char *body_zipped, int len)
{
  msg->body_zipped = body_zipped;
  msg->len_body = len;
}

void
msg_destroy(httpmsg_t *msg, int delbody)
{
  if (!msg) return;

  int i = 0;

  if (msg->method) free(msg->method);
  if (msg->path) free(msg->path);
  if (msg->status) free(msg->status);

  do {
    if (msg->headers[i].key) free(msg->headers[i].key);
    if (msg->headers[i].value) free(msg->headers[i].value);
    i++;
  } while (i < msg->num_headers);

  if (delbody) {
    if (msg->body) free(msg->body);
  }

  if (msg->body_zipped) free(msg->body_zipped);
}

int
msg_split_lines(char *line[], int *end, unsigned char *buf)
{
  unsigned char* p = buf;
  unsigned char* h = p;
  int i;
  int len;
  int size;

  if (*p == CR || *p == LF) {
    return 0;  /* empty message */
  }

  i = 0;
  len = 0;
  do {
    if (*p == LF) {
      /*
       *  xxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n
       *  ^                             ^
       *  h                             p
       *
       *  xxxxxxxxxxxxxxxxxxxxx\r\n
       *  ^
       *  h=p+1
       *
       */
      size = p - h;
      if (!size) return 0;
      len = size - 1;
      line[i] = malloc(size);
      memcpy(line[i], h, len);
      line[i][len] = 0;
      h = p + 1;
      i++;
    }

    /* a http message should at least consist of 3 lines */
    if (i > 3 && *(p - 1) == CR && *(p - 2) == LF && *(p - 3) == CR) {
      if (*(p + 1) == CR) return 0;  /* CR without LF followed */
      *end = i - 1;  /* end of headers */
    }

    p++;
  } while (*p);

  /* body */
  size = p - h;
  if (size) {
    line[i] = malloc(size);
    memcpy(line[i], h, size - 1);
  }

  return i;
}

void
msg_lines_destroy(char *line[], int count)
{
  int i = 0;
  do {
    if (line[i]) {
      free(line[i]);
      i++;
    }
  } while (i < count);
}

char *
msg_method(httpmsg_t *msg)
{
  return msg->method;
}

char *
msg_path(httpmsg_t *msg)
{
  return msg->path;
}

void
msg_set_req_line(httpmsg_t *msg, char *method, char *path, int major, int minor)
{
  int len, total = 0;

  len = strlen(method);
  msg->method = strdup(method);
  total += len;

  len = strlen(path);
  msg->path = strdup(path);
  total += len;

  msg->ver_major = major;
  msg->ver_minor = minor;

  /*
   * GET xxxxx HTTP/1.1\r\n
   *    ^     ^         ^ ^
   *    1     1|--8---| 1 1  (1+1+8+1+1 = 12)
   */
  total += 12;

  msg->len_startline = total;
}

void
msg_set_rep_line(httpmsg_t *msg, int major, int minor, int code, char *status)
{
  int len;

  msg->ver_major = major;
  msg->ver_minor = minor;
  msg->code = code;

  len = strlen(status);
  msg->status = strdup(status);

  /*
   * HTTP/1.1 200 OK\r\n
   *         ^   ^   ^ ^
   * |--8---|1|3|1   1 1  (8+1+3+1+1+1 = 15)
   */
  len += 15;

  msg->len_startline = len;
}

void
msg_add_header(httpmsg_t *msg, char *key, char *value)
{
  int len_k, len_v, total;

  len_k = strlen(key);
  len_v = strlen(value);

  msg->headers[msg->num_headers].key = strdup(key);
  msg->headers[msg->num_headers].value = strdup(value);
  total = len_k + len_v;

  /*
   * Host: www.xxxxx.com\r\n
   *     ^^              ^ ^
   *     11              1 1  (1+1+1+1 = 4)
   */
  total += 4;

  msg->len_headers += total;
  msg->num_headers++;
}

char *
msg_header_value(httpmsg_t *msg, char *key)
{
  int i;

  i = 0;
  do {
    if(strcmp(msg->headers[i].key, key) == 0) {
      return msg->headers[i].value;
    }
    i++;
  } while (i < msg->num_headers);

  return NULL;
}

int
msg_add_headers(httpmsg_t *msg, char *line[], int end)
{
  char* key;
  char* value;
  int i;

  /* headers ... */
  if (line[end][0] == 0) {  /* empty line */
    i = 1;
    do {
      if (line[i][0] == 0) { /* wrong empty line */
        DEBS("Not valid message 2!!!");
        return 0;
      }
      key = line[i];
      value = split_kv(line[i], ':');
      msg_add_header(msg, key, value);
      i++;
    } while (i < end);
  }
  else {
    DEBS("Incomplete message 2!!!");
    return 0;
  }

  return 1;
}

char *
msg_create_req(httpmsg_t *req, int *len)
{
  int i = 0;
  /*
   * GET xxxxx HTTP/1.1\r\n
   * Host: www.xxxxx.com\r\n
   * Accept-Language: en-US,en;q=0.5\r\n
   * \r\n\0
   *  ^ ^ ^
   *  1 1 1  (1+1 = 2)
   */
  *len = req->len_startline + req->len_headers + 2;
  char *msg = malloc(*len + 1);

  /* start line */
  sprintf(msg, "%s %s HTTP/%d.%d\r\n",
               req->method, req->path, req->ver_major, req->ver_minor);

  /* headers */
  do {
    strcat(msg, req->headers[i].key);
    strcat(msg, ": ");
    strcat(msg, req->headers[i].value);
    strcat(msg, "\r\n");
    i++;
  } while (i < req->num_headers);

  /* ending CRLF */
  strcat(msg, "\r\n");
  return msg;
}

char *
msg_create_rep(httpmsg_t *rep, int *len)
{
  int i = 0;
  /*
   * HTTP/1.1 200 OK\r\n
   * Host: www.xxxxx.com\r\n
   * Server: Mass/1.0\r\n
   * \r\n\0
   *  ^ ^ ^
   *  1 1 1  (1+1 = 2)
   * body ....
   *
   */
  *len = rep->len_startline + rep->len_headers + 2;
  char *msg = malloc(*len + 1);

  /* start line */
  sprintf(msg, "HTTP/%d.%d %d %s\r\n",
               rep->ver_major, rep->ver_minor, rep->code, rep->status);

  /* headers */
  do {
    strcat(msg, rep->headers[i].key);
    strcat(msg, ": ");
    strcat(msg, rep->headers[i].value);
    strcat(msg, "\r\n");
    i++;
  } while (i < rep->num_headers);

  /* ending CRLF */
  strcat(msg, "\r\n");
  return msg;
}
