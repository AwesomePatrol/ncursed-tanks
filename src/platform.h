#ifndef PLATFORM_H
#define PLATFORM_H

#ifndef PLATFORM_NO_GETOPT_LONG
#include <getopt.h>
#else /* use getopt instead of getopt_long */

#include <unistd.h>

#define getopt_long(ARGC, ARGV, OPTS, LONGOPTS, LONGINDEX)      \
    getopt(ARGC, ARGV, OPTS)

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

#define no_argument       0
#define required_argument 1
#define optional_argument 2

#endif /* PLATFORM_NO_GETOPT_LONG */

#endif /* PLATFORM_H */
