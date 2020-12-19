/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _SQLOBJ_
#define _SQLOBJ_


#define MAX_SQL_KEYS 32


typedef struct _sqlobj sqlobj_t;

struct _sqlobj {
  char table[17];
  char cmd[7];  /* INSERT, UPDATE, DELETE */
  char condition[128];  /* where clasue */
  char *keys[MAX_SQL_KEYS];
  char *values[MAX_SQL_KEYS];
  int nkeys;
  int viscols;  /* field name visibility */
};


sqlobj_t *sql_json_parse(char *body,
                         size_t len);

void sql_json_destroy(sqlobj_t *sqlo);


#endif
