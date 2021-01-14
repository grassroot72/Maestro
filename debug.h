/*
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 *
 * license: MIT license
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef DEBUG
#define D_PRINT(...)  fprintf(stderr, __VA_ARGS__)
#define D_DUMP_HEX(buf, len)  {if (buf) {                                 \
                                 int _i_;                                 \
                                 for (_i_ = 0; _i_ < len; _i_++) {        \
                                   DEBUG_PRINT("%02X ",                   \
                                               (unsigned int)(buf)[_i_]); \
                                 }                                        \
                               }                                          \
                               else {                                     \
                                 fprintf(stderr, "(null)");               \
                               }}
#define D_INDEX(fields)  print_index(fields)
#define D_DUMP(buf, length)  fwrite(buf, 1, length, stderr);
#define D_DUMP_HEX_LABEL(title, buf, len)  {fprintf(stderr, "%s (%i): ",      \
                                                            title, (int)len); \        \
                                            DEBUG_DUMP_HEX(buf, len);         \
                                            fprintf(stderr, "\n");}
#else
#define D_PRINT(...)  { }
#define D_DUMP_HEX(buf, len)  { }
#define D_INDEX(fields)  { }
#define D_DUMP(buf, length)  { }
#define D_DUMP_HEX_LABEL(title, buf, len) { }
#endif


#endif
