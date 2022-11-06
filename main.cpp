#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "lib/log.h"
#include "tree.h"
#include "wisetree.h"

int strcmp_wrapper (const void *lhs, const void *rhs);

int main ()
{
    FILE *log_file = fopen ("log.html", "w");
    FILE *dump_file = fopen (DUMP_FILE, "r");

    if (!log_file) fprintf (stderr, "Failed to open log file");

    set_log_stream (log_file);

    if (!dump_file) log (log::ERR, "Failed to open dump file");

    tree::tree_t tree = {};
    tree::ctor (&tree, OBJ_SIZE, strcmp_wrapper);
    
    if (tree::load (&tree, dump_file) != tree::OK)
    {
        log (log::ERR, "Failed to load tree");
        return -1;
    }

    fclose (dump_file);

    tree::graph_dump (&tree, "For lulz");

    screen_t screen = {};
    screen_ctor (&screen, stdout);

    run_wisetree (&tree, &screen);

    tree::dtor (&tree);
}


int strcmp_wrapper (const void *lhs, const void *rhs)
{
    return strcmp ((const char *) lhs, (const char *) rhs);
}
