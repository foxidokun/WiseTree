#include <assert.h>
#include <string.h>

#include "common.h"
#include "tree.h"
#include "wisetree.h"

// ----------------------------------------------------------------------------
// CONST SECTION
// ----------------------------------------------------------------------------


const char WISE_TREE_ASCII[] = \
" __      __.__                ___________                      \n"
"/  \\    /  \\__| ______ ____   \\__    ___/______   ____   ____  \n"
"\\   \\/\\/   /  |/  ___// __ \\    |    |  \\_  __ \\_/ __ \\_/ __ \\ \n"
" \\        /|  |\\___ \\  ___/     |    |   |  | \\/\\  ___/\\  ___/ \n"
"  \\__/\\  / |__/____  >\\___  >   |____|   |__|    \\___  >\\___  >\n"
"       \\/          \\/     \\/                         \\/     \\/ \n";

const int INP_BUF_SIZE = 20;

// ----------------------------------------------------------------------------
// DEF SECTION
// ----------------------------------------------------------------------------

static void get_input (char *line);

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node);

// ----------------------------------------------------------------------------
// PUBLIC SECTION
// ----------------------------------------------------------------------------

void guess_mode (tree::tree_t *tree)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");
    printf (WISE_TREE_ASCII);

    tree::node_t *node = tree->head_node;

    char input[INP_BUF_SIZE];

    while (true)
    {
        printf ("%s?\n (да/нет): ", (char *) node->value);
        get_input (input);

        if (strcasecmp (input, "да") == 0)
        {
            node = node->left;
            if (node == nullptr)
            {
                printf ("Лол, а сам не мог?\n");
                break;
            }
        }
        else if (strcasecmp (input, "нет") == 0)
        {
            if (node->right == nullptr)
            {
                add_unknown_object (tree, node);
                printf ("Не ну бля такое не считается\n");
                break;
            }

            node = node->right;
        }
    }
}

// ----------------------------------------------------------------------------

void definition_mode (tree::tree_t *tree)
{
    assert (tree != nullptr && "invalid pointer");


}

// ----------------------------------------------------------------------------
// STATIC SECTION
// ----------------------------------------------------------------------------

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node)
{
    assert (tree     != nullptr && "invalid pointer");
    assert (bad_node != nullptr && "invalid pointer");

    char buf[OBJ_SIZE + 1];

    printf ("Ну, гений мысли, и что же ты загадал?\n");
    get_input (buf);

    tree::node_t *good_node  = tree::new_node (buf, OBJ_SIZE);

    printf ("Ох, дружок, а сформулировать чем это отличается от '%s' сможешь то?\nЭто ... ", (char *) bad_node->value);
    get_input (buf);
    
    tree::node_t *new_bad_node  = tree::new_node (bad_node->value, OBJ_SIZE);

    tree::change_value (tree, bad_node, buf);

    bad_node->left  = good_node;
    bad_node->right = new_bad_node;
}


static void get_input (char *line)
{
    fgets (line, OBJ_SIZE, stdin);
    *(strchr (line, '\n')) = '\0';
}
