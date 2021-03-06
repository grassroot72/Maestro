/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memcpy_sse2.h"
#include "util.h"
#include "http_msg.h"

//#define DEBUG
#include "debug.h"


#define MAX_NUM_HEADERS 32
#define LF '\n'
#define CR '\r'


httpmsg_t *msg_new()
{
  httpmsg_t *msg = malloc(sizeof(struct _httpmsg));
  msg->headers = (struct _httphdr *)
                 malloc(sizeof(struct _httphdr) * MAX_NUM_HEADERS);
  msg->len_startline = 0;
  msg->len_headers = 0;
  msg->num_headers = 0;

  msg->method = METHOD_GET;
  msg->path = NULL;
  msg->status = NULL;

  msg->body = NULL;
  msg->body_zipped = NULL;
  return msg;
}


void msg_set_body_start(httpmsg_t *msg,
                        unsigned char *s)
{
  msg->body_s = s;
}

void msg_add_body(httpmsg_t *msg,
                  unsigned char *body,
                  const size_t len)
{
  msg->body = body;
  msg->len_body = len;
}

void msg_add_zipped_body(httpmsg_t *msg,
                         unsigned char *body_zipped,
                         const size_t len)
{
  msg->body_zipped = body_zipped;
  msg->len_body = len;
}

void msg_destroy(httpmsg_t *msg,
                 const int delbody)
{
  if (!msg) return;

  int i = 0;

  if (msg->path) free(msg->path);
  if (msg->status) free(msg->status);

  do {
    if (msg->headers[i].key) free(msg->headers[i].key);
    if (msg->headers[i].value) free(msg->headers[i].value);
    i++;
  } while (i < msg->num_headers);

  if (delbody) {
    if (msg->body) free(msg->body);
    if (msg->body_zipped) free(msg->body_zipped);
  }
  free(msg->headers);
  free(msg);
}

int msg_split(unsigned char *lines[],
              int *nlines,
              int *len_body,
              const unsigned char *buf)
{
  const unsigned char* p = buf;
  const unsigned char* h = p;
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
      lines[i] = malloc(size);
      memcpy_fast(lines[i], h, len);
      lines[i][len] = '\0';
      h = p + 1;
      i++;
    }

    /* a http message should at least consist of 3 lines */
    if (i > 3 && *(p - 1) == CR && *(p - 2) == LF && *(p - 3) == CR) {
      if (*(p + 1) == CR) return 0;  /* CR without LF followed */
      *nlines = i - 1;  /* end of headers */
    }

    p++;
  } while (*p);

  /* body */
  *len_body = p - h;
  if (*len_body) {
    lines[i] = malloc(*len_body);
    memcpy_fast(lines[i], h, *len_body);
  }

  return i;
}

void msg_lines_destroy(unsigned char *lines[],
                       const int count)
{
  int i = 0;
  do {
    if (lines[i]) {
      free(lines[i]);
      i++;
    }
  } while (i < count);
}

void msg_set_req_line(httpmsg_t *msg,
                      const char *method,
                      const char *path,
                      const int major,
                      const int minor)
{
  int len, total = 0;

  len = strlen(method);
  if (strcmp(method, "GET") == 0) msg->method = METHOD_GET;
  if (strcmp(method, "POST") == 0) msg->method = METHOD_POST;
  if (strcmp(method, "HEAD") == 0) msg->method = METHOD_HEAD;
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

void msg_set_rep_line(httpmsg_t *msg,
                      const int major,
                      const int minor,
                      const int code,
                      const char *status)
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

void msg_add_header(httpmsg_t *msg,
                    const char *key,
                    const char *value)
{
  int len_k, len_v, total;

  len_k = strlen(key);
  len_v = strlen(value);

  msg->headers[msg->num_headers].key = malloc(len_k + 1);
  memcpy_fast(msg->headers[msg->num_headers].key, key, len_k);
  msg->headers[msg->num_headers].key[len_k] = 0;

  msg->headers[msg->num_headers].value = malloc(len_v + 1);
  memcpy_fast(msg->headers[msg->num_headers].value, value, len_v);
  msg->headers[msg->num_headers].value[len_v] = 0;

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

char *msg_header_value(const httpmsg_t *msg,
                       const char *key)
{
  int i = 0;
  do {
    if(strcmp(msg->headers[i].key, key) == 0) {
      return msg->headers[i].value;
    }
    i++;
  } while (i < msg->num_headers);

  return NULL;
}

int msg_add_headers(httpmsg_t *msg,
                    unsigned char *lines[],
                    const int nlines)
{
  char *key;
  char *value;
  int i;

  /* headers ... */
  if (lines[nlines][0] == 0) {  /* empty line */
    i = 1;
    do {
      if (lines[i][0] == 0) {  /* wrong empty line */
        D_PRINT("Not valid message!!!\n");
        return 0;
      }
      key = (char *)lines[i];
      value = split_kv((char *)lines[i], ':');
      msg_add_header(msg, key, value);
      i++;
    } while (i < nlines);
  }
  else {
    D_PRINT("Incomplete message!!!\n");
    return 0;
  }

  return 1;
}

int msg_headers_len(const httpmsg_t *msg)
{
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
  return msg->len_startline + msg->len_headers + 2;
}

void msg_req_headers(char *msg,
                     const httpmsg_t *req)
{
  int i = 0;
  char *ret = msg;

  /* start line */
  if (req->method == METHOD_GET) ret = strbld(msg, "GET");
  if (req->method == METHOD_POST) ret = strbld(msg, "POST");
  if (req->method == METHOD_HEAD) ret = strbld(msg, "HEAD");

  *ret++ = ' ';
  ret = strbld(ret, req->path);
  ret = strbld(ret, " HTTP/");
  *ret++ = req->ver_major + '0';
  *ret++ = '.';
  *ret++ = req->ver_minor + '0';
  *ret++ = CR;
  *ret++ = LF;

  /* headers */
  do {
    ret = strbld(ret, req->headers[i].key);
    *ret++ = ':';
    *ret++ = ' ';
    ret = strbld(ret, req->headers[i].value);
    *ret++ = CR;
    *ret++ = LF;
    i++;
  } while (i < req->num_headers);

  /* ending CRLF */
  *ret++ = CR;
  *ret++ = LF;
}

void msg_rep_headers(char *msg,
                     const httpmsg_t *rep)
{
  int i = 0;
  unsigned char code[16];
  char *ret;

  /* start line */
  ret = strbld(msg, "HTTP/");
  *ret++ = rep->ver_major + '0';
  *ret++ = '.';
  *ret++ = rep->ver_minor + '0';
  *ret++ = ' ';
  itos(code, rep->code, 10, ' ');
  ret = strbld(ret, (char *)code);
  *ret++ = ' ';
  ret = strbld(ret, rep->status);
  *ret++ = CR;
  *ret++ = LF;

  /* headers */
  do {
    ret = strbld(ret, rep->headers[i].key);
    *ret++ = ':';
    *ret++ = ' ';
    ret = strbld(ret, rep->headers[i].value);
    *ret++ = CR;
    *ret++ = LF;
    i++;
  } while (i < rep->num_headers);

  /* ending CRLF */
  *ret++ = CR;
  *ret++ = LF;
}
