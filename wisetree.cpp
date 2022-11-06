#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "lib/log.h"
#include "common.h"
#include "tree.h"
#include "vn.h"
#include "wisetree.h"

// ----------------------------------------------------------------------------
// CONST SECTION
// ----------------------------------------------------------------------------

#include "ascii_arts.h"

const int INP_BUF_SIZE = 20;
const int CMD_LEN      = 100;
const int DELAY_USEC   = 2 * 1000000;
const int MAX_HEIGHT   = 64;

// ----------------------------------------------------------------------------
// TYPE SECTION
// ----------------------------------------------------------------------------

struct edge_info_t
{
    tree::node_t *node;
    bool is_true;
};

struct node_path
{
    unsigned int size               = 0;
    edge_info_t stack[MAX_HEIGHT]  = {};
};


// ----------------------------------------------------------------------------
// DEF SECTION
// ----------------------------------------------------------------------------

static void get_input (char *line);

static void ask_mode (screen_t *screen);

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node, screen_t *screen);

static void wait ();

static bool compare_node (tree::node_t *node, void *param, bool cont);
static void print_properties (screen_t *screen, const node_path *path);

static bool get_node_paths (tree::tree_t *tree, screen_t *screen,
                            node_path *path_one, node_path *path_two,
                            char *obj_one, char *obj_two);

static void print_diff (screen_t *screen, const node_path *one, const node_path *two);

static bool remember_node (tree::node_t *node, void *param, bool cont);

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

    put_line (screen, "Кабанчик вернулся и доложил, что вам придется");
    put_line (screen, "поотвечать на вопросики. Начнем");
    put_line (screen, "");

    char input[INP_BUF_SIZE];
    int n_quest = 0;    

    while (true)
    {
        put_line (screen, "Вопрос #%d: %s? (да/нет) ", n_quest, (char *) node->value);
        render   (screen, render_mode_t::MIKU);

        get_input (input);

        if (strcasecmp (input, "да") == 0)
        {
            n_quest++;

            node = node->left;
            if (node == nullptr)
            {
                put_line (screen, "Иди-ка ты K&R читать");
                render (screen, render_mode_t::ANON);
                wait ();
                break;
            }
        }
        else if (strcasecmp (input, "нет") == 0)
        {
            n_quest++;

            if (node->right == nullptr)
            {
                add_unknown_object (tree, node, screen);
                put_line (screen, "Не ну такое не считается");
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
    put_line (screen, "Ваша жалкая ошибка: ");
    render   (screen, render_mode_t::ANON);

    get_input (input);

    node_path path = {};

    if (tree::dfs_exec (tree, compare_node, input,
                          nullptr, nullptr, 
                          remember_node, &path))
    {
        put_line (screen, "Я такого не знаю, иди к тете Гале");
        render (screen, render_mode_t::ANON);
        wait ();
    }
    else
    {
        print_properties (screen, &path);

        put_line (screen, "");
        put_line (screen, "Я рад что ты хотя бы изображаешь попытки что-то узнать");
        render (screen, render_mode_t::ANON);
        wait ();
    }
}

// ----------------------------------------------------------------------------

void dump_mode (tree::tree_t *tree, screen_t *screen)
{
    assert (tree   != nullptr && "invalid pointer");
    assert (screen != nullptr && "invalid pointer");

    int dump_num = tree::graph_dump (tree, "Dump mode asked");

    char cmd[CMD_LEN] = "";
    sprintf (cmd, "xdg-open dump/%d.grv.png", dump_num);

    system (cmd);
}

// ----------------------------------------------------------------------------

void diff_mode (tree::tree_t *tree, screen_t *screen)
{
    assert (tree   != nullptr && "invalid pointer");
    assert (screen != nullptr && "invalid pointer");

    char obj_one[OBJ_SIZE + 1] = "";
    char obj_two[OBJ_SIZE + 1] = "";

    put_line (screen, "Сейчас вас попросят ввести два стула");
    render (screen, render_mode_t::MIKU);
    wait ();

    put_line (screen, "Введите первый");
    render (screen, render_mode_t::ANON);
    get_input (obj_one);    

    put_line (screen, "Введите второй");
    render (screen, render_mode_t::ANON);
    get_input (obj_two);    

    node_path path_one = {};
    node_path path_two = {};

    if (!get_node_paths (tree, screen, &path_one, &path_two, obj_one, obj_two))
    {
        print_diff (screen, &path_one, &path_two);
    }
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
        ask_mode (screen);

        get_input (input);

        if (strcasecmp (input, "1") == 0)
        {
            guess_mode (tree, screen);
        }
        else if (strcasecmp (input, "2") == 0)
        {
            definition_mode (tree, screen);
        }
        else if (strcasecmp (input, "3") == 0)
        {
            diff_mode (tree, screen);
        }
        else if (strcasecmp (input, "4") == 0)
        {
            dump_mode (tree, screen);
        }
        else if (strcasecmp (input, "5") == 0)
        {
            FILE *dump_file  = fopen (DUMP_FILE, "w");
            if (!dump_file) { log (log::ERR, "Failed to open dump file"); return; }

            tree::store (tree, dump_file);
            fclose (dump_file);
            return;
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

    put_line (screen, "Ох, дружок, а сформулировать чем это отличается от ");
    put_line (screen, "'%s' сможешь то?", (char *) bad_node->value);
    put_line (screen, "");
    put_line (screen, "...");
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

static bool compare_node (tree::node_t *node, void *param, bool cont)
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

// ----------------------------------------------------------------------------

static bool remember_node (tree::node_t *node, void *param, bool cont)
{
    assert (node  != nullptr && "invalid pointer");
    assert (param != nullptr && "invalid pointer");

    node_path *path = (node_path *) param; 

    if (!cont)
    {
        path->stack[path->size].node = node;

        if (path->size > 0)
        {
            if (path->stack[path->size-1].node == node->left)
            {
                path->stack[path->size].is_true = true;
            }
            else
            {
                path->stack[path->size].is_true = false;
            }
        }
        else /*path->size == 0*/
        {
            path->stack[path->size].is_true = true;
        }

        path->size++;
    }

    return true;
}

// ----------------------------------------------------------------------------

static bool get_node_paths (tree::tree_t *tree, screen_t *screen,
                            node_path *path_one, node_path *path_two,
                            char *obj_one, char *obj_two)
{
    bool res = false;

    res = tree::dfs_exec (tree, compare_node, obj_one,
                          nullptr, nullptr, 
                          remember_node, path_one);

    if (res)
    {
        put_line (screen, "Я не нашел первый стул, сорян");
        render (screen, render_mode_t::ANON);
        wait ();
        return res;
    }

    res = tree::dfs_exec (tree, compare_node, obj_two,
                          nullptr, nullptr, 
                          remember_node, path_two);

    if (res)
    {
        put_line (screen, "Я не нашел второй стул, сорян");
        render (screen, render_mode_t::ANON);
        wait ();
        return res;
    }

    return res;
}

// ----------------------------------------------------------------------------

#define PUT_NO_IF_NOT(val)              \
{                                       \
    if (val)   put_text (screen, "");   \
    else       put_text (screen, "¬");  \
}

static void print_diff (screen_t *screen, const node_path *one, const node_path *two)
{
    assert (screen != nullptr && "invalid pointer");
    assert (one    != nullptr && "invalid pointer");
    assert (two    != nullptr && "invalid pointer");
    assert (one->size > 0 && "diff for not found value");
    assert (two->size > 0 && "diff for not found value");

    int indx_one = (int) one->size - 1;
    int indx_two = (int) two->size - 1;

    put_line (screen, "Чтож, если ты не способен отличить");
    put_line (screen, "эти два стула, я тебе помогу");

    render (screen, render_mode_t::ANON);
    wait ();

    put_line (screen, "Ну, у этих объектов есть сходства. Например, они оба:");

    while (one->stack[indx_one].node    == two->stack[indx_two].node && 
           one->stack[indx_one].is_true == two->stack[indx_two].is_true )
    {
        PUT_NO_IF_NOT (one->stack[indx_one].is_true);
        
        put_line (screen, "%s", one->stack[indx_one].node->value);
        indx_one--;
        indx_two--;

        if (indx_one < 0 || indx_two < 0)
        {
            break;
        }
    }

    if (indx_one >= 0 || indx_two >= 0)
    {
        put_line (screen, "Однако, они все таки не одинаковы");
        put_line (screen, "");

        if (indx_one >= 0)
        {
            put_line (screen, "Так, например, первый");
            while (indx_one >= 0)
            {
                PUT_NO_IF_NOT (one->stack[indx_one].is_true);
                put_line (screen, "%s", one->stack[indx_one].node->value);
                indx_one--;
            }
            put_line (screen, "");
        }

        if (indx_two >= 0)
        {
            put_line (screen, "Второй отличается наличием");
            put_line (screen, "");

            while (indx_two >= 0)
            {
                PUT_NO_IF_NOT (two->stack[indx_two].is_true);
                put_line (screen, "%s", two->stack[indx_two].node->value);
                indx_two--;
            }
        }
    }

    render (screen, render_mode_t::ANON);
    wait ();
}

// ----------------------------------------------------------------------------

static void print_properties (screen_t *screen, const node_path *path)
{
    assert (screen != nullptr && "invalid pointer");
    assert (path   != nullptr && "invalid pointer");
    assert (path->size > 0 && "properties for not found node");

    put_line (screen, "Тащемта, свойства данный объект не rocket science");
    put_line (screen, "И обладает понятными свойствами:");
    put_line (screen, "");

    for (int indx = (int) path->size - 1; indx >= 0; indx--)
    {
        PUT_NO_IF_NOT (path->stack[indx].is_true);
        put_line (screen, "%s", path->stack[indx].node->value);
    }

    render (screen, render_mode_t::ANON);
    wait ();
}

// ----------------------------------------------------------------------------

static void ask_mode (screen_t *screen)
{
    put_line (screen, "    Выберите режим Мудрого Дерева      ");
    put_line (screen, "                                       ");
    put_line (screen, "1) Интерактивный диалог с просветленным");
    put_line (screen, "2) Получение справки в лицо            ");
    put_line (screen, "3) Различие между объектами            ");
    put_line (screen, "4) Графический дамп                    ");
    put_line (screen, "5) Выйди и сохрани                     ");

    render (screen, render_mode_t::ANON);
}

// ----------------------------------------------------------------------------

static void wait ()
{
    getchar ();
}
