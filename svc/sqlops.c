/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "io.h"
#include "pg_conn.h"
#include "sqlobj.h"
#include "sqlops.h"

#define DEBUG
#include "debug.h"


void sql_select(int clifd,
                PGconn *pgconn,
                sqlobj_t *sqlo)
{
  char sql[256];

  PGresult *res;
  const char *stmt;
  int nFields;
  char fnames[256];
  int nRows;
  char values[512];
  int i, j;

  if (strcmp(sqlo->cmd, "SELECT") == 0) {
    strcpy(sql, "SELECT * FROM ");
    strcat(sql, sqlo->table);
  }

  /* Start a transaction block */
  res = PQexec(pgconn, "BEGIN");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(pgconn));
    PQclear(res);
    pg_exit_nicely(pgconn);
  }
  PQclear(res);

  /* prepare statement name */
  stmt = "prep_select";
  res = PQprepare(pgconn,
                  stmt,
                  sql,
                  0,
                  NULL);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PREPARE failed: %s", PQerrorMessage(pgconn));
    PQclear(res);
    pg_exit_nicely(pgconn);
  }
  PQclear(res);

  /* execute the prepared statement */
  res = PQexecPrepared(pgconn,
                       stmt,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       0);  /* text result */
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "SELECT failed: %s", PQerrorMessage(pgconn));
    PQclear(res);
    pg_exit_nicely(pgconn);
  }

  nFields = PQnfields(res);

  io_send_chunk(clifd, "{");
  /* attribute names */
  if (sqlo->viscols) {
    strcpy(fnames, "\"h\":{\"hd\":[");
    for (i = 0; i < nFields; i++) {
      strcat(fnames, "\"");
      strcat(fnames, PQfname(res, i));
      if (i != nFields - 1)
        strcat(fnames, "\",");
      else
        strcat(fnames, "\"]},");
    }
    io_send_chunk(clifd, fnames);
  }

  /* next, print out the rows */
  nRows = PQntuples(res);
  for (i = 0; i < nRows; i++) {
    if (i == 0)
      sprintf(values, "\"d\":{\"r%03d\":[", i);
    else
      sprintf(values, "\"r%03d\":[", i);

    for (j = 0; j < nFields; j++) {
      strcat(values, "\"");
      strcat(values, PQgetvalue(res, i, j));
      if (j != nFields - 1)
        strcat(values, "\",");
      else {
        if (i != nRows - 1)
          strcat(values, "\"],");
        else
          strcat(values, "\"]}");
      }
    }
    io_send_chunk(clifd, values);
  }
  io_send_chunk(clifd, "}");
  PQclear(res);

  /* Deallocate all prepared statements */
  res = PQexec(pgconn, "DEALLOCATE ALL");
  PQclear(res);

  /* end the transaction */
  res = PQexec(pgconn, "END");
  PQclear(res);
}
