/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <string.h>
#include "memcpy_sse2.h"
#include "mime.h"

//#define DEBUG
#include "debug.h"


int mime_set_content_type(char *ctype,
                          const char *ext)
{
  static const char *png = "image/png";
  static const char *jpg = "image/jpeg";
  static const char *gif = "image/gif";
  static const char *bmp = "image/bmp";
  static const char *ico = "image/x-icon";
  static const char *webp = "image/webp";
  static const char *svg = "image/svg+xml";
  static const char *pdf = "application/pdf";
  static const char *gz = "application/gzip";
  static const char *css = "text/css";
  static const char *js = "text/javascript";
  static const char *html = "text/html; charset=utf-8";
  static const char *txt = "text/plain";
  static const char *bin = "application/octet-stream";

  /* MIME types - images */
  if (strcmp(ext, "png") == 0) {
    memcpy_fast(ctype, png, 9);
    ctype[9] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 ||
      strcmp(ext, "jpe") == 0 || strcmp(ext, "jfif") == 0 ||
      strcmp(ext, "pjp") == 0) {
    memcpy_fast(ctype, jpg, 10);
    ctype[10] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "gif") == 0) {
    memcpy_fast(ctype, gif, 9);
    ctype[9] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "bmp") == 0) {
    memcpy_fast(ctype, bmp, 9);
    ctype[9] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "ico") == 0 || strcmp(ext, "cur") == 0) {
    memcpy_fast(ctype, ico, 12);
    ctype[12] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "webp") == 0) {
    memcpy_fast(ctype, webp, 10);
    ctype[10] = '\0';
    return MIME_BIN;
  }
  if (strcmp(ext, "svg") == 0) {
    memcpy_fast(ctype, svg, 13);
    ctype[13] = '\0';
    return MIME_TXT;
  }

  /* MIME type - pdf */
  if (strcmp(ext, "pdf") == 0) {
    memcpy_fast(ctype, pdf, 15);
    ctype[15] = '\0';
    return MIME_BIN;
  }

  /* MIME type - gz */
  if (strcmp(ext, "gz") == 0) {
    memcpy_fast(ctype, gz, 16);
    ctype[16] = '\0';
    return MIME_BIN;
  }

  /* MIME type - css */
  if (strcmp(ext, "css") == 0) {
    memcpy_fast(ctype, css, 8);
    ctype[8] = '\0';
    return MIME_TXT;
  }

  /* MIME types - javascripts */
  if (strcmp(ext, "js") == 0 || strcmp(ext, "mjs") == 0) {
    /*
     * some doc says text/javascript is obsolete, and recommends to use
     * application/javascript instead, but Mozilla MDN still emphasizes
     * that text/javascript is the standard to be supported in the future
     */
    memcpy_fast(ctype, js, 15);
    ctype[15] = '\0';
    return MIME_TXT;
  }

  /* MIME type - html */
  if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) {
    memcpy_fast(ctype, html, 24);
    ctype[24] = '\0';
    return MIME_TXT;
  }

  /* MIME type - plain text */
  if (strcmp(ext, "txt") == 0) {
    memcpy_fast(ctype, txt, 10);
    ctype[10] = '\0';
    return MIME_TXT;
  }

  memcpy_fast(ctype, bin, 24);
  ctype[24] = '\0';
  return MIME_BIN;
}
