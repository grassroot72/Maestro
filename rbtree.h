/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 *
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 * I reformatted the code and added some adjustments
 *
 * license: MIT license
 */

#ifndef _RBTREE_H_
#define _RBTREE_H_


#define RBT_DUP 1
#define RBT_MIN 1

#define RBT_RED 0
#define RBT_BLACK 1

#define RBT_PREORDER 0
#define RBT_INORDER 1
#define RBT_POSTORDER 2


typedef struct _rbnode rbnode_t;

struct _rbnode {
  struct _rbnode *left;
  struct _rbnode *right;
  struct _rbnode *parent;
  char color;
  void *data;
};

typedef struct _rbtree rbtree_t;

struct _rbtree {
  int (*compare)(const void *, const void *);
  void (*print)(void *);
  void (*destroy)(void *);

  rbnode_t root;
  rbnode_t nil;

  #ifdef RBT_MIN
  rbnode_t *min;
  #endif

  pthread_mutex_t mutex;
};


#define RBT_ROOT(rbt) (&(rbt)->root)
#define RBT_NIL(rbt) (&(rbt)->nil)
#define RBT_FIRST(rbt) ((rbt)->root.left)
#define RBT_MINIMAL(rbt) ((rbt)->min)

#define RBT_ISEMPTY(rbt) ((rbt)->root.left == &(rbt)->nil && \
                          (rbt)->root.right == &(rbt)->nil)

#define RBT_APPLY(rbt, f, c, o) rbt_apply((rbt), (rbt)->root.left, (f), (c), (o))


rbtree_t *rbt_create(int (*compare_func)(const void *, const void *),
                     void (*destroy_func)(void *));

void rbt_destroy(rbtree_t *rbt);

rbnode_t *rbt_find(rbtree_t *rbt, void *data);

rbnode_t *rbt_successor(rbtree_t *rbt, rbnode_t *node);

int rbt_apply(rbtree_t *rbt,
              rbnode_t *node,
              int (*func)(void *, void *),
              void *cookie,
              int traversal_order);

void rbt_print(rbtree_t *rbt,
               void (*print_func)(void *));

rbnode_t *rbt_insert(rbtree_t *rbt,
                     void *data);

void *rbt_delete(rbtree_t *rbt,
                 rbnode_t *node,
                 int keep);

int rbt_order(rbtree_t *rbt,
              void *min,
              void *max);

int rbt_black_height(rbtree_t *rbt);


#endif
