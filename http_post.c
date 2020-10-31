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
#include "jsmn.h"
#include "registration.h"

#define DEBUG
#include "debug.h"


static int
_process_json(void *req)
{
  int r;
  jsmntok_t t[128];  /* We expect no more than 128 JSON tokens */
  jsmn_parser *p;
  char *body;
  int len;
  char svc[32];

  identity_t *id;


  /* process the request message here */
  body = (char *)msg_body(req);

  p = jsmn_new();
  jsmn_init(p);
  r = jsmn_parse(p, body, msg_body_len(req), t, 128);

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
  jsmn_destroy(p);

  return 0;
}

void
http_post(int clifd, char *path, void *req)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  unsigned char *body;

  /* some logic to process request here ... */
  _process_json(req);


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
