#include <stdio.h>
#include <stdlib.h>

#include "config.h"

FILE *config_file = NULL;
int config_min[] = {1, 48, 48, 1, 2, 1};
int config_values[] = {2, 128, 64, 100, 4, 50};
int config_max[] = {16, 1024, 512, 1000, 16, 1000};

void config_open(const char *filename) {
        config_file = fopen(filename, "a");
}
