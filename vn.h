#ifndef VN_H
#define VN_H

#include <stdio.h>

const int LINE_BYTE_SIZE = 256;
const int MAX_LINES      = 64;

enum class render_mode_t
{
    MIKU,
    ANON
};

struct screen_t
{
    unsigned int n_text_lines;
    unsigned int n_speak_lines;
    char text_lines[MAX_LINES][LINE_BYTE_SIZE];
    char speak_lines[MAX_LINES][LINE_BYTE_SIZE];
    FILE *stream;
};

void screen_ctor (screen_t *screen, FILE *stream);

void render (screen_t *screen, render_mode_t mode);

void put_line (screen_t *screen, const char *fmt, ...);
void put_text (screen_t *screen, const char *fmt, ...);
void newline  (screen_t *screen);

void speak (const char *phrase);
void put_speak_line (screen_t *screen, const char *fmt, ...);
void put_speak_text (screen_t *screen, const char *fmt, ...);

#endif
