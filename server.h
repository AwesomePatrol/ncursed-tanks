#ifndef SERVER_H
#define SERVER_H

#define _POSIX_C_SOURCE 1

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
// For getting seed for random
#include <time.h>

#include "debug.h"
#include "map_gen.h"
 
#define DEBUG 0 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/

#endif /* SERVER_H */
