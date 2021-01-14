/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
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
#include "mime.h"
#include "http_msg.h"
#include "http_cache.h"
#include "http_get.h"

//#define DEBUG
#include "debug.h"


#define MAX_PATH 256
#define MAX_CWD 64


static size_t _process_range(httpmsg_t *rep,
                             char *range_str,
                             size_t *len_range,
                             const size_t len_body)
{
  size_t range_si;  /* range start */
  size_t range_ei;  /* range end */
  char range[64];

  char *range_s = split_kv(range_str, '=');
  char *range_e = split_kv(range_s, '-');

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
  D_PRINT("[GET_REP] Content-Range: %s\n", range);
  return range_si;
}

static httpmsg_t *_get_rep(const char *ctype,
                           const int mtype,
                           cache_data_t *cdata,
                           const httpmsg_t *req)
{

  char len_str[16];
  char *zip_enc = msg_header_value(req, "Accept-Encoding");

  httpmsg_t* rep = msg_new();

  if (!cdata->etag) {  /* 404 code */
    msg_set_rep_line(rep, 1, 1, 404, "Not Found");
    /* compressed */
    if (mtype == MIME_TXT && zip_enc && strstr(zip_enc, "deflate")) {
      msg_add_header(rep, "Content-Encoding", "deflate");
      msg_add_zipped_body(rep, cdata->body_zipped, cdata->len_zipped);
      msg_set_body_start(rep, cdata->body_zipped);
      itos((unsigned char *)len_str, cdata->len_zipped, 10, ' ');
      msg_add_header(rep, "Content-Length", len_str);
    }
    /* uncompressed */
    else {
      msg_add_body(rep, cdata->body, cdata->len_body);
      msg_set_body_start(rep, cdata->body);
      itos((unsigned char *)len_str, cdata->len_body, 10, ' ');
      msg_add_header(rep, "Content-Length", len_str);
    }
  }
  else {
    msg_add_header(rep, "Server", SVR_VERSION);
    msg_add_header(rep, "Connection", "keep-alive");
    msg_add_header(rep, "Accept-Ranges", "bytes");

    /* reply start date */
    char rep_date[30];
    time_t rep_time = time(NULL);
    gmt_date(rep_date, &rep_time);
    msg_add_header(rep, "Date", rep_date);
    /* ETag is a strong validator */
    msg_add_header(rep, "ETag", cdata->etag);
    msg_add_header(rep, "Last-Modified", cdata->last_modified);

    size_t range_s;
    size_t len_range;
    char *range_str = msg_header_value(req, "Range");
    D_PRINT("[REQ] Range: %s\n", range_str);

    msg_add_header(rep, "Content-Type", ctype);

    /* compressed */
    if (mtype == MIME_TXT && zip_enc && strstr(zip_enc, "deflate")) {
      msg_add_header(rep, "Content-Encoding", "deflate");
      msg_add_header(rep, "Vary", "Accept-Encoding");
      msg_add_zipped_body(rep, cdata->body_zipped, cdata->len_zipped);

      if (!range_str) {
        msg_set_rep_line(rep, 1, 1, 200, "OK");
        msg_set_body_start(rep, cdata->body_zipped);
        itos((unsigned char *)len_str, cdata->len_zipped, 10, ' ');
        msg_add_header(rep, "Content-Length", len_str);
      }
      else {
        msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
        range_s = _process_range(rep, range_str, &len_range, cdata->len_zipped);
        D_PRINT("[GREP] range start: %ld, length: %ld\n", range_s, len_range);
        msg_set_body_start(rep, cdata->body_zipped + range_s);
        itos((unsigned char *)len_str, len_range, 10, ' ');
        msg_add_header(rep, "Content-Length", len_str);
      }
    }
    /* uncompressed */
    else {
      msg_add_body(rep, cdata->body, cdata->len_body);

      if (!range_str) {
        msg_set_rep_line(rep, 1, 1, 200, "OK");
        msg_set_body_start(rep, cdata->body);
        itos((unsigned char *)len_str, cdata->len_body, 10, ' ');
        msg_add_header(rep, "Content-Length", len_str);
      }
      else {
        msg_set_rep_line(rep, 1, 1, 206, "Partial Content");
        range_s = _process_range(rep, range_str, &len_range, cdata->len_body);
        D_PRINT("[GREP] range start: %ld, length: %ld\n", range_s, len_range);
        msg_set_body_start(rep, cdata->body + range_s);
        itos((unsigned char *)len_str, len_range, 10, ' ');
        msg_add_header(rep, "Content-Length", len_str);
      }
    }
  }
  return rep;
}

httpmsg_t *_get_rep_msg(list_t *cache,
                        const char *path,
                        const httpmsg_t *req)
{
  char *ret;
  httpmsg_t *rep;

  /* get the fullpath and extention of a file */
  char curdir[MAX_CWD];
  char ospath[MAX_PATH];

  if (!getcwd(curdir, MAX_CWD)) {
    D_PRINT("[SYS] Couldn't read %s\n", curdir);
  }
  ret = strbld(ospath, curdir);
  ret = strbld(ret, path);
  *ret++ = '\0';
  D_PRINT("[IO] Opening file %s\n", ospath);

  char content_type[32];
  char *ext = find_ext(path);
  int mime_type = mime_set_content_type(content_type, ext);


  /* check if the body is in the cache */
  cache_data_t *data = http_cache_data(cache, path);
  if (data) {
    rep = _get_rep(content_type, mime_type, data, req);
    D_PRINT("[CACHE] In the cache!\n");
    return rep;
  }

  /* not in the cache ... */
  struct stat sb;
  char *last_modified;
  char *etag;
  unsigned char *body;
  unsigned char *body_zipped;
  size_t len_body;
  size_t len_zipped;
  size_t len_zipbuf;

  data = http_cache_data_new();
  if (stat(ospath, &sb) == -1) {
    D_PRINT("[SYS] --> %s <-- No such file or directory\n", ospath);
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
    D_PRINT("[IO] len_body: %ld\n", len_body);
    body = io_fread(ospath, len_body);
  }

  if (mime_type == MIME_TXT) {
    struct sdefl c;   /* compressor */
    /* compress the body */
    len_zipbuf = deflate_bound(len_body);
    body_zipped = malloc(len_zipbuf);
    /* compressed body start should sync with body start */
    len_zipped = deflate(&c, body_zipped, body, len_body, 5);

    D_PRINT("[MEM] len_zipped: %ld\n", len_zipped);
    http_set_cache_data(data, strdup(path), etag, last_modified,
                        body, len_body, body_zipped, len_zipped);
  }
  else
    http_set_cache_data(data, strdup(path), etag, last_modified,
                        body, len_body, NULL, 0);

  list_update(cache, data, mstime());
  rep = _get_rep(content_type, mime_type, data, req);
  D_PRINT("[CACHE] Cached in...\n");

  return rep;
}

void http_get(const int clifd,
              list_t *cache,
              const char *path,
              const httpmsg_t *req)
{
  httpmsg_t *rep = _get_rep_msg(cache, path, req);

  int len_headers = msg_headers_len(rep);
  char *headers = malloc(len_headers);
  msg_rep_headers(headers, rep);

  /* send msg */
  D_PRINT("[GREP] Sending reply headers... %d\n", clifd);
  io_socket_write(clifd, (unsigned char *)headers, len_headers);

  /* if method is GET (NOT HEAD), then send body */
  if (req->method == METHOD_GET) {
    /* send body */
    D_PRINT("[GREP] Sending reply body... %d\n", clifd);
    io_socket_write(clifd, rep->body_s, rep->len_body);
  }

  free(headers);
  msg_destroy(rep, 0);
}
