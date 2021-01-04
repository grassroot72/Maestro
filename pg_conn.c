/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "pg_conn.h"

#define DEBUG
#include "debug.h"


void pg_exit_nicely(PGconn *conn)
{
  PQfinish(conn);
  exit(1);
}

PGconn *pg_connect(const char *conninfo,
                   const char *schema)
{
  PGconn *conn;
  PGresult *res;
  char path[64] = "SET search_path=";

  /* Make a connection to the database */
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK) {
    DEBSS("[DB] Connection to database failed", PQerrorMessage(conn));
    pg_exit_nicely(conn);
  }

  /* Set always-secure search path, so malicious users can't take control */
  strcat(path, schema);
  res = PQexec(conn, path);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    DEBSS("[DB] SET search_path failed", PQerrorMessage(conn));
    PQclear(res);
    pg_exit_nicely(conn);
  }

  /* PQclear PGresult whenever it is no longer needed to avoid memory leaks */
  PQclear(res);

  return conn;
}
