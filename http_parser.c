/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_msg.h"

#include "debug.h"


#define MAX_NUM_MSG_LINES 35  /* MAX_NUM_HEADERS + 3 */


httpmsg_t *
http_parse_req(char *buf)
{
  char *line[MAX_NUM_MSG_LINES];  /* http messages lines */
  char *method;
  char *path;
  char *version;
  int major;
  int minor;
  int count, end;

  char *rest;

  httpmsg_t *req;

  if (!buf) return NULL;

  end = 0;
  count = msg_split_lines(line, &end, buf);
  if (!count) {
    DEBS("Empty message!!!");
    return NULL;
  }

  if (count < 3) {
    DEBS("Incomplete message 1!!!");
    msg_lines_destroy(line, count);
    return NULL;
  }

  if ((count - end) > 1) {
    DEBS("Not a valid message");
    msg_lines_destroy(line, count);
    return NULL;
  }

  req = msg_new();

  /* request line ... */
  rest = line[0];
  method = strtok_r(rest, " ", &rest);
  path = strtok_r(NULL, " ", &rest);
  version = strtok_r(NULL, " ", &rest);
  major = version[5] - '0';
  minor = version[7] - '0';

  if (strcmp(path, "/") == 0) {
    msg_set_req_line(req, method, "/index.html", major, minor);
  }
  else {
    msg_set_req_line(req, method, path, major, minor);
  }

  /* headers */
  if (!msg_add_headers(req, line, end)) {
    msg_lines_destroy(line, count);
    return NULL;
  }

  msg_lines_destroy(line, count);
  return req;
}

httpmsg_t *
http_parse_rep(char *buf)
{
  char *line[MAX_NUM_MSG_LINES];  /* http messages lines */
  char *version;
  int major;
  int minor;
  int code;
  char *status;
  int count, end;

  char *rest;

  httpmsg_t *rep;

  if (!buf) return NULL;

  end = 0;
  count = msg_split_lines(line, &end, buf);
  if (count < 3) {
    DEBS("Incomplete message 1!!!");
    msg_lines_destroy(line, count);
    return NULL;
  }

  if ((count - end) > 1) {
    DEBS("Not a valid message");
    msg_lines_destroy(line, count);
    return NULL;
  }

  rep = msg_new();

  /* status line ... */
  rest = line[0];
  version = strtok_r(rest, " ", &rest);
  major = version[5] - '0';
  minor = version[7] - '0';
  code = atoi(strtok_r(NULL, " ", &rest));
  status = strtok_r(NULL, " ", &rest);

  msg_set_rep_line(rep, major, minor, code, status);

  /* headers */
  if (!msg_add_headers(rep, line, end)) {
    msg_lines_destroy(line, count);
    return NULL;
  }

  msg_lines_destroy(line, count);
  return rep;
}

httpmsg_t *
http_parse_headers(char *buf)
{
  char *line[MAX_NUM_MSG_LINES];
  int count, end;

  httpmsg_t *hdrs;

  end = 0;
  count = msg_split_lines(line, &end, buf);
  if (count < 3) {
    DEBS("Incomplete message 1!!!");
    return NULL;
  }

  if ((count - end) > 1) {
    DEBS("Not a valid message");
    return NULL;
  }

  hdrs = msg_new();

  /* headers */
  if (!msg_add_headers(hdrs, line, end)) {
    msg_lines_destroy(line, count);
    return NULL;
  }

  msg_lines_destroy(line, count);
  return hdrs;
}
