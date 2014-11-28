#include <stdio.h>
#include <stdlib.h>

#include "config.h"

FILE *config_file = NULL;
int config_int[] = {1, 48, 48, 1, 2, 1};

void config_open(const char *filename) {
        config_file = fopen(filename, "a");
}
