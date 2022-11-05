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

#define VOICE

// ----------------------------------------------------------------------------

const int N_LINES  = 32;
const int LINE_LEN = 200;
const int CMD_LEN  = 300;
const int PORT     = 3015;

#define I  "\033[3m"
#define DC "\033[0m"

static size_t utf8nlen(const char *str, size_t n);
static void braile_translate (const char *inp, char *out);

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
    const char (*ascii_art)[LINE_LEN] = nullptr;

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

    char buf[LINE_LEN*4] = "";

    for (unsigned int j = 0; j < screen->n_lines; ++i, ++j)
    {
        printf ("%s\t# " I "%s " DC, ascii_art[i], screen->lines[j]);

        cur_len = utf8nlen (screen->lines[j], strlen (screen->lines[j]));

        for (size_t pos = cur_len; pos < max_len; ++pos)
        {
            printf (" ");
        }
        printf ("#\t");

        braile_translate (screen->lines[j], buf);

        printf ("%s\n", buf);
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


    char cmd[CMD_LEN] = "";

    #ifdef VOICE
        for (int n = 0; n < screen->n_lines; ++n)
        {
            sprintf (cmd, "echo '%s' | RHVoice-test -r 4000 -t 2000 -p evgeny", screen->lines[n]);
            system (cmd);
        }
    #endif

    screen->n_lines = 0;
}

// ----------------------------------------------------------------------------

static size_t utf8nlen (const char *str, size_t n) {
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
        BR_LETTER ("а",  "⠁")
        BR_LETTER ("А",  "⠁")
        BR_LETTER ("б",  "⠃")
        BR_LETTER ("Б",  "⠃")
        BR_LETTER ("в",  "⠺")
        BR_LETTER ("В",  "⠺")
        BR_LETTER ("г",  "⠛")
        BR_LETTER ("Г",  "⠛")
        BR_LETTER ("д",  "⠙")
        BR_LETTER ("Д",  "⠙")
        BR_LETTER ("е",  "⠑")
        BR_LETTER ("Е",  "⠑")
        BR_LETTER ("ж",  "⠚")
        BR_LETTER ("Ж",  "⠚")
        BR_LETTER ("з",  "⠵")
        BR_LETTER ("З",  "⠵")
        BR_LETTER ("и",  "⠊")
        BR_LETTER ("И",  "⠊")
        BR_LETTER ("й",  "⠯")
        BR_LETTER ("Й",  "⠯")
        BR_LETTER ("к",  "⠅")
        BR_LETTER ("К",  "⠅")
        BR_LETTER ("л",  "⠇")
        BR_LETTER ("Л",  "⠇")
        BR_LETTER ("м",  "⠍")
        BR_LETTER ("М",  "⠍")
        BR_LETTER ("н",  "⠝")
        BR_LETTER ("Н",  "⠝")
        BR_LETTER ("о",  "⠕")
        BR_LETTER ("О",  "⠕")
        BR_LETTER ("п",  "⠏")
        BR_LETTER ("П",  "⠏")
        BR_LETTER ("р",  "⠗")
        BR_LETTER ("Р",  "⠗")
        BR_LETTER ("с",  "⠎")
        BR_LETTER ("С",  "⠎")
        BR_LETTER ("т",  "⠞")
        BR_LETTER ("Т",  "⠞")
        BR_LETTER ("у",  "⠥")
        BR_LETTER ("У",  "⠥")
        BR_LETTER ("ф",  "⠋")
        BR_LETTER ("Ф",  "⠋")
        BR_LETTER ("х",  "⠓")
        BR_LETTER ("Х",  "⠓")
        BR_LETTER ("ц",  "⠉")
        BR_LETTER ("Ц",  "⠉")
        BR_LETTER ("ч",  "⠟")
        BR_LETTER ("Ч",  "⠟")
        BR_LETTER ("ш",  "⠱")
        BR_LETTER ("Ш",  "⠱")
        BR_LETTER ("щ",  "⠭")
        BR_LETTER ("Щ",  "⠭")
        BR_LETTER ("ъ",  "⠷")
        BR_LETTER ("Ъ",  "⠷")
        BR_LETTER ("ы",  "⠮")
        BR_LETTER ("Ы",  "⠮")
        BR_LETTER ("ь",  "⠾")
        BR_LETTER ("Ь",  "⠾")
        BR_LETTER ("э",  "⠪")
        BR_LETTER ("Э",  "⠪")
        BR_LETTER ("ю",  "⠳")
        BR_LETTER ("Ю",  "⠳")
        BR_LETTER ("я",  "⠫")
        BR_LETTER ("Я",  "⠫")
        BR_LETTER (".",  "⠲")
        BR_LETTER (",",  "⠂")
        BR_LETTER ("-",  "⠤") 
        BR_LETTER ("(",  "⠣") 
        BR_LETTER (")",  "⠜") 
        BR_LETTER ("?",  "⠢") 
        BR_LETTER (":",  ":") 
        BR_LETTER ("#",  "#") 
        BR_LETTER ("/",  "/") 
        BR_LETTER ("0",  "⠚") 
        BR_LETTER ("1",  "⠁") 
        BR_LETTER ("2",  "⠃") 
        BR_LETTER ("3",  "⠉") 
        BR_LETTER ("4",  "⠙") 
        BR_LETTER ("5",  "⠑") 
        BR_LETTER ("6",  "⠋") 
        BR_LETTER ("7",  "⠛") 
        BR_LETTER ("8",  "⠓") 
        BR_LETTER ("9",  "⠊") 
        BR_LETTER (" ",  " ") 
        BR_LETTER ("\t", " ")
        /*else*/
        {
            fprintf (stderr, "Unexpected char in <%s> (%d)\n", inp, inp[0]);
            assert (0 && "Unexpected char");
        }  
    }
}
