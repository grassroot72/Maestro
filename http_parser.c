/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_msg.h"

#define DEBUG
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
    DEBS("Empty message!!!");
    return MSG_EMPTY;
  }

  if (*count < 3) {
    DEBS("Incomplete message 1!!!");
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
  char *method;
  char *path;
  char *version;
  int major, minor;
  int count, nlines;
  int len_body;
  int rc;

  char *rest;
  httpmsg_t *req;

  rc = _fill_lines(lines, &nlines, &len_body, &count, buf);
  if (rc != MSG_OK) return NULL;

  req = msg_new();

  /* request line ... */
  rest = (char *)lines[0];
  method = strtok_r(rest, " ", &rest);
  path = strtok_r(NULL, " ", &rest);
  version = strtok_r(NULL, " ", &rest);
  major = version[5] - '0';
  minor = version[7] - '0';

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
  char *version;
  int major, minor;
  int code;
  char *status;
  int count, nlines;
  int len_body;
  int rc;

  char *rest;
  httpmsg_t *rep;

  rc = _fill_lines(lines, &nlines, &len_body, &count, buf);
  if (rc != MSG_OK) return NULL;

  rep = msg_new();

  /* status line ... */
  rest = (char *)lines[0];
  version = strtok_r(rest, " ", &rest);
  major = version[5] - '0';
  minor = version[7] - '0';
  code = atoi(strtok_r(NULL, " ", &rest));
  status = strtok_r(NULL, " ", &rest);

  msg_set_rep_line(rep, major, minor, code, status);

  rc = _fill_msg(rep, lines, nlines, len_body, count);
  if (rc != MSG_OK) return NULL;

  return rep;
}
