#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
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

const int N_LINES     = 32;
const int LINE_LEN    = 200;
const int CMD_LEN     = 400;
const int CONSOLE_LEN = 60;

const char SEP_LINE[3*(LINE_LEN+4)] = "═══════════════════════════════════════════════════════════════════════════════════";

#define I  "\033[3m"
#define DC "\033[0m"

static void render_text_lines (const screen_t *screen, const char (*ascii_art)[LINE_LEN],
                                                        unsigned int i, size_t max_len);
static void tts_run (const screen_t *screen);

static size_t utf8len (const char *str);
static size_t max_utf8len (const char lines[][LINE_BYTE_SIZE], unsigned int n_lines);
static void utf8cat (char *dest, const char *src, size_t n);
static size_t utf8_get_n_symbols_size (const char *str, size_t n);
static unsigned char get_utf8_char_width (const char *str);

static void braile_translate (const char *inp, char *out);

static size_t count_free_len (const char *const dest, const char *const src);
static void put_text_general (char lines[][LINE_BYTE_SIZE], unsigned int *index, const char *buf);

static void clear_console (const screen_t *screen);
static void clear_lines   (screen_t *screen);

#define PRINT(fmt, ...) fprintf (screen->stream, fmt, ##__VA_ARGS__)

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

void put_line (screen_t *screen, const char *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    char buf[LINE_LEN] = "";
    va_list args;
    va_start (args, fmt);

    vsprintf (buf, fmt, args);
    strcat (screen->text_lines[screen->n_text_lines], buf);
    screen->n_text_lines++;

    va_end (args);
}

void put_text (screen_t *screen, const char *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    char buf[LINE_LEN] = "";
    va_list args;
    va_start (args, fmt);

    vsprintf (buf, fmt, args);

    put_text_general (screen->text_lines, &screen->n_text_lines, buf);

    va_end (args);
}

// ----------------------------------------------------------------------------

void speak (const char *phrase)
{
    char cmd[CMD_LEN] = "";

    sprintf (cmd, "echo '%s' | festival --tts &>console.log", phrase);
    system (cmd);
}

void put_speak_line (screen_t *screen, const char *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    char buf[LINE_LEN] = "";
    va_list args;
    va_start (args, fmt);

    vsprintf (buf, fmt, args);
    strcat (screen->text_lines[screen->n_text_lines], buf);
    strcat (screen->speak_lines[screen->n_speak_lines], buf);
    screen->n_text_lines++;
    screen->n_speak_lines++;

    va_end (args);
}

void put_speak_text (screen_t *screen, const char *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    char buf[LINE_LEN] = "";
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);

    put_text_general (screen->text_lines,  &screen->n_text_lines,  buf);
    put_text_general (screen->speak_lines, &screen->n_speak_lines, buf);
    
    va_end (args);
}

// ----------------------------------------------------------------------------

void render (screen_t *screen, render_mode_t mode)
{
    assert (screen != nullptr && "pointer can't be null");

    unsigned int text_line_beg = (N_LINES - screen->n_text_lines) / 2;
    unsigned int i = 0;
    const char (*ascii_art)[LINE_LEN] = nullptr;

    // Set ascii art
    if      (mode == render_mode_t::ANON)  ascii_art = ANON;
    else if (mode == render_mode_t::MIKU)  ascii_art = MIKU;
    else    assert (0 && "unexpected mode");

    clear_console (screen);

    // Begin art block
    for (; i < text_line_beg - 1; ++i) PRINT ("%s\n", ascii_art[i]);
    //

    // Top # block
    PRINT ("%s\t╔%.*s╗\n", ascii_art[i],  3*((int)CONSOLE_LEN+2), SEP_LINE);
    i++;

    // Text block
    render_text_lines (screen, ascii_art, i, CONSOLE_LEN);
    i += screen->n_text_lines;

    // Bottom # block
    PRINT ("%s\t╚%.*s╝\n", ascii_art[i], 3*((int)CONSOLE_LEN+2), SEP_LINE);
    i++;

    //Continue art block
    for (; i < N_LINES; ++i)  PRINT ("%s\n", ascii_art[i]);

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

    char buf[LINE_LEN*4] = "";
    size_t byte_size = 0;
    size_t len       = 0;

    for (unsigned int j = 0; j < screen->n_text_lines; ++i, ++j)
    {
        byte_size = strlen  (screen->text_lines[j]);
        len       = utf8len (screen->text_lines[j]);

        braile_translate (screen->text_lines[j], buf);
        PRINT ("%s\t║ " I "%-*s ║\t%s\n" DC, ascii_art[i], (int) (byte_size + max_len - len),
                                                                screen->text_lines[j], buf);
    }
}

// ----------------------------------------------------------------------------

static void put_text_general (char lines[][LINE_BYTE_SIZE], unsigned int *index, const char *buf)
{
    assert (lines != nullptr && "invalid pointer");
    assert (index != nullptr && "invalid pointer");
    assert (buf   != nullptr && "invalid pointer");

    ssize_t buf_len  = utf8len (buf);
    size_t buf_index = 0;

    while (buf_len > 0)
    {
        size_t screen_free_len = count_free_len (lines[*index], buf);

        utf8cat (lines[*index], buf + buf_index, screen_free_len);

        if (buf_len > screen_free_len)
        {
            (*index)++;
        }

        buf_len   -= screen_free_len;
        buf_index += utf8_get_n_symbols_size (buf, screen_free_len);
    }
}

// ----------------------------------------------------------------------------

static void tts_run (const screen_t *screen)
{
    assert (screen != nullptr && "invalid pointer");

    for (unsigned int n = 0; n < screen->n_speak_lines; ++n)
    {
        speak (screen->speak_lines[n]);
    }
}

// ----------------------------------------------------------------------------

static size_t utf8len (const char *str) {
    assert (str != nullptr && "invalid pointer");

    size_t length = 0;

    while (*str != '\0') {
        str += get_utf8_char_width (str);
        length++;
    }

    return length;
}

// ----------------------------------------------------------------------------

#define COPY_TYPE(type)                     \
{                                           \
    *(type *) dest = *(const type *) src;   \
    dest += sizeof (type);                  \
    src  += sizeof (type);                  \
}

static void utf8cat (char *dest, const char *src, size_t n) {
    assert (dest != nullptr && "invalid pointer");
    assert (src  != nullptr && "invalid pointer");

    while (*dest != '\0')
    {
        dest++;
    }

    unsigned char char_width = 0;

    while (n > 0 && *src != '\0') {
        char_width = get_utf8_char_width (src);
        while (char_width > 0)
        {
            *(dest++) = *(src++);
            char_width--;
        }

        n--;
    }

    dest[0] = '\0';
}

// ----------------------------------------------------------------------------

static size_t utf8_get_n_symbols_size (const char *str, size_t n)
{
    assert (str != nullptr && "invalid pointer");

    size_t byte_size = 0;

    while (n > 0)
    {       
        if (0xf0 == (0xf8 & *str)) {
            /* 4-byte utf8 code point (began with 0b11110xxx) */
            byte_size += 4;
        } else if (0xe0 == (0xf0 & *str)) {
            /* 3-byte utf8 code point (began with 0b1110xxxx) */
            byte_size += 3;
        } else if (0xc0 == (0xe0 & *str)) {
            /* 2-byte utf8 code point (began with 0b110xxxxx) */
            byte_size += 2;
        } else { /* if (0x00 == (0x80 & *s)) { */
            /* 1-byte ascii (began with 0b0xxxxxxx) */
            byte_size += 1;            
        }

        n--;
    }

    return byte_size - 1; // delete '\0' character
}

// ----------------------------------------------------------------------------

static size_t max_utf8len (const char lines[][LINE_BYTE_SIZE], unsigned int n_lines)
{
    assert (lines != nullptr && "pointer can't be null");

    size_t max_len = 0;
    size_t cur_len = 0;

    for (unsigned int n = 0; n < n_lines; ++n)
    {
        cur_len = utf8len (lines[n]);
        if (cur_len > max_len)
        {
            max_len = cur_len;
        }
    }

    return max_len;
}

// ----------------------------------------------------------------------------

static size_t count_free_len (const char *const dest, const char *const src)
{
    assert (dest != nullptr && "invalid pointer");
    assert (src  != nullptr && "invalid pointer");

    size_t max_possible_len  = CONSOLE_LEN - utf8len (dest);

    size_t current_len     = 0;
    size_t char_width      = 0;
    size_t last_whitespace = 0;

    while (*src != '\0')
    {
        char_width = get_utf8_char_width (src);

        if (char_width == 1 && *src == ' ')
        {
            last_whitespace = current_len;
        }

        current_len += char_width;

        if (current_len > max_possible_len)
        {
            return last_whitespace;
        }
    }

    return last_whitespace;
}

// ----------------------------------------------------------------------------

#define BR_LETTER(symb, br)                         \
if (strncmp (inp, symb, sizeof (symb) - 1) == 0)    \
{                                                   \
    strcat (out, br);                               \
    inp += sizeof (symb) - 1;                       \
}                                                   \
else        

static void braile_translate (const char *inp, char *out)
{
    assert (inp != nullptr && "invalid pointer");
    assert (out != nullptr && "invalid pointer");

    out[0] = '\0';

    while (*inp != '\0')
    {
        #include  "braile_alphabet.h"
        /*else*/
        {
            fprintf (stderr, "Unexpected char in <%s> (%d)\n", inp, inp[0]);
            assert (0 && "Unexpected char");
        }  
    }
}

// ----------------------------------------------------------------------------

static void clear_console (const screen_t *screen)
{
    assert (screen != nullptr && "pointer can't be null");

    PRINT ("\033[2J\033[1;1H");
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

// ----------------------------------------------------------------------------

static unsigned char get_utf8_char_width (const char *str)
{
    assert (str != nullptr && "invalid pointer");

    if (0xf0 == (0xf8 & *str)) {
        /* 4-byte utf8 code point (began with 0b11110xxx) */
        return 4;
    } else if (0xe0 == (0xf0 & *str)) {
        /* 3-byte utf8 code point (began with 0b1110xxxx) */
        return 3;
    } else if (0xc0 == (0xe0 & *str)) {
        /* 2-byte utf8 code point (began with 0b110xxxxx) */
        return 2;
    } else { /* if (0x00 == (0x80 & *s)) { */
        /* 1-byte ascii (began with 0b0xxxxxxx) */
        return 1;            
    }
}
