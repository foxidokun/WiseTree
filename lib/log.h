#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdio.h>
#include <time.h>

#ifndef HTML_LOGS
    #define HTML_LOGS 1
#endif

#ifndef TEXT_LOGS
    #define TEXT_LOGS 0
#endif

enum class log
{
    DBG = 1,
    INF = 2,
    WRN = 3,
    ERR = 4,
};

const size_t __TIME_BUF_SIZE = 10;

#ifndef __LOG_CPP
extern enum log __LOG_LEVEL;
extern FILE *__LOG_OUT_STREAM;
#endif

#if HTML_LOGS
    #define R       "<font color=\"red\">"
    #define G       "<font color=\"green\">"
    #define Cyan    "<font color=\"cyan\">"
    #define Y       "<font color=\"yellow\">"
    #define D       "</font>"
    #define Bold    "<b>"
    #define Plain   "</b>"
#elif TEXT_LOGS
    #define R       "\033[91m"
    #define G       "\033[92m"
    #define Cyan    "\033[96m"
    #define Y       "\033[93m"
    #define D       "\033[39m"
    #define Bold    "\033[1m"
    #define Plain   "\033[0m"
#endif

static inline void current_time (char *buf, size_t buf_size);

/**
 * @brief      Write to log stream time, file&line and your formatted message
 *
 * @param      lvl   Log level
 * @param[in]  fmt   Format string
 * @param[in]  file  File where log is called  
 * @param      line  Line where log is called
 * @param      ...   printf parameters (format string & it's parameters)
 */
#ifndef DISABLE_LOGS

void _log (enum log lvl, const char *fmt, const char *file, unsigned int line, ...);

#define log(lvl, fmt, ...)                               \
{                                                        \
    _log (lvl, fmt, __FILE__, __LINE__, ##__VA_ARGS__);  \
}                                                       

#else

#define log(lvl, ...) {;}

#endif
/**
 * @brief      Sets the log level.
 *
 * @param[in]  level  The level
 */

void set_log_level (enum log level);

/**
 * @brief      Sets the output log stream.
 *
 * @param      stream  Stream
 */
void set_log_stream (FILE *stream);

FILE *get_log_stream ();

/**
 * @brief      Write current time in HH:MM:SS format to given buffer
 *
 * @param[out] buf       Output buffer
 * @param[in]  buf_size  Buffer size
 */
static inline void current_time (char *buf, size_t buf_size)
{
    assert (buf != nullptr && "pointer can't be null");
    assert (buf_size >= 9 && "Small buffer");

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime (buf, buf_size, "%H:%M:%S", timeinfo);
}
#endif //LOG_H