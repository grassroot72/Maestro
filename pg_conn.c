/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "pg_conn.h"


static void
_exit_nicely(PGconn *conn)
{
  PQfinish(conn);
  exit(1);
}

PGconn *
pg_connect(const char *conninfo)
{
  PGconn *conn;

  /* Make a connection to the database */
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
    _exit_nicely(conn);
  }

  return conn;
}

