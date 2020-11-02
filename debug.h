/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef DEBUG
  #define DEBS(x) printf("%s\n", x)
  #define DEBSS(x, y) printf("%s: %s\n", x, y)
  #define DEBSI(s, i) printf("%s: %d\n", s, i)
  #define DEBSU(s, u) printf("%s: %u\n", s, u)
  #define DEBSL(s, l) printf("%s: %lu\n", s, l)
#else
  /* std=c99 */
  #define DEBS(...)
  #define DEBSS(...)
  #define DEBSI(...)
  #define DEBSU(...)
  #define DEBSL(...)
#endif


#endif
