#ifndef WISETREE_H
#define WISETREE_H

#include "vn.h"
#include "tree.h"

void guess_mode (tree::tree_t *tree, screen_t *screen);
void definition_mode (tree::tree_t *tree, screen_t *screen);
void dump_mode (tree::tree_t *tree, screen_t *screen);
void dump_mode (tree::tree_t *tree, screen_t *screen);

void run_wisetree (tree::tree_t *tree, screen_t *screen);

#endif
