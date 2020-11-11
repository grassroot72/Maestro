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
#include "http_msg.h"
#include "http_svc.h"
#include "jsmn.h"
#include "registration.h"

#define DEBUG
#include "debug.h"


static int
_process_json(void *data)
{
  int r;
  jsmntok_t t[128];  /* We expect no more than 128 JSON tokens */
  jsmn_parser p;
  httpmsg_t *req;
  char *body;
  int len;
  char svc[32];

  identity_t *id;


  /* process the request message here */
  req = (httpmsg_t *)data;
  body = (char *)req->body;
  jsmn_init(&p);
  r = jsmn_parse(&p, body, req->len_body, t, 128);

  DEBSI("[REQ] number of json elements", r);
  if (r < 0) {
    DEBS("Failed to parse JSON");
    return r;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    DEBS("Object expected");
    return r;
  }

  if (jsoneq(body, &t[1], "svc") == 0) {
    len =  t[2].end - t[2].start;
    strncpy(svc, body + t[2].start, len);
    svc[len] = '\0';
  }
  else
    return -3;  /* not a valid json string */

  if (strcmp(svc, "registration") == 0) {
    id = registration_parse_json_identity(body, t, r);
  }

  registration_destroy_json_identity(id);

  return 0;
}

void
_send_chunk(int clifd, char *data)
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

void
http_post(int clifd, char *path, void *req)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  /* some logic to process request here ... */
  _process_json(req);


  rep = msg_new();
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
