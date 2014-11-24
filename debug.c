#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "debug.h"

FILE *debug_file = NULL;

void debug_open(const char *filename) {
    debug_file = fopen(filename, "a");
}

void debug_do(int lvl, const char *name, void (*action)(void))
{
    if (DEBUG <= lvl) {
        time_t tmp = time(NULL);
        struct tm *t;
        t = localtime(&tmp);
        char date[100];
        strftime(date, 100, "%F %T", t);
        fprintf(debug_file, "%s [%s] ", date, name);

        action();

        fflush(debug_file);
    }
}

void debug_s(int lvl, const char *name, const char *str)
{
    void print(void)
    {
        fprintf(debug_file, "%s\n", str);
    }

    debug_do(lvl, name, print);
}

void debug_d(int lvl, const char *name, long long dec)
{
    void print(void)
    {
        fprintf(debug_file, "%lld\n", dec);
    }

    debug_do(lvl, name, print);
}

void debug_c(int lvl, const char *name, char ch)
{
    void print(void)
    {
        fprintf(debug_file, "%c\n", ch);
    }

    debug_do(lvl, name, print);
}
