/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <errno.h>
#include "util.h"
#include "io.h"
#include "base64.h"
#include "deflate.h"
#include "thpool.h"
#include "http_msg.h"
#include "http_parser.h"
#include "http_svc.h"
#include "debug.h"


#define SVC_VERSION "Maestro/1.0"

#define MAX_PATH 255
#define MAX_CWD 64

#define MIME_BIN 0   /* binary data */
#define MIME_TXT 1


static int
_process_range(httpmsg_t *rep, char *range_str, int len_body, int *len_range)
{
  char *range_s;
  char *range_e;
  int range_si;
  int range_ei;
  char range[128];

  range_s = split_kv(range_str, '=');
  range_e = split_kv(range_s, '-');

  range_si = atoi(range_s);
  if (*range_e) {
    range_ei = atoi(range_e);
    *len_range = range_ei - range_si + 1;
    sprintf(range, "bytes %d-%d/%d", range_si, range_ei, len_body);
  }
  else {
    *len_range = len_body - range_si;
    sprintf(range, "bytes %d-%d/%d", range_si, len_body, len_body);
  }

  msg_add_header(rep, "Content-Range", range);
  return range_si;
}

static int
_add_mime_type(httpmsg_t *rep, char *ext)
{
  /* MIME types - images */
  if (strcmp(ext, "png") == 0) {
    msg_add_header(rep, "Content-Type", "image/png");
    return MIME_BIN;
  }
  if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 ||
      strcmp(ext, "jpe") == 0 || strcmp(ext, "jfif") == 0 ||
      strcmp(ext, "pjp") == 0) {
    msg_add_header(rep, "Content-Type", "image/jpeg");
    return MIME_BIN;
  }
  if (strcmp(ext, "gif") == 0) {
    msg_add_header(rep, "Content-Type", "image/gif");
    return MIME_BIN;
  }
  if (strcmp(ext, "bmp") == 0) {
    msg_add_header(rep, "Content-Type", "image/bmp");
    return MIME_BIN;
  }
  if (strcmp(ext, "ico") == 0 || strcmp(ext, "cur") == 0) {
    msg_add_header(rep, "Content-Type", "image/x-icon");
    return MIME_BIN;
  }
  if (strcmp(ext, "svg") == 0) {
    msg_add_header(rep, "Content-Type", "image/svg+xml");
    return MIME_BIN;
  }
  if (strcmp(ext, "webp") == 0) {
    msg_add_header(rep, "Content-Type", "image/webp");
    return MIME_BIN;
  }

  /* MIME type - pdf */
  if (strcmp(ext, "pdf") == 0) {
    msg_add_header(rep, "Content-Type", "application/pdf");
    return MIME_BIN;
  }

  /* MIME type - css */
  if (strcmp(ext, "css") == 0) {
    msg_add_header(rep, "Content-Type", "text/css");
    return MIME_TXT;
  }

  /* MIME types - javascripts */
  if (strcmp(ext, "js") == 0 || strcmp(ext, "mjs") == 0) {
    /*
     * some doc says text/javascript is obsolete, and recommends to use
     * application/javascript instead, but Mozilla MDN still emphasizes
     * that text/javascript is the standard to be supported in the future
     */
    msg_add_header(rep, "Content-Type", "text/javascript");
    return MIME_TXT;
  }

  /* MIME type - html */
  if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) {
    msg_add_header(rep, "Content-Type", "text/html; charset=utf-8");
    return MIME_TXT;
  }

  /* MIME type - plain text */
  if (strcmp(ext, "txt") == 0) {
    msg_add_header(rep, "Content-Type", "text/plain");
    return MIME_TXT;
  }

  return MIME_BIN;
}

static httpmsg_t *
_build_rep(char *ext, char *fullpath, char *body, int len_body, httpmsg_t *req)
{
  long rep_time;
  char rep_date[30];
  char etag[30];
  /* the If-Range header can be used either with a Last-Modified validator,
   * or with an ETag, but not with both
   *
   * long last_modified_time;
   * char last_modified[30];
   */

  size_t len;
  char len_str[I2S_SIZE];
  int mime_type;

  deflate_t *c;   /* compressor */
  char *zip_encoding;
  char *body_zipped;
  int len_zipped;
  int len_zipbuf;

  char *range_str;
  int range_s;
  int len_range;


  httpmsg_t* rep = msg_new();
  msg_set_body_start(rep, body);

  /* 404 code */
  if (!fullpath) {
    msg_set_rep_line(rep, 1, 1, 404, "Not Found");
    msg_add_body(rep, body, len_body);
    msg_add_header(rep, "Content-Length", itos(len_body, len_str, &len));
  }
  else {
    msg_add_header(rep, "Server", SVC_VERSION);
    msg_add_header(rep, "Accept-Ranges", "bytes");
    msg_add_header(rep, "Connection", "keep-alive");

    range_str = msg_header_value(req, "Range");
    if (range_str) {
      msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
      range_s = _process_range(rep, range_str, len_body, &len_range);
      DEBSI("range start", range_s);
      DEBSI("range length", len_range);
      /* body syncs with the range start */
      msg_set_body_start(rep, body + range_s);
      len_body = len_range;
    }
    else
      msg_set_rep_line(rep, 1, 1, 200, "OK");

    /* reply start date */
    rep_time = time(NULL);
    gmt_date(rep_date, &rep_time);
    msg_add_header(rep, "Date", rep_date);
    /* ETag is a strong validator */
    /* last_modified_time = mk_etag(etag, fullpath); */
    mk_etag(etag, fullpath);
    msg_add_header(rep, "ETag", etag);

    /* Last-Modified is a weak validator,
     * the If-Range header can be used either with a Last-Modified validator,
     * or with an ETag, but not with both
     *
     *
     * gmt_date(last_modified, &last_modified_time);
     * msg_add_header(rep, "Last-Modified", last_modified);
     */

    mime_type = _add_mime_type(rep, ext);
    if (mime_type == MIME_TXT) {
      zip_encoding = msg_header_value(req, "Accept-Encoding");
      if (zip_encoding && strstr(zip_encoding, "deflate")) {
        msg_add_header(rep, "Content-Encoding", "deflate");
        msg_add_header(rep, "Vary", "Accept-Encoding");
      }

      msg_add_body(rep, body, len_body);

      /* compress the body */
      c = deflate_new();
      len_zipbuf = deflate_bound(len_body);
      body_zipped = malloc(len_zipbuf);
      len_zipped = deflate(c, (unsigned char *)body_zipped,
                              /* compressed body start syncs with body start */
                              (unsigned char *)msg_body_start(rep),
                              len_body, 8);
      free(c);

      /* set the compressed body start */
      msg_set_body_start(rep, body_zipped);
      msg_add_zipped_body(rep, body_zipped, len_zipped);
      /* use compressed body length */
      msg_add_header(rep, "Content-Length", itos(len_zipped, len_str, &len));
    }
    else {
      msg_add_body(rep, body, len_body);
      /* use uncompressed body length */
      msg_add_header(rep, "Content-Length", itos(len_body, len_str, &len));
    }
  }

  return rep;
}

void
http_rep_get(int clifd, char *path, httpmsg_t *req, int head_method)
{
  FILE *f;
  char *body;
  int len_body;
  struct stat sb;

  char *ext;
  httpmsg_t *rep;
  int len_msg;
  char *bytes;

  char curdir[MAX_CWD];
  char fullpath[MAX_PATH];

  /* get the fullpath and extention of a file */
  if (!getcwd(curdir, MAX_CWD)) {
    perror("Couldn't read curdir");
  }
  strcpy(fullpath, curdir);
  strcat(fullpath, path);
  ext = find_ext(path);
  DEBSS("[SVC] Opening file", fullpath);

  f = fopen(fullpath, "r");
  if (!f) {
    perror("[SVC]");
    body = strdup("<html><body>404 Page Not Found</body></html>");
    len_body = strlen(body);
    rep = _build_rep("html", NULL, body, len_body, req);
    bytes = msg_create_rep(rep, &len_msg);
  }
  else {
    stat(fullpath, &sb);
    len_body = sb.st_size;
    body = io_fread(f, len_body);
    rep = _build_rep(ext, fullpath, body, len_body, req);
    bytes = msg_create_rep(rep, &len_msg);
  }

  /* send msg */
  DEBSI("[SVC] Sending reply msg...", clifd);
  write(clifd, bytes, len_msg);
  /* send body */
  if (!head_method) {
    DEBSI("[SVC] Sending body...", clifd);
    write(clifd, msg_body_start(rep), msg_body_len(rep));
  }

  free(bytes);
  msg_destroy(rep);
}
