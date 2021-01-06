/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <string.h>
#include "mime.h"

//#define DEBUG
#include "debug.h"


int mime_set_content_type(char *ctype,
                          const char *ext)
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
