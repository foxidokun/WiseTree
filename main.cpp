#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "tree.h"

int strcmp_wrapper (const void *lhs, const void *rhs)
{
    return strcmp ((const char *) lhs, (const char *) rhs);
}

int main ()
{
    FILE *lol = fopen ("dump.txt", "w");

    tree::tree_t tree = {};
    tree::ctor (&tree, 64, strcmp_wrapper);

    char str[64] = "Мать жива";

    tree::insert (&tree, str);

    strcpy (str, "Человек");
    tree::insert_left (&tree, tree.head_node, str);

    strcpy (str, "Дотер");
    tree::insert_right (&tree, tree.head_node, str);

    tree::dump (&tree, lol);

    tree::dtor (&tree);
}