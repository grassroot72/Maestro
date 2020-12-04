/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libpq-fe.h>
#include "io.h"
#include "base64.h"
#include "deflate.h"
#include "util.h"
#include "linkedlist.h"
#include "jsmn.h"
#include "pg_conn.h"
#include "dml_obj.h"
#include "dml_ops.h"
#include "http_msg.h"
#include "http_post.h"


#define DEBUG
#include "debug.h"


static int _process_json(int clifd,
                         httpmsg_t *req,
                         PGconn *pgconn)
{
  char *body;
  dml_obj_t *dmlo;

  /* process the request message here */
  body = (char *)req->body;
  DEBSS("[REQ] json string", body);

  dmlo = dml_json_parse(body, req->len_body);

  /* this is the microservice */
  dml_select(clifd, pgconn, dmlo);

  dml_json_destroy(dmlo);

  return 0;
}

void http_post(int clifd,
               PGconn *pgconn,
               char *path,
               httpmsg_t *req)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  rep = msg_new();
  msg_add_header(rep, "Server", SVC_VERSION);
  msg_add_header(rep, "Connection", "keep-alive");
  msg_add_header(rep, "Accept-Ranges", "bytes");
  msg_set_rep_line(rep, 1, 1, 200, "OK");
  msg_add_header(rep, "Transfer-Encoding", "chunked");

  bytes = (unsigned char *)msg_create_rep(rep, &len_msg);

  /* send msg */
  DEBSI("[POST_REP] Sending reply headers...", clifd);
  io_write_socket(clifd, bytes, len_msg);

  /* connect to database after receiving request */
  _process_json(clifd, req, pgconn);

  /* terminating the chuncked transfer */
  DEBSI("[POST_REP] Sending terminating chunk...", clifd);
  io_write_socket(clifd, (unsigned char *)"0\r\n", 3);
  io_write_socket(clifd, (unsigned char *)"\r\n", 2);

  free(bytes);
  msg_destroy(rep, 0);
}
