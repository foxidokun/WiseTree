#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <stdarg.h>

#include "lib/log.h"
#include "vn.h"
#include "ascii_arts.h"

// ----------------------------------------------------------------------------
// CONFIG OPTIONS
// ----------------------------------------------------------------------------

// #define VOICE

// ----------------------------------------------------------------------------
// CONST & HEADER
// ----------------------------------------------------------------------------

const int N_ART_LINES = 32;
const int CMD_LEN     = 400;

const wchar_t SEP_LINE  [CONSOLE_LEN+4] = L"═══════════════════════════════════════════════════════════════";
const wchar_t SHADE_LINE[CONSOLE_LEN+4] = L"▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓";

#define I  "\033[3m"
#define DC "\033[0m"

static void render_text_lines (const screen_t *screen, const char (*ascii_art)[LINE_LEN],
                                                        unsigned int i, size_t max_len);
static void tts_run (const screen_t *screen);

static void braile_translate (const wchar_t *inp, wchar_t *out);

static size_t count_free_len (const wchar_t *const dest, const wchar_t *const src);
static void put_text_general (wchar_t lines[][LINE_LEN], unsigned int *index, const wchar_t *buf);

static void clear_console (const screen_t *screen);
static void clear_lines   (screen_t *screen);

#define PRINT(fmt, ...) fwprintf (screen->stream, fmt, ##__VA_ARGS__)

// ----------------------------------------------------------------------------
// PUBLIC
// ----------------------------------------------------------------------------

void screen_ctor (screen_t *screen, FILE *stream)
{
    assert (screen != nullptr && "invalid pointer");
    assert (stream != nullptr && "invalid pointer");

    screen->n_text_lines  = 0;
    screen->n_speak_lines = 0;
    screen->stream        = stream;
}

// ----------------------------------------------------------------------------

void put_line (screen_t *screen, const wchar_t *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    wchar_t buf[LINE_LEN] = L"";
    va_list args;
    va_start (args, fmt);

    vswprintf (buf, LINE_LEN, fmt, args);
    put_text_general (screen->text_lines,  &screen->n_text_lines,  buf);
    screen->n_text_lines++;

    va_end (args);
}

void put_text (screen_t *screen, const wchar_t *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    wchar_t buf[LINE_LEN] = L"";
    va_list args;
    va_start (args, fmt);

    vswprintf (buf, LINE_LEN, fmt, args);

    put_text_general (screen->text_lines, &screen->n_text_lines, buf);

    va_end (args);
}

// ----------------------------------------------------------------------------

void speak (const wchar_t *phrase)
{
    char cmd[CMD_LEN] = "";

    sprintf (cmd, "echo %ls | festival --tts &>console.log", phrase);
    system (cmd);
}

void put_speak_line (screen_t *screen, const wchar_t *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    wchar_t buf[LINE_LEN] = L"";
    va_list args;
    va_start (args, fmt);

    vswprintf (buf, LINE_LEN, fmt, args);
    put_text_general (screen->text_lines,  &screen->n_text_lines,  buf);
    put_text_general (screen->speak_lines, &screen->n_speak_lines, buf);
    screen->n_text_lines++;
    screen->n_speak_lines++;
    va_end (args);
}

void put_speak_text (screen_t *screen, const wchar_t *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    wchar_t buf[LINE_LEN] = L"";
    va_list args;
    va_start (args, fmt);
    vswprintf (buf, LINE_LEN, fmt, args);

    put_text_general (screen->text_lines,  &screen->n_text_lines,  buf);
    put_text_general (screen->speak_lines, &screen->n_speak_lines, buf);
    
    va_end (args);
}

// ----------------------------------------------------------------------------

void render (screen_t *screen, render_mode_t mode)
{
    assert (screen != nullptr && "pointer can't be null");

    unsigned int text_line_beg = (N_ART_LINES - screen->n_text_lines) / 2;
    unsigned int i = 0;
    const char (*ascii_art)[LINE_LEN] = nullptr;

    // Set ascii art
    if      (mode == render_mode_t::ANON)  ascii_art = ANON;
    else if (mode == render_mode_t::MIKU)  ascii_art = MIKU;
    else    assert (0 && "unexpected mode");

    clear_console (screen);

    // Begin art block
    for (; i < text_line_beg - 1; ++i) PRINT (L"%s\n", ascii_art[i]);
    //

    // Top # block
    PRINT (L"%s\t╔%.*ls╗\n", ascii_art[i], CONSOLE_LEN+2, SEP_LINE);
    i++;

    // Text block
    render_text_lines (screen, ascii_art, i, CONSOLE_LEN);
    i += screen->n_text_lines;

    // Bottom # block
    PRINT (L"%s\t╚%.*ls╝▓\n", ascii_art[i], CONSOLE_LEN+2, SEP_LINE);
    PRINT (L"%s\t %.*ls\n",   ascii_art[i], CONSOLE_LEN+4, SHADE_LINE);
    i+=2;

    //Continue art block
    for (; i < N_ART_LINES; ++i)  PRINT (L"%s\n", ascii_art[i]);

    tts_run (screen);

    clear_lines (screen);
}

// ----------------------------------------------------------------------------
// STATIC SECTION
// ----------------------------------------------------------------------------

static void render_text_lines (const screen_t *screen, const char (*ascii_art)[LINE_LEN],
                                                        unsigned int i, size_t max_len) 
{
    assert (screen    != nullptr && "invalid pointer");
    assert (ascii_art != nullptr && "invalid pointer");

    wchar_t buf[LINE_LEN*4] = L"";

    for (unsigned int j = 0; j < screen->n_text_lines; ++i, ++j)
    {
        braile_translate (screen->text_lines[j], buf);
        PRINT (L"%s\t║ " I "%-*ls ║▓\t%ls\n" DC, ascii_art[i], (int)max_len,
                                                                screen->text_lines[j], buf);
    }
}

// ----------------------------------------------------------------------------

static void put_text_general (wchar_t lines[][LINE_LEN], unsigned int *index, const wchar_t *buf)
{
    assert (lines != nullptr && "invalid pointer");
    assert (index != nullptr && "invalid pointer");
    assert (buf   != nullptr && "invalid pointer");

    ssize_t buf_len  = (ssize_t) wcslen (buf);

    while (buf_len > 0)
    {
        size_t screen_free_len = count_free_len (lines[*index], buf);

        assert (screen_free_len < CONSOLE_LEN);
        assert (screen_free_len != 0);

        wcsncat (lines[*index], buf, screen_free_len);

        if ((size_t) buf_len > screen_free_len)
        {
            (*index)++;
        }

        buf_len -= (ssize_t) screen_free_len;
        buf     += screen_free_len;
    }
}

// ----------------------------------------------------------------------------

static void tts_run (const screen_t *screen)
{
    assert (screen != nullptr && "invalid pointer");

    #ifdef VOICE
        for (unsigned int n = 0; n < screen->n_speak_lines; ++n)
        {
            speak (screen->speak_lines[n]);
        }
    #endif
}

// ----------------------------------------------------------------------------

static size_t count_free_len (const wchar_t *const dest, const wchar_t *src)
{
    assert (dest != nullptr && "invalid pointer");
    assert (src  != nullptr && "invalid pointer");

    size_t max_possible_len = CONSOLE_LEN - wcslen (dest);

    size_t current_len     = 0;
    size_t last_whitespace = 0;

    while (*src != L'\0')
    {
        current_len++;

        if (iswspace((wint_t) *src))
        {
            last_whitespace = current_len;
        }

        src++;

        if (current_len > max_possible_len)
        {
            return last_whitespace;
        }
    }

    return current_len;
}

// ----------------------------------------------------------------------------

#define BR_LETTER(symb, br)                         \
if (*inp == symb)                                   \
{                                                   \
    *out = br;                                      \
    inp++;                                          \
    out++;                                          \
}                                                   \
else        

static void braile_translate (const wchar_t *inp, wchar_t *out)
{
    assert (inp != nullptr && "invalid pointer");
    assert (out != nullptr && "invalid pointer");

    while (*inp != '\0')
    {
        #include  "braile_alphabet.h"
        /*else*/
        {
            fprintf (stderr, "Unexpected char in <%ls>\n", inp);
            assert (0 && "Unexpected char");
        }  
    }

    out[0] = '\0';
}

// ----------------------------------------------------------------------------

static void clear_console (const screen_t *screen)
{
    assert (screen != nullptr && "pointer can't be null");

    PRINT (L"\033[2J\033[1;1H");
}

// ----------------------------------------------------------------------------

static void clear_lines (screen_t *screen)
{
    assert (screen != nullptr && "pointer can't be null");

    for (unsigned int n = 0; n < screen->n_text_lines; ++n)
    {
        screen->text_lines[n][0] = '\0';
    }

    for (unsigned int n = 0; n < screen->n_speak_lines; ++n)
    {
        screen->speak_lines[n][0] = '\0';
    }

    screen->n_text_lines  = 0;
    screen->n_speak_lines = 0;
}
