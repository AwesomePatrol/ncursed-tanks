#ifndef SERVER_LIB_INCLUDES_H
#define SERVER_LIB_INCLUDES_H

#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "config.h"
#include "dyn_arr.h"
#include "debug.h"
#include "map_gen.h"
#include "shot.h"
#include "server_updates_queue.h"

#endif /* SERVER_LIB_INCLUDES_H */
