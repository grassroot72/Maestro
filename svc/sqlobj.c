/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memcpy_sse2.h"
#include "jsmn.h"
#include "sqlobj.h"

#define DEBUG
#include "debug.h"


sqlobj_t *sqlobj_new()
{
  sqlobj_t *sqlo = calloc(1, sizeof(struct _sqlobj));
  sqlo->viscols = 1;

  return sqlo;
}

void sqlobj_destroy(sqlobj_t *sqlo)
{
  int i = 0;
  do {
    if (sqlo->keys[i]) free(sqlo->keys[i]);
    if (sqlo->values[i]) free(sqlo->values[i]);
    i++;
  } while (i < sqlo->nkeys);
  free(sqlo);
}

sqlobj_t *sql_parse_json(const char *body,
                         const size_t len_body)
{
  sqlobj_t *sqlo = sqlobj_new();
  if (!sqlo) return NULL;

  jsmntok_t t[MAX_JSMN_TOKENS];  /* MAX_JSMN_TOKENS = 128 */
  jsmntok_t *g;
  jsmn_parser_t p;

  jsmn_init(&p);
  int n = jsmn_parse(&p, body, len_body, t, MAX_JSMN_TOKENS);
  D_PRINT("[SQL] n_toks: %d\n", n);

  /* Loop over all keys */
  int i, j;
  for (i = 1; i < n; i++) {
    int len = 0;
    int count = 0;

    if (jsoneq(body, &t[i], "table") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      memcpy_fast(sqlo->table, body + t[i + 1].start, len);
      sqlo->table[len] = '\0';
      i++;
      D_PRINT("[SQL] table: %s\n", sqlo->table);
    }
    else if (jsoneq(body, &t[i], "cmd") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      memcpy_fast(sqlo->cmd, body + t[i + 1].start, len);
      sqlo->cmd[len] = '\0';
      i++;
      D_PRINT("[SQL] cmd: %s\n", sqlo->cmd);
    }
    else if (jsoneq(body, &t[i], "clause") == 0) {
      len =  t[i + 1].end - t[i + 1].start;
      memcpy_fast(sqlo->clause, body + t[i + 1].start, len);
      sqlo->clause[len] = '\0';
      i++;
      D_PRINT("[SQL] clause: %s\n", sqlo->clause);
    }
    else if (jsoneq(body, &t[i], "viscols") == 0) {
      sqlo->viscols = atoi(body + t[i + 1].start);
      i++;
      D_PRINT("[SQL] viscols %d\n", sqlo->viscols);
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
        sqlo->keys[count] = malloc(len + 1);
        memcpy_fast(sqlo->keys[count], body + g->start, len);
        sqlo->keys[count][len] = '\0';
        D_PRINT("[SQL] key: %s\n", sqlo->keys[count]);
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
        sqlo->values[count] = malloc(len + 1);
        memcpy_fast(sqlo->values[count], body + g->start, len);
        sqlo->values[count][len] = '\0';
        D_PRINT("[SQL] value: %s\n", sqlo->values[count]);
        count++;
      }
      i += t[i + 1].size + 1;
      sqlo->nkeys = count;
    }
  }
  D_PRINT("[SQL] nkeys: %d\n", sqlo->nkeys);

  return sqlo;
}
