#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>

#include "vn.h"
#include "ascii_arts.h"

const int N_LINES  = 32;
const int LINE_LEN = 300;

#define I "\033[3m"
#define D "\033[0m"

size_t utf8nlen(const char *str, size_t n) {
  const char *t = str;
  size_t length = 0;

  while ((size_t)(str - t) < n && '\0' != *str) {
    if (0xf0 == (0xf8 & *str)) {
      /* 4-byte utf8 code point (began with 0b11110xxx) */
      str += 4;
    } else if (0xe0 == (0xf0 & *str)) {
      /* 3-byte utf8 code point (began with 0b1110xxxx) */
      str += 3;
    } else if (0xc0 == (0xe0 & *str)) {
      /* 2-byte utf8 code point (began with 0b110xxxxx) */
      str += 2;
    } else { /* if (0x00 == (0x80 & *s)) { */
      /* 1-byte ascii (began with 0b0xxxxxxx) */
      str += 1;
    }

    /* no matter the bytes we marched s forward by, it was
     * only 1 utf8 codepoint */
    length++;
  }

  if ((size_t)(str - t) > n) {
    length--;
  }
  return length;
}

// ----------------------------------------------------------------------------

void screen_ctor (screen_t *screen, FILE *stream)
{
    assert (screen != nullptr && "invalid pointer");

    screen->n_lines = 0;
    screen->stream  = stream;
}

// ----------------------------------------------------------------------------

void put_line (screen_t *screen, const char *fmt, ...)
{
    assert (screen && "invalid pointer");
    assert (fmt    && "invalid pointer");

    va_list args;
    va_start (args, fmt);

    vsprintf (screen->lines[screen->n_lines], fmt, args);
    screen->n_lines++;

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

    va_end (args);

    strcat (screen->lines[screen->n_lines], buf);
}

// ----------------------------------------------------------------------------

void render (screen_t *screen, render_mode_t mode)
{
    assert (screen != nullptr && "pointer can't be null");

    unsigned int text_line_beg = (N_LINES - screen->n_lines) / 2;
    unsigned int i = 0;
    const char (*ascii_art)[300] = nullptr;

    if      (mode == render_mode_t::ANON)  ascii_art = ANON;
    else if (mode == render_mode_t::MIKU)  ascii_art = MIKU;
    else     assert (0 && "invalid mode");

    size_t max_len = 0;
    size_t cur_len = 0;

    for (unsigned int n = 0; n < screen->n_lines; ++n)
    {
        cur_len = utf8nlen (screen->lines[n], strlen (screen->lines[n]));
        if (cur_len > max_len)
        {
            max_len = cur_len;
        }
    }

    printf ("\033[2J\033[1;1H");

    for (; i < text_line_beg - 1; ++i) 
    {
        printf ("%s\n", ascii_art[i]);
    }

    printf ("%s\t", ascii_art[i]);
    i++;

    for (size_t n = 0; n < max_len+4; ++n)
    {
        printf ("#");
    }
    printf ("\n");

    char cmd[300] = "";

    for (unsigned int j = 0; j < screen->n_lines; ++i, ++j)
    {
        printf ("%s\t# " I "%s " D, ascii_art[i], screen->lines[j]);

        cur_len = utf8nlen (screen->lines[j], strlen (screen->lines[j]));

        for (size_t pos = cur_len; pos < max_len; ++pos)
        {
            printf (" ");
        }
        printf ("#\n");
    }

    printf ("%s\t", ascii_art[i]);
    i++;

    for (size_t n = 0; n < max_len + 4; ++n)
    {
        printf ("#");
    }
    
    printf ("\n");

    for (; i < N_LINES; ++i)
    {
        printf ("%s\n", ascii_art[i]);
    }

    for (int n = 0; n < screen->n_lines; ++n)
    {
        sprintf (cmd, "echo '%s' | RHVoice-test -r 4000 -t 2000 -p evgeny", screen->lines[n]);
        system (cmd);
    }

    screen->n_lines = 0;
}
