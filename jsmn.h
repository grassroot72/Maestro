/*
 * Copyright (c) 2010  Serge Zaitsev
 *
 * I reformatted the code according to my coding style and made some tweeks.
 *
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _JSMN_H_
#define _JSMN_H_


#define MAX_JSMN_TOKENS 128

/**
 * JSON type identifier. Basic types are:
 *  o Object
 *  o Array
 *  o String
 *  o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
  JSMN_UNDEFINED = 0,
  JSMN_OBJECT = 1,
  JSMN_ARRAY = 2,
  JSMN_STRING = 3,
  JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
  /* Not enough tokens were provided */
  JSMN_ERROR_NOMEM = -1,
  /* Invalid character inside JSON string */
  JSMN_ERROR_INVAL = -2,
  /* The string is not a full JSON packet, more bytes expected */
  JSMN_ERROR_PART = -3
};


typedef struct _jsmntok jsmntok_t;
/**
 * JSON token description.
 * type   type (object, array, string etc.)
 * start  start position in JSON data string
 * end    end position in JSON data string
 */
struct _jsmntok {
  jsmntype_t type;
  int start;
  int end;
  int size;
#ifdef JSMN_PARENT_LINKS
  int parent;
#endif
};

typedef struct _jsmn_parser jsmn_parser_t;

/*
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
struct _jsmn_parser {
  unsigned int pos;     /* offset in the JSON string */
  unsigned int toknext; /* next token to allocate */
  int toksuper;         /* superior token node, e.g. parent object or array */
};


/*
 * Run JSON parser.
 * It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse(jsmn_parser_t *parser,
               const char *js,
               const size_t len,
               jsmntok_t *tokens,
               const unsigned int num_tokens);


/*
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser_t *parser);

int jsoneq(const char *json,
           const jsmntok_t *tok,
           const char *s);


#endif
