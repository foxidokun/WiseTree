#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "tree.h"
#include "wisetree.h"

// ----------------------------------------------------------------------------
// CONST SECTION
// ----------------------------------------------------------------------------

#define I "\033[3m"
#define D "\033[0m"

const char WISE_TREE_ASCII[] = 
"\t\t __      __.__                ___________                              \n"
"\t\t/  \\    /  \\__| ______ ____   \\__    ___/______   ____   ____       \n"
"\t\t\\   \\/\\/   /  |/  ___// __ \\    |    |  \\_  __ \\_/ __ \\_/ __ \\ \n"
"\t\t \\        /|  |\\___ \\  ___/     |    |   |  | \\/\\  ___/\\  ___/   \n"
"\t\t  \\__/\\  / |__/____  >\\___  >   |____|   |__|    \\___  >\\___  >   \n"
"\t\t       \\/          \\/     \\/                         \\/     \\/    \n";

const char ANON[21][125] = {
"⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⣀⣀⣤⣤⣴⣶⣶⣶⣶⣶⣶⣶⣶⣶⣶⣶⣶⣶⣦⣤⣤⣄⣀⡀⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⣴⣿⣿⡿⣿⢿⣟⣿⣻⣟⡿⣟⣿⣟⡿⣟⣿⣻⣟⣿⣻⢿⣻⡿⣿⢿⣷⣆⠄⠄⠄",
"⠄⠄⠄⢘⣿⢯⣷⡿⡿⡿⢿⢿⣷⣯⡿⣽⣞⣷⣻⢯⣷⣻⣾⡿⡿⢿⢿⢿⢯⣟⣞⡮⡀⠄⠄",
"⠄⠄⠄⢸⢞⠟⠃⣉⢉⠉⠉⠓⠫⢿⣿⣷⢷⣻⣞⣿⣾⡟⠽⠚⠊⠉⠉⠉⠙⠻⣞⢵⠂⠄⠄",
"⠄⠄⠄⢜⢯⣺⢿⣻⣿⣿⣷⣔⡄⠄⠈⠛⣿⣿⡾⠋⠁⠄⠄⣄⣶⣾⣿⡿⣿⡳⡌⡗⡅⠄⠄",
"⠄⠄⠄⢽⢱⢳⢹⡪⡞⠮⠯⢯⡻⡬⡐⢨⢿⣿⣿⢀⠐⡥⣻⡻⠯⡳⢳⢹⢜⢜⢜⢎⠆⠄⠄",
"⠄⠄⠠⣻⢌⠘⠌⡂⠈⠁⠉⠁⠘⠑⢧⣕⣿⣿⣿⢤⡪⠚⠂⠈⠁⠁⠁⠂⡑⠡⡈⢮⠅⠄⠄",
"⠄⠄⠠⣳⣿⣿⣽⣭⣶⣶⣶⣶⣶⣺⣟⣾⣻⣿⣯⢯⢿⣳⣶⣶⣶⣖⣶⣮⣭⣷⣽⣗⠍⠄⠄",
"⠄⠄⢀⢻⡿⡿⣟⣿⣻⣽⣟⣿⢯⣟⣞⡷⣿⣿⣯⢿⢽⢯⣿⣻⣟⣿⣻⣟⣿⣻⢿⣿⢀⠄⠄",
"⠄⠄⠄⡑⡏⠯⡯⡳⡯⣗⢯⢟⡽⣗⣯⣟⣿⣿⣾⣫⢿⣽⠾⡽⣺⢳⡫⡞⡗⡝⢕⠕⠄⠄⠄",
"⠄⠄⠄⢂⡎⠅⡃⢇⠇⠇⣃⣧⡺⡻⡳⡫⣿⡿⣟⠞⠽⠯⢧⣅⣃⠣⠱⡑⡑⠨⢐⢌⠂⠄⠄",
"⠄⠄⠄⠐⠼⣦⢀⠄⣶⣿⢿⣿⣧⣄⡌⠂⠢⠩⠂⠑⣁⣅⣾⢿⣟⣷⠦⠄⠄⡤⡇⡪⠄⠄⠄",
"⠄⠄⠄⠄⠨⢻⣧⡅⡈⠛⠿⠿⠿⠛⠁⠄⢀⡀⠄⠄⠘⠻⠿⠿⠯⠓⠁⢠⣱⡿⢑⠄⠄⠄⠄",
"⠄⠄⠄⠄⠈⢌⢿⣷⡐⠤⣀⣀⣂⣀⢀⢀⡓⠝⡂⡀⢀⢀⢀⣀⣀⠤⢊⣼⡟⡡⡁⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠈⢢⠚⣿⣄⠈⠉⠛⠛⠟⠿⠿⠟⠿⠻⠻⠛⠛⠉⠄⣠⠾⢑⠰⠈⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⠄⠑⢌⠿⣦⡡⣱⣸⣸⣆⠄⠄⠄⣰⣕⢔⢔⠡⣼⠞⡡⠁⠁⠄⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⠄⠄⠄⠑⢝⢷⣕⡷⣿⡿⠄⠄⠠⣿⣯⣯⡳⡽⡋⠌⠄⠄⠄⠄⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠙⢮⣿⣽⣯⠄⠄⢨⣿⣿⡷⡫⠃⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠘⠙⠝⠂⠄⢘⠋⠃⠁⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄",
"⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄"
};

const int INP_BUF_SIZE = 20;

// ----------------------------------------------------------------------------
// DEF SECTION
// ----------------------------------------------------------------------------

static void get_input (char *line);

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node);

static bool definition_pre (tree::node_t *node, void *param, bool cont);
static bool definition_post (tree::node_t *node, void *param, bool cont);

// ----------------------------------------------------------------------------
// PUBLIC SECTION
// ----------------------------------------------------------------------------

void guess_mode (tree::tree_t *tree)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");

    tree::node_t *node = tree->head_node;

    printf (I "\nВопросик принят на карандаш, работаем...\n" D);
    usleep (5 * 1000000);
    printf (I "Кабанчик вернулся и доложил, что вам придется поотвечать на вопросики. Начнем\n" D);

    char input[INP_BUF_SIZE];
    int n_quest = 0;    

    while (true)
    {
        n_quest++;

        printf ("Вопрос #%d: %s?\n (да/нет): ", n_quest, (char *) node->value);
        get_input (input);

        if (strcasecmp (input, "да") == 0)
        {
            node = node->left;
            if (node == nullptr)
            {
                printf (I "Лол, а сам не мог?\n" D);
                break;
            }
        }
        else if (strcasecmp (input, "нет") == 0)
        {
            if (node->right == nullptr)
            {
                add_unknown_object (tree, node);
                printf (I "Не ну ля такое не считается\n" D);
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
    assert (tree->head_node != nullptr && "invalid tree");

    char input[OBJ_SIZE+1] = "";

    printf (I "Ну, пирожок, что же ты хочешь узнать?\n" D);
    printf (I "Ваш жалкий объект: " D);
    get_input (input);

    if (tree::dfs_exec (tree, definition_pre, input, 
                                nullptr,        nullptr,
                                definition_post, input))
    {
        printf (I "Я такого не знаю, иди к тете Гале\n" D);
    }
    else
    {
        printf (I "\nЯ рад что ты хотя бы пытаешься что-то узнать\n" D);
    }
}

// ----------------------------------------------------------------------------

void run_wisetree (tree::tree_t *tree)
{
    assert (tree != nullptr && "invalid pointer");
    assert (tree->head_node != nullptr && "invalid tree");

    printf (WISE_TREE_ASCII);

    printf ("\n\n");

    printf ("%s                                                  \n", ANON[0]);
    printf ("%s                                                  \n", ANON[1]);
    printf ("%s                                                  \n", ANON[2]);
    printf ("%s                                                  \n", ANON[3]);
    printf ("%s                                                  \n", ANON[4]);
    printf ("%s                                                  \n", ANON[5]);
    printf ("%s       ###########################################\n", ANON[6]);
    printf ("%s       #     Выберите режим Мудрого Дерева       #\n", ANON[7]);
    printf ("%s       #                                         #\n", ANON[8]);
    printf ("%s       # 1) Интерактивный диалог с просветленным #\n", ANON[9]);
    printf ("%s       # 2) Получение справки в лицо             #\n", ANON[10]);
    printf ("%s       # 3) Различие между объектами             #\n", ANON[11]);
    printf ("%s       # 4) Графический дамп                     #\n", ANON[12]);
    printf ("%s       ###########################################\n", ANON[13]);
    printf ("%s                                                  \n", ANON[14]);
    printf ("%s                                                  \n", ANON[15]);
    printf ("%s                                                  \n", ANON[16]);
    printf ("%s                                                  \n", ANON[17]);
    printf ("%s                                                  \n", ANON[18]);
    printf ("%s                                                  \n", ANON[19]);
    printf ("%s                                                  \n", ANON[20]);

    char input[OBJ_SIZE+1] = "";

    while (true)
    {
        printf ("Выбранный режим: ");

        get_input (input);

        if (strcasecmp (input, "1") == 0)
        {
            guess_mode (tree);

            break;
        }
        else if (strcasecmp (input, "2") == 0)
        {
            definition_mode (tree);

            break;
        }
        else
        {
            printf (I "Че?\n" D);
        }
    }
}

// ----------------------------------------------------------------------------
// STATIC SECTION
// ----------------------------------------------------------------------------

static void add_unknown_object (tree::tree_t *tree, tree::node_t *bad_node)
{
    assert (tree     != nullptr && "invalid pointer");
    assert (bad_node != nullptr && "invalid pointer");

    char buf[OBJ_SIZE + 1];

    printf (I "Ну, гений мысли, и что же ты загадал?\n" D);
    get_input (buf);

    tree::node_t *good_node  = tree::new_node (buf, OBJ_SIZE);

    printf (I "Ох, дружок, а сформулировать чем это отличается от '%s' сможешь то?\nЭто ... " D, (char *) bad_node->value);
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
        printf ("Свойство: %s\n", (char *) node->value);
    }

    return true;
}
