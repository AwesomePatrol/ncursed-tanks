#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 0 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/

void debug_open(const char *filename);
void debug_s(int lvl, const char *name, const char *str);
void debug_d(int lvl, const char *name, long long dec);
void debug_c(int lvl, const char *name, char ch);

#endif /* DEBUG_H */
