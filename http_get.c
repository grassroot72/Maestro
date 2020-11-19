/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "util.h"
#include "linkedlist.h"
#include "io.h"
#include "deflate.h"
#include "http_msg.h"
#include "http_cache.h"
#include "http_svc.h"

//#define DEBUG
#include "debug.h"


#define MAX_PATH 256
#define MAX_CWD 64

#define MIME_BIN 0   /* don't zip this type of data */
#define MIME_TXT 1


static int
_set_content_type(char *ctype, char *ext)
{
  /* MIME types - images */
  if (strcmp(ext, "png") == 0) {
    strcpy(ctype, "image/png");
    return MIME_BIN;
  }
  if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 ||
      strcmp(ext, "jpe") == 0 || strcmp(ext, "jfif") == 0 ||
      strcmp(ext, "pjp") == 0) {
    strcpy(ctype, "image/jpeg");
    return MIME_BIN;
  }
  if (strcmp(ext, "gif") == 0) {
    strcpy(ctype, "image/gif");
    return MIME_BIN;
  }
  if (strcmp(ext, "bmp") == 0) {
    strcpy(ctype, "image/bmp");
    return MIME_BIN;
  }
  if (strcmp(ext, "ico") == 0 || strcmp(ext, "cur") == 0) {
    strcpy(ctype, "image/x-icon");
    return MIME_BIN;
  }
  if (strcmp(ext, "webp") == 0) {
    strcpy(ctype, "image/webp");
    return MIME_BIN;
  }
  if (strcmp(ext, "svg") == 0) {
    strcpy(ctype, "image/svg+xml");
    return MIME_TXT;
  }

  /* MIME type - pdf */
  if (strcmp(ext, "pdf") == 0) {
    strcpy(ctype, "application/pdf");
    return MIME_BIN;
  }

  /* MIME type - gz */
  if (strcmp(ext, "gz") == 0) {
    strcpy(ctype, "application/gzip");
    return MIME_BIN;
  }

  /* MIME type - css */
  if (strcmp(ext, "css") == 0) {
    strcpy(ctype, "text/css");
    return MIME_TXT;
  }

  /* MIME types - javascripts */
  if (strcmp(ext, "js") == 0 || strcmp(ext, "mjs") == 0) {
    /*
     * some doc says text/javascript is obsolete, and recommends to use
     * application/javascript instead, but Mozilla MDN still emphasizes
     * that text/javascript is the standard to be supported in the future
     */
    strcpy(ctype, "text/javascript");
    return MIME_TXT;
  }

  /* MIME type - html */
  if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) {
    strcpy(ctype, "text/html; charset=utf-8");
    return MIME_TXT;
  }

  /* MIME type - plain text */
  if (strcmp(ext, "txt") == 0) {
    strcpy(ctype, "text/plain");
    return MIME_TXT;
  }

  strcpy(ctype, "application/octet-stream");
  return MIME_BIN;
}

static size_t
_process_range(httpmsg_t *rep, char *range_str, size_t len_body, size_t *len_range)
{
  char *range_s;
  char *range_e;
  size_t range_si;  /* range start */
  size_t range_ei;  /* range end */
  char range[64];

  range_s = split_kv(range_str, '=');
  range_e = split_kv(range_s, '-');

  range_si = atoi(range_s);
  /* req: bytes=xxxx-xxxx */
  if (*range_e) {
    range_ei = atol(range_e);
    *len_range = range_ei - range_si + 1;
    sprintf(range, "bytes %lu-%lu/%lu", range_si, range_ei, len_body);
  }
  /* req: bytes=xxxx- */
  else {
    *len_range = len_body - range_si;
    //sprintf(range, "bytes %lu-%lu/%lu", range_si, len_body - 1, len_body);
    sprintf(range, "bytes %lu-/%lu", range_si, len_body);
  }

  msg_add_header(rep, "Content-Range", range);
  DEBSS("[GET_REP] Content-Range", range);
  return range_si;
}

static httpmsg_t *
_get_rep(char *ctype, int mtype, cache_data_t *cdata, httpmsg_t *req)
{
  time_t rep_time;
  char rep_date[30];

  size_t len;
  char len_str[I2S_SIZE];

  char *zip_encoding;

  char *range_str;
  size_t range_s;
  size_t len_range;


  zip_encoding = msg_header_value(req, "Accept-Encoding");

  httpmsg_t* rep = msg_new();

  if (!cdata->etag) {  /* 404 code */
    msg_set_rep_line(rep, 1, 1, 404, "Not Found");
    /* compressed */
    if (mtype == MIME_TXT && zip_encoding && strstr(zip_encoding, "deflate")) {
      msg_add_header(rep, "Content-Encoding", "deflate");
      msg_add_zipped_body(rep, cdata->body_zipped, cdata->len_zipped);
      msg_set_body_start(rep, cdata->body_zipped);
      msg_add_header(rep, "Content-Length",
                     uitos(cdata->len_zipped, len_str, &len));
    }
    /* uncompressed */
    else {
      msg_add_body(rep, cdata->body, cdata->len_body);
      msg_set_body_start(rep, cdata->body);
      msg_add_header(rep, "Content-Length",
                     uitos(cdata->len_body, len_str, &len));
    }
  }
  else {
    msg_add_header(rep, "Server", SVC_VERSION);
    msg_add_header(rep, "Connection", "keep-alive");
    msg_add_header(rep, "Accept-Ranges", "bytes");

    /* reply start date */
    rep_time = time(NULL);
    gmt_date(rep_date, &rep_time);
    msg_add_header(rep, "Date", rep_date);
    /* ETag is a strong validator */
    msg_add_header(rep, "ETag", cdata->etag);
    msg_add_header(rep, "Last-Modified", cdata->last_modified);

    range_str = msg_header_value(req, "Range");
    DEBSS("[REQ] Range", range_str);

    msg_add_header(rep, "Content-Type", ctype);

    /* compressed */
    if (mtype == MIME_TXT &&
        zip_encoding && strstr(zip_encoding, "deflate")) {
      msg_add_header(rep, "Content-Encoding", "deflate");
      msg_add_header(rep, "Vary", "Accept-Encoding");
      msg_add_zipped_body(rep, cdata->body_zipped, cdata->len_zipped);

      if (!range_str) {
        msg_set_rep_line(rep, 1, 1, 200, "OK");
        msg_set_body_start(rep, cdata->body_zipped);
        msg_add_header(rep, "Content-Length",
                       uitos(cdata->len_zipped, len_str, &len));
      }
      else {
        msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
        range_s = _process_range(rep, range_str, cdata->len_zipped, &len_range);
        DEBSL("[GET_REP] range start", range_s);
        DEBSL("[GET_REP] range length", len_range);
        msg_set_body_start(rep, cdata->body_zipped + range_s);
        msg_add_header(rep, "Content-Length", uitos(len_range, len_str, &len));
      }
    }
    /* uncompressed */
    else {
      msg_add_body(rep, cdata->body, cdata->len_body);

      if (!range_str) {
        msg_set_rep_line(rep, 1, 1, 200, "OK");
        msg_set_body_start(rep, cdata->body);
        msg_add_header(rep, "Content-Length", uitos(cdata->len_body, len_str, &len));
      }
      else {
        msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
        range_s = _process_range(rep, range_str, cdata->len_body, &len_range);
        DEBSL("[GET_REP] range start", range_s);
        DEBSL("[GET_REP] range length", len_range);
        msg_set_body_start(rep, cdata->body + range_s);
        msg_add_header(rep, "Content-Length", uitos(len_range, len_str, &len));
      }
    }
  }

  return rep;
}

httpmsg_t *
_get_rep_msg(list_t *cache, char *path, httpmsg_t *req)
{
  unsigned char *body;
  unsigned char *body_zipped;
  size_t len_body;
  size_t len_zipped;
  size_t len_zipbuf;

  char *ext;
  char curdir[MAX_CWD];
  char ospath[MAX_PATH];

  char content_type[32];
  int mime_type;

  deflate_t c;   /* compressor */

  struct stat sb;
  char *last_modified;
  char *etag;

  cache_data_t *data;
  httpmsg_t *rep;


  /* get the fullpath and extention of a file */
  if (!getcwd(curdir, MAX_CWD)) {
    perror("Couldn't read curdir");
  }
  strcpy(ospath, curdir);
  strcat(ospath, path);
  DEBSS("[GET_IO] Opening file", ospath);

  ext = find_ext(path);
  mime_type = _set_content_type(content_type, ext);


  /* check if the body is in the cache */
  data = http_cache_data(cache, path);
  if (data) {
    rep = _get_rep(content_type, mime_type, data, req);
    DEBS("[CACHE] In the cache");
    return rep;
  }

  /* not in the cache ... */
  data = http_cache_data_new();

  if (stat(ospath, &sb) == -1) {
    perror("[SYS]");
    len_body = 44;
    body = (unsigned char *)
           strdup("<html><body>404 Page Not Found</body></html>");
    etag = NULL;
    last_modified = NULL;
  }
  else {
    etag = malloc(30);
    last_modified = malloc(30);
    sprintf(etag, "\"%lu-%lu-%ld\"", sb.st_ino, sb.st_size, sb.st_mtime);
    gmt_date(last_modified, &sb.st_mtime);
    len_body = sb.st_size;
    DEBSL("[IO] len_body", len_body);
    body = io_fread(ospath, len_body);
  }

  if (mime_type == MIME_TXT) {
    /* compress the body */
    len_zipbuf = deflate_bound(len_body);
    body_zipped = malloc(len_zipbuf);
    /* compressed body start should sync with body start */
    len_zipped = deflate(&c, body_zipped, body, len_body, 8);

    DEBSL("[MEM] len_zipped", len_zipped);
    http_set_cache_data(data, strdup(path), etag, last_modified,
                        body, len_body, body_zipped, len_zipped);
  }
  else {
    http_set_cache_data(data, strdup(path), etag, last_modified,
                        body, len_body, NULL, 0);
  }

  list_update(cache, data, mstime());
  rep = _get_rep(content_type, mime_type, data, req);
  DEBS("[CACHE] Cached in...");

  return rep;
}

void
http_rep_static(int clifd, void *cache, char *path, void *req, int method)
{
  httpmsg_t *rep;
  int len_msg;
  unsigned char *bytes;

  rep = _get_rep_msg((list_t *)cache, path, req);
  bytes = (unsigned char *)msg_create_rep(rep, &len_msg);

  /* send msg */
  DEBSI("[GET_REP] Sending reply headers...", clifd);
  io_write_socket(clifd, bytes, len_msg);

  if (method == METHOD_GET) {
    /* send body */
    DEBSI("[GET_REP] Sending reply body...", clifd);
    io_write_socket(clifd, rep->body_s, rep->len_body);
  }

  free(bytes);
  msg_destroy(rep, 0);
}
