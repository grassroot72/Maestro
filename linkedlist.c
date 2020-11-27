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

#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"

//#define DEBUG
#include "debug.h"


static node_t *_node_new(void *data,
                         long stamp)
{
  node_t *n = malloc(sizeof(struct _node));
  if (!n) return NULL;

  n->data = data;
  n->stamp = stamp;
  n->next = NULL;
  return n;
}

list_t *list_new()
{
  list_t *list = malloc(sizeof(struct _list));
  if (!list) return NULL;

  list->head = NULL;
  list->current = NULL;
  return list;
}

static void _add(list_t *list,
                 void *data,
                 long stamp)
{
  node_t *current = NULL;
  if (list->head == NULL) {
    list->head = _node_new(data, stamp);
  }
  else {
    current = list->head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = _node_new(data, stamp);
  }
}

void list_update(list_t *list,
                 void *data,
                 long stamp)
{
  node_t *current = list->head;
  while (current != NULL){
    if (current->data == data && current->stamp == stamp) return;
    if (current->data == data && current->stamp < stamp) {
      DEBSL("[STAMP]  record time", current->stamp);
      DEBSL("[STAMP] current time", stamp);
      current->stamp = stamp;
      return;
    }
    current = current->next;
  }
  _add(list, data, stamp);
}

void list_del(list_t *list,
              long stamp)
{
  node_t *current = list->head;
  node_t *previous = current;
  while (current != NULL) {
    if (current->stamp == stamp) {
      previous->next = current->next;
      if (current == list->head) list->head = current->next;
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }
}

void list_reverse(list_t *list)
{
  node_t *reversed = NULL;
  node_t *current = list->head;
  node_t *temp = NULL;
  while (current != NULL) {
    temp = current;
    current = current->next;
    temp->next = reversed;
    reversed = temp;
  }
  list->head = reversed;
}

static void _swap(node_t *p1,
                  node_t *p2)
{
  long stamp = p1->stamp;
  void *data = p1->data;

  p1->stamp = p2->stamp;
  p1->data = p2->data;

  p2->stamp = stamp;
  p2->data = data;
}

void list_sort(list_t *list,
               int asc)
{
  node_t *start = list->head;
  node_t *traverse;
  node_t *min;

  if (start == NULL) return;

  while (start->next) {
    min = start;
    traverse = start->next;

    while (traverse) {
      /* Find the minimum */
      if (!asc) {
        if (min->stamp < traverse->stamp)
          min = traverse;
      }
      else {
        if (min->stamp > traverse->stamp)
          min = traverse;
      }
      traverse = traverse->next;
    }
    _swap(start, min);
    start = start->next;
  }
}

void list_display(list_t *list)
{
  node_t *current = list->head;
  if (list->head == NULL) return;

  for(; current != NULL; current = current->next) {
    DEBSL("[LIST] data", (long)current->data);
    DEBSL("[LIST] stamp", current->stamp);
  }
}

void list_destroy(list_t *list)
{
  node_t *current = list->head;
  node_t *next = current;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  free(list);
}

node_t *list_first(list_t *list)
{
  list->current = list->head;
  return list->head;
}

node_t *list_next(list_t *list)
{
  node_t *current = list->current;
  if (current) {
    list->current = list->current->next;
    current = list->current;
  }
  return current;
}
