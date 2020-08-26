/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


//#define DEBUG


#ifdef DEBUG
  #define DEBS(x) printf("%s\n", x)
  #define DEBSS(x, y) printf("%s: [%s]\n", x, y)
  #define DEBSI(s, i) printf("%s: [%d]\n", s, i)
  #define DEBSL(s, l) printf("%s: [%ld]\n", s, l)
#else
  #define DEBS(...)
  #define DEBSS(...)
  #define DEBSI(...)
  #define DEBSL(...)
#endif


#endif
