#ifndef DEBUG_H
#define DEBUG_H

#include <errno.h>

#ifdef _DEBUG
#   define DEBUG 0
#else
#   define DEBUG 5 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/
#endif

void debug_open(const char *filename);

void debug_s(int lvl, const char *name, const char *str);
void debug_d(int lvl, const char *name, long long dec);
void debug_f(int lvl, const char *name, double num);
void debug_x(int lvl, const char *name, long long hex);
void debug_c(int lvl, const char *name, char ch);

void debug_errno(const char *name);

#endif /* DEBUG_H */
