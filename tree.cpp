#include <assert.h>
#include <string.h>

#include "tree.h"

// ----------------------------------------------------------------------------
// CONST SECTION
// ----------------------------------------------------------------------------

const tree::node_t DEFAULT_NODE = {
    nullptr,    // value
    nullptr,    // left
    nullptr,    // right
};

// ----------------------------------------------------------------------------
// STATIC PROTOTYPES SECTION
// ----------------------------------------------------------------------------

static tree::node_t *new_node (const void *elem, size_t obj_size);

static void dfs_recursion (tree::node_t *node, tree::walk_f pre_exec,  void *pre_param,
                                               tree::walk_f in_exec,   void *in_param,
                                               tree::walk_f post_exec, void *post_param);

// ----------------------------------------------------------------------------
// PUBLIC SECTION
// ----------------------------------------------------------------------------

void tree::ctor (tree_t *tree, size_t obj_size, int (*objcmp)(const void *, const void *))
{
    assert (tree   != nullptr && "invalid pointer");
    assert (objcmp != nullptr && "invalid pointer");

    tree->objcmp    = objcmp;
    tree->obj_size  = obj_size;
    tree->head_node = nullptr;
}

void tree::dtor (tree_t *tree)
{
    assert (tree != nullptr && "invalid pointer");

    tree::walk_f free_node_func = [](node_t* node, void *param){ free(node); };

    dfs_exec (tree, do_nothing,     nullptr,
                    do_nothing,     nullptr,
                    free_node_func, nullptr);
}

// ----------------------------------------------------------------------------

tree::tree_err_t tree::insert (tree_t *tree, const void *elem)
{
    assert (tree != nullptr && "invalid pointer");
    assert (elem != nullptr && "invalid pointer");

    node_t *allocated_node = new_node (elem, tree->obj_size);
    if (allocated_node == nullptr) { return OOM; };

    node_t *current_node = tree->head_node;

    if (current_node == nullptr)
    {
        tree->head_node = allocated_node;
        return OK;
    }

    int cmpres = 0;

    while (true)
    {
        cmpres = tree->objcmp (current_node->value, elem);

        if (cmpres > 0)
        {
            if (current_node->right != nullptr)
            {
                current_node = current_node->right;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (current_node->left != nullptr)
            {
                current_node = current_node->left;
            }
            else
            {
                break;
            }
        }
    }

    if (cmpres > 0) current_node->right = allocated_node;
    else            current_node->left  = allocated_node;

    return OK;
}

// ----------------------------------------------------------------------------

tree::tree_err_t tree::insert_left  (tree_t *tree, node_t *node, const void *elem)
{
    assert (tree != nullptr && "inVALid poointer");
    assert (node != nullptr && "inVALid poointer");
    assert (elem != nullptr && "inVALid poointer");

    assert (node->left == nullptr && "cringe bro");

    node_t *allocated_node = new_node (elem, tree->obj_size); 
    if (allocated_node == nullptr) { return OOM; }

    node->left = allocated_node;

    return OK;
}

// ----------------------------------------------------------------------------

tree::tree_err_t tree::insert_right  (tree_t *tree, node_t *node, const void *elem)
{
    assert (tree != nullptr && "inVALid poointer");
    assert (node != nullptr && "inVALid poointer");
    assert (elem != nullptr && "inVALid poointer");

    assert (node->right == nullptr && "cringe bro");

    node_t *allocated_node = new_node (elem, tree->obj_size); 
    if (allocated_node == nullptr) { return OOM; }

    node->right = allocated_node;

    return OK;
}

// ----------------------------------------------------------------------------

void tree::change_value (tree_t *tree, node_t *node, const void *elem)
{
    assert (tree != nullptr && "inVALid poointer");
    assert (node != nullptr && "inVALid poointer");
    assert (elem != nullptr && "inVALid poointer");

    memcpy (node + 1, elem, tree->obj_size);
}

// ----------------------------------------------------------------------------

void tree::dfs_exec (tree_t *tree, walk_f pre_exec,  void *pre_param,
                                   walk_f in_exec,   void *in_param,
                                   walk_f post_exec, void *post_param)
{
    assert (tree      != nullptr && "invalid pointer");
    assert (pre_exec  != nullptr && "invalid pointer");
    assert (in_exec   != nullptr && "invalid pointer");
    assert (post_exec != nullptr && "invalid pointer");

    if (tree->head_node != nullptr)
    {
        dfs_recursion (tree->head_node, pre_exec,  pre_param,
                                        in_exec,   in_param,
                                        post_exec, post_param);
    }
}

// ----------------------------------------------------------------------------

void tree::dump (tree_t *tree, FILE *stream)
{
    assert (tree   != nullptr && "invalid pointer");
    assert (stream != nullptr && "invalid pointer");

    walk_f pre_exec = [](node_t *node, void *stream)
        {   
            fprintf ((FILE *) stream, "{ '%s'\n", (char *) node->value);
        };
    
    walk_f post_exec = [](node_t *, void *stream)
        {   
            fprintf ((FILE *) stream, "}\n");
        };


    dfs_exec (tree, pre_exec,   stream,
                    do_nothing, nullptr,
                    post_exec,  stream);
}

// ----------------------------------------------------------------------------
// PRIVATE SECTION
// ----------------------------------------------------------------------------

static tree::node_t *new_node (const void *elem, size_t obj_size)
{
    assert (elem != nullptr && "invalid pointer");
    assert (obj_size != 0 && "invalid size");

    tree::node_t *node = (tree::node_t *) calloc (sizeof (tree::node_t) + obj_size, 1);
    if (node == nullptr) { return nullptr; }

    memcpy (node, &DEFAULT_NODE, sizeof (tree::node_t));
    memcpy (node + 1, elem, obj_size);
    
    node->value = node + 1;

    return node;
}

// ----------------------------------------------------------------------------

static void dfs_recursion (tree::node_t *node, tree::walk_f pre_exec,  void *pre_param,
                                               tree::walk_f in_exec,   void *in_param,
                                               tree::walk_f post_exec, void *post_param)
{
    assert (node      != nullptr && "invalid pointer");
    assert (pre_exec  != nullptr && "invalid pointer");
    assert (in_exec   != nullptr && "invalid pointer");
    assert (post_exec != nullptr && "invalid pointer");

    pre_exec (node, pre_param);

    if (node->left != nullptr)
    {
        dfs_recursion (node->left, pre_exec,  pre_param,
                                   in_exec,   in_param,
                                   post_exec, post_param);
    }

    in_exec (node, in_param);

    if (node->right != nullptr)
    {
        dfs_recursion (node->right, pre_exec,  pre_param,
                                    in_exec,   in_param,
                                    post_exec, post_param);
    }

    post_exec (node, post_param);
}