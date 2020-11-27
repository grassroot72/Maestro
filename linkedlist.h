/*
 * Copyright (c) 2019  Alan Skorkin
 * https://github.com/skorks/c-linked-list
 *
 * I reformatted the code according to my coding style, made heavy tweeks
 * and add my code as well
 *
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_


typedef struct _node node_t;

struct _node {
  void *data;
  long stamp;  /* time stamp */
  struct _node *next;
};

typedef struct _list list_t;
struct _list {
  struct _node *head;
  struct _node *current;
};


list_t *list_new();

void list_update(list_t *list,
                 void *data,
                 long stamp);

void list_del(list_t *list,
              long stamp);

void list_reverse(list_t *list);

void list_sort(list_t *list,
               int asc);

void list_display(list_t *list);

void list_destroy(list_t *list);

node_t *list_first(list_t *list);

node_t *list_next(list_t *list);


#endif
