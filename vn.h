#ifndef VN_H
#define VN_H

#include <stdio.h>
#include <wchar.h>

const int MAX_LINES   = 64;
const int CONSOLE_LEN = 60;
const int LINE_LEN    = 200;

enum class render_mode_t
{
    MIKU,
    ANON
};

struct screen_t
{
    unsigned int n_text_lines;
    unsigned int n_speak_lines;
    wchar_t text_lines [MAX_LINES][LINE_LEN];
    wchar_t speak_lines[MAX_LINES][LINE_LEN];
    FILE *stream;
};

void screen_ctor (screen_t *screen, FILE *stream);

void render (screen_t *screen, render_mode_t mode);

void put_line (screen_t *screen, const wchar_t *fmt, ...);
void put_text (screen_t *screen, const wchar_t *fmt, ...);
void newline  (screen_t *screen);

void speak (const wchar_t *phrase);
void put_speak_line (screen_t *screen, const wchar_t *fmt, ...);
void put_speak_text (screen_t *screen, const wchar_t *fmt, ...);

#endif
