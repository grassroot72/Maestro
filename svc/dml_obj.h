/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _DML_OBJ_
#define _DML_OBJ_


typedef struct _dml_obj dml_obj_t;

struct _dml_obj {
  char table[17];
  char cmd[7];
  char *keys[3];
  char *values[3];
  int nkeys;
};

dml_obj_t *dml_json_parse(char *body, size_t len);
void dml_json_destroy(dml_obj_t *dmlo);



#endif
