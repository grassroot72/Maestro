/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _PG_CONN_H_
#define _PG_CONN_H_


void pg_exit_nicely(PGconn *conn);

PGconn *pg_connect(const char *conninfo,
                   const char *schema);


#endif
