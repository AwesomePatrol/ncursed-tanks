#include <stdio.h>
#include <stdlib.h>

#include "config.h"

FILE *config_file = NULL;
int config_min[] = {1, 48, 48, 1, 2, 1};
int config_values[] = {2, 128, 64, 100, 4, 50};
int config_max[] = {16, 1024, 512, 1000, 16, 1000};

const char *config_names[] = {
    "players_number",
    "map_width",
    "map_height",
    "tank_hp",
    "dmg_radius",
    "dmg_cap"
};

void write_config()
{
    config_file = fopen("server.conf", "w");
    int n = sizeof(config_values)/sizeof(config_values[0]);
    for (int i=0; i<n; i++)
        fprintf(config_file, "%s %d\n",
                config_names[i], config_values[i]);
    fclose(config_file);
}
