#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "tree.h"
#include "vn.h"
#include "wisetree.h"

// ----------------------------------------------------------------------------
// CONST SECTION
// ----------------------------------------------------------------------------

#include "ascii_arts.h"

#define I "\033[3m"
#define D "\033[0m"

const int INP_BUF_SIZE = 20;
const int CMD_LEN      = 64;
const int DELAY_USEC   = 2 * 1000000;

// ----------------------------------------------------------------------------
// DEF SECTION
// ----------------------------------------------------------------------------

static void get_input (char *line);

static void ask_mode_art (screen_t *screen);

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node, screen_t *screen);

static void wait ();

static bool definition_pre (tree::node_t *node, void *param, bool cont);
static bool definition_post (tree::node_t *node, void *param, bool cont);

// ----------------------------------------------------------------------------
// PUBLIC SECTION
// ----------------------------------------------------------------------------

void guess_mode (tree::tree_t *tree, screen_t *screen)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");

    tree::node_t *node = tree->head_node;

    put_line (screen, "Вопросик принят на карандаш, работаем...");
    render   (screen, render_mode_t::MIKU);

    usleep (DELAY_USEC);

    put_line (screen, "Кабанчик вернулся и доложил, что вам придется поотвечать на вопросики. Начнем");
    put_line (screen, "");

    char input[INP_BUF_SIZE];
    int n_quest = 0;    

    while (true)
    {
        n_quest++;

        put_line (screen, "Вопроc #%d: %s? (да/нет) ", n_quest, (char *) node->value);
        render   (screen, render_mode_t::MIKU);

        get_input (input);

        if (strcasecmp (input, "да") == 0)
        {
            node = node->left;
            if (node == nullptr)
            {
                put_line (screen, "Лол, а сам ты не мог этого понять?");
                render (screen, render_mode_t::ANON);
                wait ();
                break;
            }
        }
        else if (strcasecmp (input, "нет") == 0)
        {
            if (node->right == nullptr)
            {
                add_unknown_object (tree, node, screen);
                put_line (screen, "Не ну ля такое не считается");
                render (screen, render_mode_t::ANON);
                wait ();
                break;
            }

            node = node->right;
        }
    }
}

// ----------------------------------------------------------------------------

void definition_mode (tree::tree_t *tree, screen_t *screen)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");

    char input[OBJ_SIZE+1] = "";

    put_line (screen, "Ну, пирожок, что же ты хочешь узнать?");
    put_line (screen, "");
    put_line (screen, "Ваш жалкий объект: ");
    render   (screen, render_mode_t::ANON);

    get_input (input);

    if (tree::dfs_exec (tree, definition_pre,    input, 
                                nullptr,         nullptr,
                                definition_post, screen))
    {
        put_line (screen, "Я такого не знаю, иди к тете Гале");
        render (screen, render_mode_t::ANON);
        wait ();
    }
    else
    {
        put_line (screen, "");
        put_line (screen, "Я рад что ты хотя бы изображаешь попытки что-то узнать");
        render (screen, render_mode_t::ANON);
        wait ();
    }
}

// ----------------------------------------------------------------------------

void dump_mode (tree::tree_t *tree, screen_t *screen)
{
    assert (tree != nullptr && "invalid pointer");

    int dump_num = tree::graph_dump (tree, "Dump mode asked");

    char cmd[CMD_LEN] = "";
    sprintf (cmd, "xdg-open dump/%d.grv.png", dump_num);

    system (cmd);
}

// ----------------------------------------------------------------------------

void run_wisetree (tree::tree_t *tree, screen_t *screen)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");

    printf (WISE_TREE_ASCII);
    usleep (DELAY_USEC);

    char input[OBJ_SIZE+1] = "";

    while (true)
    {
        ask_mode_art (screen);

        get_input (input);

        if (strcasecmp (input, "1") == 0)
        {
            guess_mode (tree, screen);
        }
        else if (strcasecmp (input, "2") == 0)
        {
            definition_mode (tree, screen);
        }
        else if (strcasecmp (input, "4") == 0)
        {
            dump_mode (tree, screen);
        }
        else
        {
            put_line (screen, "Че? Миша, давай по новой");
            render (screen, render_mode_t::ANON);
            wait ();
        }
    }
}

// ----------------------------------------------------------------------------
// STATIC SECTION
// ----------------------------------------------------------------------------

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node, screen_t *screen)
{
    assert (tree     != nullptr && "invalid pointer");
    assert (bad_node != nullptr && "invalid pointer");

    char buf[OBJ_SIZE + 1];

    put_line (screen, "Ну, гений мысли, и что же ты загадал?");
    render (screen, render_mode_t::ANON);
    get_input (buf);

    tree::node_t *good_node  = tree::new_node (buf, OBJ_SIZE);

    put_line (screen, "Ох, дружок, а сформулировать чем это отличается от '%s' сможешь то?", (char *) bad_node->value);
    put_line (screen, "Это...");
    render   (screen, render_mode_t::ANON);
    get_input (buf);
    
    tree::node_t *new_bad_node  = tree::new_node (bad_node->value, OBJ_SIZE);

    tree::change_value (tree, bad_node, buf);

    bad_node->left  = good_node;
    bad_node->right = new_bad_node;
}

// ----------------------------------------------------------------------------

static void get_input (char *line)
{
    fgets (line, OBJ_SIZE, stdin);
    *(strchr (line, '\n')) = '\0';
}

// ----------------------------------------------------------------------------

static bool definition_pre (tree::node_t *node, void *param, bool cont)
{
    assert (node  != nullptr && "invalid pointer");
    assert (param != nullptr && "invalid pointer");
    assert (cont && "invalid call");

    if (strcasecmp ((char *) node->value, (char *) param) == 0)
    {
        return false;
    }

    return true;
}

static bool definition_post (tree::node_t *node, void *param, bool cont)
{
    assert (node  != nullptr && "invalid pointer");
    assert (param != nullptr && "invalid pointer");

    if (!cont)
    {
        put_line ((screen_t *) param, "Свойство: %s", (char *) node->value);
    }

    return true;
}

// ----------------------------------------------------------------------------

static void ask_mode_art (screen_t *screen)
{
    put_line (screen, "    Выберите режим Мудрого Дерева      ");
    put_line (screen, "                                       ");
    put_line (screen, "1) Интерактивный диалог с просветленным");
    put_line (screen, "2) Получение справки в лицо            ");
    put_line (screen, "3) Различие между объектами            ");
    put_line (screen, "4) Графический дамп                    ");

    render (screen, render_mode_t::ANON);
}

// ----------------------------------------------------------------------------

static void wait ()
{
    getchar ();
}
