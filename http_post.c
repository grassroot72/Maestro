/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
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
#include "sqlobj.h"
#include "sqlops.h"
#include "http_msg.h"
#include "http_post.h"


#define DEBUG
#include "debug.h"


static void _process_json(char *sqlres,
                          PGconn *pgconn,
                          const httpmsg_t *req)
{
  char *body;
  sqlobj_t *sqlo = NULL;

  /* process the request message here */
  body = (char *)req->body;
  D_PRINT("[REQ] json string: %s\n", body);

  sqlo = sql_parse_json(body, req->len_body);

  if (sqlo) {
    /* this is the microservice */
    if (strcmp(sqlo->cmd, "SELECT") == 0) {
      sql_fetch(sqlres, pgconn, sqlo);
    }
    sqlobj_destroy(sqlo);
  }
}

void http_post(const int clifd,
               PGconn *pgconn,
               const char *path,
               const httpmsg_t *req)
{
  httpmsg_t *rep;
  int len_headers;
  char *headers;
  char sqlres[2048];

  /* connect to database after receiving request */
  _process_json(sqlres, pgconn, req);

  rep = msg_new();
  msg_add_header(rep, "Server", SVR_VERSION);
  msg_add_header(rep, "Connection", "keep-alive");
  msg_add_header(rep, "Accept-Ranges", "bytes");
  msg_set_rep_line(rep, 1, 1, 200, "OK");
  msg_add_header(rep, "Transfer-Encoding", "chunked");

  len_headers = msg_headers_len(rep);
  headers = malloc(len_headers);
  msg_rep_headers(headers, rep);

  /* send msg */
  D_PRINT("[PREP] Sending reply headers... %d\n", clifd);
  io_socket_write(clifd, (unsigned char *)headers, len_headers);

  /* send chunks */
  D_PRINT("[PREP] Sending chunks... %d\n", clifd);
  io_send_chunk(clifd, sqlres);

  /* terminating the chuncked transfer */
  D_PRINT("[PREP] Sending terminating chunk... %d\n", clifd);
  io_socket_write(clifd, (unsigned char *)"0\r\n\r\n", 5);

  free(headers);
  msg_destroy(rep, 0);
}
