#ifndef SERVER_H
#define SERVER_H

#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
// For getting seed for random
#include <time.h>

#include "debug.h"
#include "map_gen.h"
 
#define DEBUG 0 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/


#define MAX_THREADS MAX_PLAYERS

/* TODO move to separate .h */

struct updates_queue
{
    struct updates_queue_elt *first;
    struct updates_queue_elt *last;
};

struct updates_queue_elt
{
    struct update cur;

    struct updates_queue_elt *next;
};

typedef struct updates_queue_elt uq_elt_t;

#endif /* SERVER_H */
