#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>

namespace tree
{
    struct node_t
    {
        void *value;

        node_t *left;
        node_t *right;
    };

    struct tree_t
    {
        int (*objcmp)(const void*, const void *);
        size_t obj_size;

        node_t *head_node;
    };

    enum tree_err_t
    {
        OK = 0,
        OOM,
    };

    typedef void (*walk_f)(node_t *node, void *param);

    void ctor (tree_t *tree, size_t obj_size, int (*objcmp)(const void *, const void *));
    void dtor (tree_t *tree);

    tree_err_t insert (tree_t *tree, const void *elem);
    tree_err_t insert_left  (tree_t *tree, node_t *node, const void *elem);
    tree_err_t insert_right (tree_t *tree, node_t *node, const void *elem);

    void change_value (tree_t *tree, node_t *node, const void *elem);

    void dfs_exec (tree_t *tree, walk_f pre_exec,  void *pre_param,
                                 walk_f in_exec,   void *in_param,
                                 walk_f post_exec, void *post_param);


    void dump (tree_t *tree, FILE *stream);
    
    static inline void do_nothing (node_t *, void *) {;}    
}

#endif //TREE_H