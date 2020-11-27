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
#include "util.h"
#include "linkedlist.h"
#include "jsmn.h"
#include "http_msg.h"
#include "dml_obj.h"
#include "http_post.h"


#define DEBUG
#include "debug.h"


static int _process_json(httpmsg_t *req)
{
  char *body;
  dml_obj_t *dmlo;


  /* process the request message here */
  body = (char *)req->body;
  DEBSS("[REQ] json string", body);

  dmlo = dml_json_parse(body, req->len_body);



  dml_json_destroy(dmlo);

  return 0;
}

void _send_chunk(int clifd,
                 char *data)
{
  char *chunk;
  int len_chunk;
  unsigned char hex_str[16];

  chunk = strdup(data);
  len_chunk = strlen(chunk);
  itohex(len_chunk, 16, ' ', hex_str);
  /* send chunked length */
  DEBSI("[POST_REP] Sending chunked length...", clifd);
  io_write_socket(clifd, hex_str, strlen((char *)hex_str));
  io_write_socket(clifd, (unsigned char *)"\r\n", 2);
  /* send chunked */
  DEBSI("[POST_REP] Sending chunked...", clifd);
  io_write_socket(clifd, (unsigned char *)chunk, len_chunk);
  io_write_socket(clifd, (unsigned char *)"\r\n", 2);

  free(chunk);
}

void http_post(int clifd,
               char *path,
               httpmsg_t *req)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  /* connect to database after receiving request */
  _process_json(req);

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

  _send_chunk(clifd, "<html><body>Data stored, ");
  _send_chunk(clifd, "this is a chunked transfer</body></html> ");

  /* terminating the chuncked transfer */
  DEBSI("[POST_REP] Sending terminating chunk...", clifd);
  io_write_socket(clifd, (unsigned char *)"0\r\n", 3);
  io_write_socket(clifd, (unsigned char *)"\r\n", 2);

  free(bytes);
  msg_destroy(rep, 0);
}
