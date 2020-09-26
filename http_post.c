
/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "io.h"
#include "base64.h"
#include "deflate.h"
#include "http_msg.h"
#include "http_svc.h"

#define DEBUG
#include "debug.h"


void
http_post(int clifd, char *path, void *req)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  unsigned char *body;

  DEBSS("[REQ] body", msg_body(req));

  rep = msg_new();
  msg_set_rep_line(rep, 1, 1, 200, "OK");

  body = (unsigned char *)strdup("<html><body>Data stored</body></html>");
  msg_add_body(rep, body, 37);
  msg_add_header(rep, "Content-Length", "37");

  bytes = (unsigned char *)msg_create_rep(rep, &len_msg);

  /* send msg */
  DEBSI("[POST_REP] Sending reply headers...", clifd);
  io_write_socket(clifd, bytes, len_msg);
  /* send body */
  DEBSI("[POST_REP] Sending reply body...", clifd);
  io_write_socket(clifd, msg_body(rep), msg_body_len(rep));

  free(bytes);
  msg_destroy(rep, 0);
}
