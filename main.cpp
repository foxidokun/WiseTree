#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#include "lib/log.h"

#include "tree.h"

int strcmp_wrapper (const void *lhs, const void *rhs)
{
    return strcmp ((const char *) lhs, (const char *) rhs);
}

int main ()
{
    FILE *lol  = fopen ("dump.txt", "r");
    FILE *logs = fopen ("log.html", "w");

    set_log_stream (logs);

    tree::tree_t tree = {};
    tree::ctor (&tree, OBJ_SIZE, strcmp_wrapper);

    tree::load (&tree, lol);

    tree::store (&tree, lol);
    tree::graph_dump (&tree, "For lulz");

    tree::dtor (&tree);
}