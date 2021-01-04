/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
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


void _prep_select(char *sql,
                  const sqlobj_t *sqlo)
{
  strcat(sql, "SELECT ");
  if (sqlo->qfield[0]) {
    strcat(sql, sqlo->qfield);
    strcat(sql, " FROM ");
  }
  else
    strcat(sql, "* FROM ");

  strcat(sql, sqlo->table);

  if (sqlo->clause[0]) {
    strcat(sql, sqlo->clause);
  }
}

void _parse_result(char *res,
                   PGresult *pgres,
                   const int viscols)
{
  int nFields;
  int nRows;
  int i, j;
  char tmp[64];

  nFields = PQnfields(pgres);

  strcat(res, "{");
  /* show attribute names? */
  if (viscols) {
    strcat(res, "\"h\":{\"hd\":[");
    for (i = 0; i < nFields; i++) {
      strcat(res, "\"");
      strcat(res, PQfname(pgres, i));
      if (i != nFields - 1)
        strcat(res, "\",");
      else
        strcat(res, "\"]},");
    }
  }

  /* next, show the row values */
  nRows = PQntuples(pgres);
  for (i = 0; i < nRows; i++) {
    if (i == 0)
      sprintf(tmp, "\"d\":{\"r%03d\":[", i);
    else
      sprintf(tmp, "\"r%03d\":[", i);

    strcat(res, tmp);

    for (j = 0; j < nFields; j++) {
      strcat(res, "\"");
      strcat(res, PQgetvalue(pgres, i, j));
      if (j != nFields - 1)
        strcat(res, "\",");
      else {
        if (i != nRows - 1)
          strcat(res, "\"],");
        else
          strcat(res, "\"]}");
      }
    }
  }
  strcat(res, "}");
  PQclear(pgres);
}

void sql_select(char *res,
                PGconn *pgconn,
                const sqlobj_t *sqlo)
{
  char sql[256] = "";

  PGresult *pgres;
  const char *stmt;


  _prep_select(sql, sqlo);

  /* Start a transaction block */
  pgres = PQexec(pgconn, "BEGIN");
  if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }
  PQclear(pgres);

  /* prepare statement name */
  stmt = "prep_select";
  pgres = PQprepare(pgconn,
                    stmt,
                    sql,
                    0,
                    NULL);
  if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PREPARE failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }
  PQclear(pgres);

  /* execute the prepared statement */
  pgres = PQexecPrepared(pgconn,
                         stmt,
                         0,
                         NULL,
                         NULL,
                         NULL,
                         0);  /* text result */
  if (PQresultStatus(pgres) != PGRES_TUPLES_OK) {
    fprintf(stderr, "SELECT failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }

  /* parse the result set */
  _parse_result(res, pgres, sqlo->viscols);
  DEBSS("[SQL] pg result", res);

  /* Deallocate all prepared statements */
  pgres = PQexec(pgconn, "DEALLOCATE ALL");
  PQclear(pgres);

  /* end the transaction */
  pgres = PQexec(pgconn, "END");
  PQclear(pgres);
}

void _prep_cursor(char *sql,
                  const sqlobj_t *sqlo)
{
  strcat(sql, "DECLARE portal CURSOR FOR ");
  _prep_select(sql, sqlo);
}

void sql_fetch(char *res,
               PGconn *pgconn,
               const sqlobj_t *sqlo)
{
  char sql[256] = "";

  PGresult *pgres;
  const char *stmt;


  _prep_cursor(sql, sqlo);

  /* Start a transaction block */
  pgres = PQexec(pgconn, "BEGIN");
  if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }
  PQclear(pgres);

  /* prepare statement name */
  stmt = "prep_cursor";
  pgres = PQprepare(pgconn,
                    stmt,
                    sql,
                    0,
                    NULL);
  if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PREPARE failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }
  PQclear(pgres);

  /* execute the prepared statement */
  pgres = PQexecPrepared(pgconn,
                         stmt,
                         0,
                         NULL,
                         NULL,
                         NULL,
                         0);  /* text result */
  if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
    fprintf(stderr, "OPEN CURSOR failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }

  pgres = PQexec(pgconn, "FETCH ALL in portal");
  if (PQresultStatus(pgres) != PGRES_TUPLES_OK) {
    fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(pgconn));
    PQclear(pgres);
    pg_exit_nicely(pgconn);
  }

  /* parse the result set */
  _parse_result(res, pgres, sqlo->viscols);
  DEBSS("[SQL] pg result", res);

  /* Deallocate all prepared statements */
  pgres = PQexec(pgconn, "DEALLOCATE ALL");
  PQclear(pgres);

  /* close the portal ... we don't bother to check for errors ... */
  pgres = PQexec(pgconn, "CLOSE portal");
  PQclear(pgres);

  /* end the transaction */
  pgres = PQexec(pgconn, "END");
  PQclear(pgres);
}
