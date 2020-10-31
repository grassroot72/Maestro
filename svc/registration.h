/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_


typedef struct _identity identity_t;


identity_t *registration_parse_json_identity(char *body, jsmntok_t *t, int n);
void registration_destroy_json_identity(identity_t *id);


#endif
