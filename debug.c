#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "debug.h"

FILE *debug_file = NULL;

void debug_open(const char *filename) {
        debug_file = fopen(filename, "a");
}

void debug_s(int lvl, const char *name, const char *str)
{
    if (DEBUG <= lvl) {
        time_t tmp = time(NULL);
        struct tm *t;
        t = localtime(&tmp);
        char date[100];
        strftime(date, 100, "%T", t);
        fprintf(debug_file, "%s [%s] ", date, name);
        fprintf(debug_file, "%s\n", str);
        fflush(debug_file);
    }
}

void debug_d(int lvl, const char *name, int dec)
{
    if (DEBUG <= lvl) {
        time_t tmp = time(NULL);
        struct tm *t;
        t = localtime(&tmp);
        char date[100];
        strftime(date, 100, "%T", t);
        fprintf(debug_file, "%s [%s] ", date, name);
        fprintf(debug_file, "%d\n", dec);
        fflush(debug_file);
    }
}
