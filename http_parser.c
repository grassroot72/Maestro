/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_msg.h"

//#define DEBUG
#include "debug.h"


static int _fill_lines(unsigned char *lines[],
                       int *nlines,
                       int *len_body,
                       int *count,
                       const unsigned char *buf)
{
  if (!buf) return MSG_EMPTY;

  *nlines = 0;
  *count = msg_split(lines, nlines, len_body, buf);
  if (!*count) {
    D_PRINT("Empty message!!!\n");
    return MSG_EMPTY;
  }

  if (*count < 3) {
    D_PRINT("Incomplete message!!!\n");
    msg_lines_destroy(lines, *count);
    return MSG_IMCOMPLETE;
  }

  return MSG_OK;
}

static int _fill_msg(httpmsg_t *msg,
                     unsigned char *lines[],
                     const int nlines,
                     const int len_body,
                     const int count)
{
  /* headers */
  if (!msg_add_headers(msg, lines, nlines)) {
    msg_lines_destroy(lines, count);
    msg_destroy(msg, 1);
    return MSG_EMPTY;
  }

  /* body */
  if (len_body > 0) {
    msg_add_body(msg, lines[count], len_body);
  }

  msg_lines_destroy(lines, count);
  return MSG_OK;
}

httpmsg_t *http_parse_req(const unsigned char *buf)
{
  unsigned char *lines[MAX_NUM_MSG_LINES];  /* http messages lines */
  int nlines, count, len_body;

  int rc = _fill_lines(lines, &nlines, &len_body, &count, buf);
  if (rc != MSG_OK) return NULL;

  /* request line ... */
  char *rest = (char *)lines[0];
  char *method = strtok_r(rest, " ", &rest);
  char *path = strtok_r(NULL, " ", &rest);
  char *version = strtok_r(NULL, " ", &rest);
  int major = version[5] - '0';
  int minor = version[7] - '0';

  httpmsg_t *req = msg_new();

  if (strcmp(path, "/") == 0)
    msg_set_req_line(req, method, "/index.html", major, minor);
  else
    msg_set_req_line(req, method, path, major, minor);

  rc = _fill_msg(req, lines, nlines, len_body, count);
  if (rc != MSG_OK) return NULL;

  return req;
}

httpmsg_t *http_parse_rep(const unsigned char *buf)
{
  unsigned char *lines[MAX_NUM_MSG_LINES];  /* http messages lines */
  int nlines, count, len_body;

  int rc = _fill_lines(lines, &nlines, &len_body, &count, buf);
  if (rc != MSG_OK) return NULL;

  /* status line ... */
  char *rest = (char *)lines[0];
  char *version = strtok_r(rest, " ", &rest);
  int major = version[5] - '0';
  int minor = version[7] - '0';
  int code = atoi(strtok_r(NULL, " ", &rest));
  char *status = strtok_r(NULL, " ", &rest);

  httpmsg_t *rep = msg_new();
  msg_set_rep_line(rep, major, minor, code, status);

  rc = _fill_msg(rep, lines, nlines, len_body, count);
  if (rc != MSG_OK) return NULL;

  return rep;
}
