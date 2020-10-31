/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"
#include "json.h"
#include "registration.h"


struct _identity {
  char *action;
  char *first_name;
  char *last_name;
  char *email;
};


identity_t *
registration_parse_json_identity(char *body, jsmntok_t *t, int n)
{
  int i;
  int len;

  identity_t *id;

  id = malloc(sizeof(struct _identity));

  /* Loop over all keys */
  for (i = 3; i < n; i++) {
    if (jsoneq(body, &t[i], "action") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      id->action = malloc(len + 1);
      strncpy(id->action, body + t[i + 1].start, len);
      id->action[len] = '\0';
      i++;
    }
    else if (jsoneq(body, &t[i], "first_name") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      id->first_name = malloc(len + 1);
      strncpy(id->first_name, body + t[i + 1].start, len);
      id->first_name[len] = '\0';
      i++;
    }
    else if (jsoneq(body, &t[i], "last_name") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      id->last_name = malloc(len + 1);
      strncpy(id->last_name, body + t[i + 1].start, len);
      id->last_name[len] = '\0';
      i++;
    }
    else if (jsoneq(body, &t[i], "email") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      id->email = malloc(len + 1);
      strncpy(id->email, body + t[i + 1].start, len);
      id->email[len] = '\0';
      i++;
    }
  }
  return id;
}

void
registration_destroy_json_identity(identity_t *id)
{
  if (id) {
    if (id->action) free(id->action);
    if (id->first_name) free(id->first_name);
    if (id->last_name) free(id->last_name);
    if (id->email) free(id->email);
    free(id);
  }
}
