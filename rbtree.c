/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 *
 * Copyright (C) 2021  Edward LEI <edward_lei72@hotmail.com>
 * I reformatted the code and added some adjustments
 *
 * license: MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rbtree.h"

#define DEBUG
#include "debug.h"


rbtree_t *rbt_create(int (*compare)(const void *, const void *),
                     void (*destroy)(void *))
{
  rbtree_t *rbt = (rbtree_t *)malloc(sizeof(rbtree_t));
  if (rbt == NULL) return NULL; /* out of memory */

  rbt->compare = compare;
  rbt->destroy = destroy;

  /* sentinel node nil */
  rbt->nil.left = rbt->nil.right = rbt->nil.parent = RBT_NIL(rbt);
  rbt->nil.color = RBT_BLACK;
  rbt->nil.data = NULL;

  /* sentinel node root */
  rbt->root.left = rbt->root.right = rbt->root.parent = RBT_NIL(rbt);
  rbt->root.color = RBT_BLACK;
  rbt->root.data = NULL;

  #ifdef RBT_MIN
  rbt->min = NULL;
  #endif

  return rbt;
}

static void _destroy(rbtree_t *rbt,
                     rbnode_t *n)
{
  if (n != RBT_NIL(rbt)) {
    _destroy(rbt, n->left);
    _destroy(rbt, n->right);
    rbt->destroy(n->data);
    free(n);
  }
}

void rbt_destroy(rbtree_t *rbt)
{
  _destroy(rbt, RBT_FIRST(rbt));
  free(rbt);
}

rbnode_t *rbt_find(rbtree_t *rbt,
                   void *data)
{
  rbnode_t *p = RBT_FIRST(rbt);

  while (p != RBT_NIL(rbt)) {
    int cmp;
    cmp = rbt->compare(data, p->data);
    if (cmp == 0)
      return p; /* found */
    p = cmp < 0 ? p->left : p->right;
  }

  return NULL;
}

rbnode_t *rbt_successor(rbtree_t *rbt,
                        rbnode_t *node)
{
  rbnode_t *p = node->right;

  if (p != RBT_NIL(rbt)) {
    /* move down until we find it */
    for ( ; p->left != RBT_NIL(rbt); p = p->left) ;
  }
  else {
    /* move up until we find it or hit the root */
    for (p = node->parent; node == p->right; node = p, p = p->parent) ;

    if (p == RBT_ROOT(rbt)) p = NULL; /* not found */
  }

  return p;
}

int rbt_apply(rbtree_t *rbt,
              rbnode_t *node,
              int (*func)(void *, void *),
              void *cookie,
              int order)
{
  int err;

  if (node != RBT_NIL(rbt)) {
    /* preorder */
    if (order == RBT_PREORDER && (err = func(node->data, cookie)) != 0)
      return err;

    /* left */
    if ((err = rbt_apply(rbt, node->left, func, cookie, order)) != 0)
      return err;

    /* inorder */
    if (order == RBT_INORDER && (err = func(node->data, cookie)) != 0)
      return err;

    /* right */
    if ((err = rbt_apply(rbt, node->right, func, cookie, order)) != 0)
      return err;

    /* postorder */
    if (order == RBT_POSTORDER && (err = func(node->data, cookie)) != 0)
      return err;
  }

  return 0;
}

static void _rotate_left(rbtree_t *rbt,
                         rbnode_t *x)
{
  rbnode_t *y = x->right; /* child */

  /* tree x */
  x->right = y->left;
  if (x->right != RBT_NIL(rbt))
    x->right->parent = x;

  /* tree y */
  y->parent = x->parent;
  if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  /* assemble tree x and tree y */
  y->left = x;
  x->parent = y;
}

static void _rotate_right(rbtree_t *rbt,
                          rbnode_t *x)
{
  rbnode_t *y = x->left; /* child */

  /* tree x */
  x->left = y->right;
  if (x->left != RBT_NIL(rbt))
    x->left->parent = x;

  /* tree y */
  y->parent = x->parent;
  if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  /* assemble tree x and tree y */
  y->right = x;
  x->parent = y;
}

static void _insert_fix(rbtree_t *rbt,
                        rbnode_t *current)
{
  rbnode_t *uncle;

  do {
    /* current node is RED and parent node is RED */
    if (current->parent == current->parent->parent->left) {
      uncle = current->parent->parent->right;
      if (uncle->color == RBT_RED) {
        /* insertion into 4-children cluster */

        /* split */
        current->parent->color = RBT_BLACK;
        uncle->color = RBT_BLACK;
        /* send grandparent node up the tree */
        current = current->parent->parent; /* goto loop or break */
        current->color = RBT_RED;
      }
      else {
        /* insertion into 3-children cluster */

        /* equivalent BST */
        if (current == current->parent->right) {
          current = current->parent;
          _rotate_left(rbt, current);
        }
        /* 3-children cluster has two representations */
        current->parent->color = RBT_BLACK; /* thus goto break */
        current->parent->parent->color = RBT_RED;
        _rotate_right(rbt, current->parent->parent);
      }
    }
    else {
      uncle = current->parent->parent->left;
      if (uncle->color == RBT_RED) {
        /* insertion into 4-children cluster */

        /* split */
        current->parent->color = RBT_BLACK;
        uncle->color = RBT_BLACK;
        /* send grandparent node up the tree */
        current = current->parent->parent; /* goto loop or break */
        current->color = RBT_RED;
      }
      else {
        /* insertion into 3-children cluster */

        /* equivalent BST */
        if (current == current->parent->left) {
          current = current->parent;
          _rotate_right(rbt, current);
        }
        /* 3-children cluster has two representations */
        current->parent->color = RBT_BLACK; /* thus goto break */
        current->parent->parent->color = RBT_RED;
        _rotate_left(rbt, current->parent->parent);
      }
    }
  } while (current->parent->color == RBT_RED);
}

rbnode_t *rbt_insert(rbtree_t *rbt,
                     void *data)
{
  rbnode_t *current, *parent;
  rbnode_t *new_node;

  /* do a binary search to find where it should be */
  current = RBT_FIRST(rbt);
  parent = RBT_ROOT(rbt);

  while (current != RBT_NIL(rbt)) {
    int cmp;
    pthread_mutex_lock(&rbt->mutex);
    cmp = rbt->compare(data, current->data);
    pthread_mutex_unlock(&rbt->mutex);

    #ifndef RBT_DUP
    if (cmp == 0) {
      rbt->destroy(current->data);
      current->data = data;
      return current; /* updated */
    }
    #endif

    if (cmp == 0) return current;

    parent = current;
    current = cmp < 0 ? current->left : current->right;
  }

  /* replace the termination NIL pointer with the new node pointer */

  current = new_node = (rbnode_t *)malloc(sizeof(rbnode_t));
  if (current == NULL) return NULL; /* out of memory */

  current->left = current->right = RBT_NIL(rbt);
  current->parent = parent;
  current->color = RBT_RED;
  current->data = data;

  if (parent == RBT_ROOT(rbt) || rbt->compare(data, parent->data) < 0)
    parent->left = current;
  else
    parent->right = current;

  #ifdef RBT_MIN
  if (rbt->min == NULL || rbt->compare(current->data, rbt->min->data) < 0)
    rbt->min = current;
  #endif

  /*
   * insertion into a red-black tree:
   *
   *   0-children root cluster (parent node is BLACK)
   *   becomes 2-children root cluster (new root node)
   *   -> paint root node BLACK, and done
   *
   *   2-children cluster (parent node is BLACK)
   *   becomes 3-children cluster
   *   -> done
   *
   *   3-children cluster (parent node is BLACK)
   *   becomes 4-children cluster
   *   -> done
   *
   *   3-children cluster (parent node is RED)
   *   becomes 4-children cluster
   *   -> rotate, and done
   *
   *   4-children cluster (parent node is RED)
   *   splits into 2-children cluster and 3-children cluster
   *   -> split, and insert grandparent node into parent cluster
   */
  if (current->parent->color == RBT_RED) {
    /* insertion into 3-children cluster (parent node is RED) */
    /* insertion into 4-children cluster (parent node is RED) */
    pthread_mutex_lock(&rbt->mutex);
    _insert_fix(rbt, current);
    pthread_mutex_unlock(&rbt->mutex);
  }
  else {
    /* insertion into 0-children root cluster (parent node is BLACK) */
    /* insertion into 2-children cluster (parent node is BLACK) */
    /* insertion into 3-children cluster (parent node is BLACK) */
  }

  /*
   * the root is always BLACK
   * insertion into 0-children root cluster or
   * insertion into 4-children root cluster require this recoloring
   */
  RBT_FIRST(rbt)->color = RBT_BLACK;

  return new_node;
}

static void _delete_fix(rbtree_t *rbt,
                        rbnode_t *current)
{
  rbnode_t *sibling;

  do {
    if (current == current->parent->left) {
      sibling = current->parent->right;

      if (sibling->color == RBT_RED) {
        /*
         * perform an adjustment
         * (3-children parent cluster has two representations)
         */
        sibling->color = RBT_BLACK;
        current->parent->color = RBT_RED;
        _rotate_left(rbt, current->parent);
        sibling = current->parent->right;
      }

      /* sibling node must be BLACK now */
      if (sibling->right->color == RBT_BLACK &&
          sibling->left->color == RBT_BLACK) {
        /* 2-children sibling cluster, fuse by recoloring */
        sibling->color = RBT_RED;
        /* 3/4-children parent cluster */
        if (current->parent->color == RBT_RED) {
          current->parent->color = RBT_BLACK;
          break; /* goto break */
        }
        /* 2-children parent cluster */
        else {
          current = current->parent; /* goto loop */
        }
      }
      else {
        /* 3/4-children sibling cluster */
        /*
         * perform an adjustment
         * (3-children sibling cluster has two representations)
         */
        if (sibling->right->color == RBT_BLACK) {
          sibling->left->color = RBT_BLACK;
          sibling->color = RBT_RED;
          _rotate_right(rbt, sibling);
          sibling = current->parent->right;
        }

        /* transfer by rotation and recoloring */
        sibling->color = current->parent->color;
        current->parent->color = RBT_BLACK;
        sibling->right->color = RBT_BLACK;
        _rotate_left(rbt, current->parent);
        break; /* goto break */
      }
    }
    else {
      sibling = current->parent->left;

      if (sibling->color == RBT_RED) {
        /*
         * perform an adjustment
         * (3-children parent cluster has two representations)
         */
        sibling->color = RBT_BLACK;
        current->parent->color = RBT_RED;
        _rotate_right(rbt, current->parent);
        sibling = current->parent->left;
      }

      /* sibling node must be BLACK now */
      if (sibling->right->color == RBT_BLACK &&
          sibling->left->color == RBT_BLACK) {
        /* 2-children sibling cluster, fuse by recoloring */
        sibling->color = RBT_RED;
        /* 3/4-children parent cluster */
        if (current->parent->color == RBT_RED) {
          current->parent->color = RBT_BLACK;
          break; /* goto break */
        }
        /* 2-children parent cluster */
        else {
          current = current->parent; /* goto loop */
        }
      }
      else {
        /* 3/4-children sibling cluster */
        /*
         * perform an adjustment
         * (3-children sibling cluster has two representations)
         */
        if (sibling->left->color == RBT_BLACK) {
          sibling->right->color = RBT_BLACK;
          sibling->color = RBT_RED;
          _rotate_left(rbt, sibling);
          sibling = current->parent->left;
        }

        /* transfer by rotation and recoloring */
        sibling->color = current->parent->color;
        current->parent->color = RBT_BLACK;
        sibling->left->color = RBT_BLACK;
        _rotate_right(rbt, current->parent);
        break; /* goto break */
      }
    }
  } while (current != RBT_FIRST(rbt));
}

void *rbt_delete(rbtree_t *rbt,
                 rbnode_t *node,
                 int keep)
{
  rbnode_t *target, *child;
  void *data;

  data = node->data;

  /* choose node's in-order successor if it has two children */
  if (node->left == RBT_NIL(rbt) || node->right == RBT_NIL(rbt)) {
    target = node;

    #ifdef RBT_MIN
    if (rbt->min == target)
      rbt->min = rbt_successor(rbt, target); /* deleted, thus min = successor */
    #endif
  }
  else {
    /* node->right must not be NIL, thus move down */
    target = rbt_successor(rbt, node);
    /* data swapped */
    node->data = target->data;

    #ifdef RBT_MIN
    /*
     * if min == node, then min = successor = node (swapped),
     * thus idle
     *
     * if min == target, then min = successor, which is not the minimal,
     * thus impossible
     */
    #endif
  }

  /* child may be NIL */
  child = (target->left == RBT_NIL(rbt)) ? target->right : target->left;

  /*
   * deletion from red-black tree:
   *
   *   4-children cluster (RED target node)
   *   becomes 3-children cluster
   *   -> done
   *
   *   3-children cluster (RED target node)
   *   becomes 2-children cluster
   *   -> done
   *
   *   3-children cluster (BLACK target node, RED child node)
   *   becomes 2-children cluster
   *   -> paint child node BLACK, and done
   *
   *   2-children root cluster (BLACK target node, BLACK child node)
   *   becomes 0-children root cluster
   *   -> done
   *
   *   2-children cluster (BLACK target node, 4-children sibling cluster)
   *   becomes 3-children cluster
   *   -> transfer, and done
   *
   *   2-children cluster (BLACK target node, 3-children sibling cluster)
   *   becomes 2-children cluster
   *   -> transfer, and done
   *
   *   2-children cluster (BLACK target node,
   *                       2-children sibling cluster,
   *                       3/4-children parent cluster)
   *   becomes 3-children cluster
   *   -> fuse, paint parent node BLACK, and done
   *
   *   2-children cluster (BLACK target node,
   *                       2-children sibling cluster,
   *                       2-children parent cluster)
   *   becomes 3-children cluster
   *   -> fuse, and delete parent node from parent cluster
   */
  if (target->color == RBT_BLACK) {
    if (child->color == RBT_RED) {
      /* deletion from 3-children cluster (BLACK target node, RED child node) */
      child->color = RBT_BLACK;
    }
    else if (target == RBT_FIRST(rbt)) {
      /*
       * deletion from 2-children root cluster
       * (BLACK target node, BLACK child node)
       */
    }
    else {
      /* deletion from 2-children cluster (BLACK target node, ...) */
      pthread_mutex_lock(&rbt->mutex);
      _delete_fix(rbt, target);
      pthread_mutex_unlock(&rbt->mutex);
    }
  }
  else {
    /* deletion from 4-children cluster (RED target node) */
    /* deletion from 3-children cluster (RED target node) */
  }

  if (child != RBT_NIL(rbt))
    child->parent = target->parent;

  if (target == target->parent->left)
    target->parent->left = child;
  else
    target->parent->right = child;

  free(target);

  /* keep or discard data */
  if (keep == 0) {
    rbt->destroy(data);
    data = NULL;
  }

  return data;
}

static int _check_order(rbtree_t *rbt,
                        rbnode_t *n,
                        void *min,
                        void *max)
{
  if (n == RBT_NIL(rbt)) return 1;

  #ifdef RBT_DUP
  if (rbt->compare(n->data, min) < 0 || rbt->compare(n->data, max) > 0)
  #else
  if (rbt->compare(n->data, min) <= 0 || rbt->compare(n->data, max) >= 0)
  #endif
    return 0;

  return _check_order(rbt, n->left, min, n->data) &&
         _check_order(rbt, n->right, n->data, max);
}

int rbt_order(rbtree_t *rbt,
              void *min,
              void *max)
{
  return _check_order(rbt, RBT_FIRST(rbt), min, max);
}

static int _check_black_height(rbtree_t *rbt,
                               rbnode_t *n)
{
  int lbh, rbh;

  if (n == RBT_NIL(rbt)) return 1;

  if (n->color == RBT_RED &&
      (n->left->color == RBT_RED ||
       n->right->color == RBT_RED ||
       n->parent->color == RBT_RED))
    return 0;

  if ((lbh = _check_black_height(rbt, n->left)) == 0) return 0;

  if ((rbh = _check_black_height(rbt, n->right)) == 0) return 0;

  if (lbh != rbh) return 0;

  return lbh + (n->color == RBT_BLACK ? 1 : 0);
}

int rbt_black_height(rbtree_t *rbt)
{
  if (RBT_ROOT(rbt)->color == RBT_RED ||
      RBT_FIRST(rbt)->color == RBT_RED ||
      RBT_NIL(rbt)->color == RBT_RED)
    return 0;

  return _check_black_height(rbt, RBT_FIRST(rbt));
}

static void _print(rbtree_t *rbt,
                   rbnode_t *n,
                   void (*print_func)(void *),
                   int depth,
                   char *label)
{
  if (n != RBT_NIL(rbt)) {
    _print(rbt, n->right, print_func, depth + 1, "R");
    printf("%*s", 8 * depth, "");
    if (label) printf("%s: ", label);
    print_func(n->data);
    printf(" (%s)\n", n->color == RBT_RED ? "r" : "b");
    _print(rbt, n->left, print_func, depth + 1, "L");
  }
}

void rbt_print(rbtree_t *rbt,
               void (*print_func)(void *))
{
  printf("\n--------------------------------------------------\n");
  _print(rbt, RBT_FIRST(rbt), print_func, 0, "T");
  printf("\n--------------------------------------------------\n");
  printf("black_height = %d\n", rbt_black_height(rbt));
}
