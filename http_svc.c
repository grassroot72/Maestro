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
#include "io.h"
#include "linkedlist.h"
#include "base64.h"
#include "deflate.h"
#include "http_msg.h"
#include "http_svc.h"

#define DEBUG
#include "debug.h"


struct _cached_body {
  char *path;
  char *etag;
  char *last_modified;
  unsigned char *body;
  size_t len;
};


#define SVC_VERSION "Maestro/1.0"

#define MAX_PATH 256
#define MAX_CWD 64

#define MIME_BIN 0   /* binary data */
#define MIME_TXT 1


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

  /* MIME type - gz */
  if (strcmp(ext, "gz") == 0) {
    msg_add_header(rep, "Content-Type", "application/gzip");
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

  msg_add_header(rep, "Content-Type", "application/octet-stream");
  return MIME_BIN;
}

static size_t
_process_range(httpmsg_t *rep, char *range_str, size_t len_body, size_t *len_range)
{
  char *range_s;
  char *range_e;
  size_t range_si;
  size_t range_ei;
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
    //sprintf(range, "bytes %d-%d/%d", range_si, len_body - 1, len_body);
    sprintf(range, "bytes %lu-/%lu", range_si, len_body);
  }

  msg_add_header(rep, "Content-Range", range);
  DEBSS("[REP] Content-Range", range);
  return range_si;
}

static httpmsg_t *
_get_rep(char *ext, cached_body_t *data, httpmsg_t *req)
{
  time_t rep_time;
  char rep_date[30];

  char *last_modified;
  char *etag;

  unsigned char *body;
  size_t len_body;

  size_t len;
  char len_str[I2S_SIZE];
  int mime_type;

  deflate_t *c;   /* compressor */
  char *zip_encoding;
  unsigned char *body_zipped;
  size_t len_zipped;
  size_t len_zipbuf;

  char *range_str;
  size_t range_s;
  size_t len_range;


  etag = data->etag;
  last_modified = data->last_modified;
  body = data->body;
  len_body = data->len;

  httpmsg_t* rep = msg_new();
  msg_set_body_start(rep, body);

  /* 404 code */
  if (!etag) {
    msg_set_rep_line(rep, 1, 1, 404, "Not Found");
    msg_add_body(rep, body, len_body);
    msg_add_header(rep, "Content-Length", itos(len_body, len_str, &len));
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
    msg_add_header(rep, "ETag", etag);
    msg_add_header(rep, "Last-Modified", last_modified);

    range_str = msg_header_value(req, "Range");
    DEBSS("[REQ] Range", range_str);

    if (!range_str)
      msg_set_rep_line(rep, 1, 1, 200, "OK");
    else {
      msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
      range_s = _process_range(rep, range_str, len_body, &len_range);
      DEBSL("[REP] range start", range_s);
      DEBSL("[REP] range length", len_range);
      /* body syncs with the range start */
      msg_set_body_start(rep, body + range_s);
      len_body = len_range;
    }


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
      /* compressed body start should sync with body start */
      len_zipped = deflate(c, body_zipped, msg_body_start(rep), len_body, 8);
      free(c);

      DEBSL("[REP] len_body", len_body);
      DEBSL("[REP] len_zipped", len_zipped);

      /* set the compressed body start */
      msg_set_body_start(rep, body_zipped);
      msg_add_zipped_body(rep, body_zipped, len_zipped);
      /* use compressed body length */
      msg_add_header(rep, "Content-Length", uitos(len_zipped, len_str, &len));
    }
    else {
      msg_add_body(rep, body, len_body);
      DEBSL("[REP] len_body", len_body);
      /* use uncompressed body length */
      msg_add_header(rep, "Content-Length", uitos(len_body, len_str, &len));
    }
  }

  return rep;
}

static cached_body_t *
_get_cached_body(list_t *cache, char *path)
{
  node_t *node;
  cached_body_t *data;

  node = list_first(cache);
  if (node) {
    do {
      data = (cached_body_t *)list_node_data(node);
      if (strcmp(path, data->path) == 0) {
        return data;
      }
      node = list_next(cache);
    } while (node);
  }

  return NULL;
}

static void
_set_cached_body(cached_body_t *data, char* path, char *etag, char *modified,
                 unsigned char *body, size_t len)
{
  data->path = path;
  data->etag = etag;
  data->last_modified = modified;
  data->body = body;
  data->len = len;
}

void
http_del_cached_body(cached_body_t *data)
{
  if (data) {
    if (data->path) free(data->path);
    if (data->etag) free(data->etag);
    if (data->last_modified) free(data->last_modified);
    free(data->body);
  }
}

httpmsg_t *
_get_rep_msg(list_t *cache, char *path, httpmsg_t *req)
{
  FILE *f;
  unsigned char *body;

  char *ext;
  char curdir[MAX_CWD];
  char fullpath[MAX_PATH];

  struct stat sb;
  char last_modified[30];
  char etag[30];

  cached_body_t *data;
  httpmsg_t *rep;


  /* get the fullpath and extention of a file */
  if (!getcwd(curdir, MAX_CWD)) {
    perror("Couldn't read curdir");
  }
  strcpy(fullpath, curdir);
  strcat(fullpath, path);
  ext = find_ext(path);
  DEBSS("[SVC] Opening file", fullpath);


  /* check if the body is in the cache */
  data = _get_cached_body(cache, path);
  if (data) {
    rep = _get_rep(ext, data, req);
    DEBS("[CACHE] In the cache");
    return rep;
  }

  /* not in the cache ... */
  data = (cached_body_t *)malloc(sizeof(cached_body_t));
  f = fopen(fullpath, "r");
  if (!f) {
    perror("[SVC]");
    body = (unsigned char *)strdup("<html><body>404 Page Not Found</body></html>");
    _set_cached_body(data, NULL, NULL, NULL, body, 44);
    rep = _get_rep("html", data, req);
  }
  else {
    stat(fullpath, &sb);
    sprintf(etag, "\"%lu-%lu-%lu\"", sb.st_ino, sb.st_size, sb.st_mtime);
    gmt_date(last_modified, &sb.st_mtime);
    body = io_fread(f, sb.st_size);

    _set_cached_body(data, strdup(path), strdup(etag), strdup(last_modified),
                     body, sb.st_size);

    list_update(cache, data, mstime());

    rep = _get_rep(ext, data, req);
    DEBS("[CACHE] Cached in...");
  }

  return rep;
}

void
http_rep_head(int clifd, void *cache, char *path, void *req)
{
  httpmsg_t *rep;
  int len_msg;
  char *bytes;

  rep = _get_rep_msg((list_t *)cache, path, req);
  bytes = msg_create_rep(rep, &len_msg);

  /* send msg */
  DEBSI("[REP] Sending reply headers...", clifd);
  write(clifd, bytes, len_msg);

  free(bytes);
  msg_destroy(rep, 0);
}

void
http_rep_get(int clifd, void *cache, char *path, void *req)
{
  httpmsg_t *rep;
  int len_msg;
  char *bytes;

  rep = _get_rep_msg((list_t *)cache, path, req);
  bytes = msg_create_rep(rep, &len_msg);

  /* send msg */
  DEBSI("[REP] Sending reply headers...", clifd);
  write(clifd, bytes, len_msg);
  /* send body */
  DEBSI("[REP] Sending reply body...", clifd);
  io_write_socket(clifd, msg_body_start(rep), msg_body_len(rep));

  free(bytes);
  msg_destroy(rep, 0);
}
