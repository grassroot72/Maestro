/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"
#include "dml_obj.h"

#define DEBUG
#include "debug.h"


dml_obj_t *
dml_json_parse(char *body, size_t len)
{
  int i, j, n;
  int count;

  jsmntok_t t[128];  /* We expect no more than 128 JSON tokens */
  jsmntok_t *g;
  jsmn_parser p;

  dml_obj_t *dmlo;


  jsmn_init(&p);
  n = jsmn_parse(&p, body, len, t, 128);

  dmlo = malloc(sizeof(struct _dml_obj));
  count = 0;

  DEBSI("[DML] n_toks", n);
  /* Loop over all keys */
  for (i = 1; i < n; i++) {
    if (jsoneq(body, &t[i], "table") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      strncpy(dmlo->table, body + t[i + 1].start, len);
      dmlo->table[len] = '\0';
      i++;
      DEBSS("table", dmlo->table);
    }
    else if (jsoneq(body, &t[i], "cmd") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      strncpy(dmlo->cmd, body + t[i + 1].start, len);
      dmlo->cmd[len] = '\0';
      i++;
      DEBSS("cmd", dmlo->cmd);
    }

    /* keys - json array */
    else if (jsoneq(body, &t[i], "keys") == 0) {
      if (t[i + 1].type != JSMN_ARRAY) {
        /* We expect keys to be an array of strings */
        continue;
      }
      for (j = 0; j < t[i + 1].size; j++) {
        g = &t[i + j + 2];
        len = g->end - g->start;
        dmlo->keys[count] = malloc(len + 1);
        strncpy(dmlo->keys[count], body + g->start, len);
        dmlo->keys[count][len] = '\0';
        DEBSS("key", dmlo->keys[count]);
        count++;
      }
      i += t[i + 1].size + 1;
      count = 0;  /* reset counter */
    }
    /* values - json array */
    else if (jsoneq(body, &t[i], "values") == 0) {
      if (t[i + 1].type != JSMN_ARRAY) {
        /* We expect keys to be an array of strings */
        continue;
      }
      for (j = 0; j < t[i + 1].size; j++) {
        g = &t[i + j + 2];
        len = g->end - g->start;
        dmlo->values[count] = malloc(len + 1);
        strncpy(dmlo->values[count], body + g->start, len);
        dmlo->values[count][len] = '\0';
        DEBSS("value", dmlo->values[count]);
        count++;
      }
      i += t[i + 1].size + 1;
      dmlo->nkeys = count;
    }
  }
  DEBSI("[DML] nkeys", dmlo->nkeys);

  return dmlo;
}

void
dml_json_destroy(dml_obj_t *dmlo)
{
  int i;
  if (dmlo) {
    for (i = 0; i < dmlo->nkeys; i++) {
      if (dmlo->keys[i]) free(dmlo->keys[i]);
      if (dmlo->values[i]) free(dmlo->values[i]);
    }
  }
}
