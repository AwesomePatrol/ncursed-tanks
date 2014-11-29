#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "config.h"

FILE *config_file = NULL;

struct config_item config[] = {
    {"players_number", 2, 1, 16},
    {"map_width", 128, 48, 1024},
    {"map_height", 64, 48, 512},
    {"tank_hp", 100, 1, 1000},
    {"dmg_radius", 4, 2, 16},
    {"dmg_cap", 50, 1, 1000},
};

void read_config()
{
    /* TODO error checking */
    config_file = fopen(SERVER_CONFIG_FILENAME, "r");
    if (config_file != NULL)
    {
        int n = sizeof(config)/sizeof(config[0]);
        for (int i=0; i<n; i++)
        {
            char buff[50], buff_int;
            fscanf(config_file, "%s %d", buff, &buff_int);
            //TODO sometimes reads negative value
            debug_s(1, "config: string read", buff);
            debug_d(1, "config: int read", buff_int);
            for (int j=0; j<n; j++)
            {
                int cmp = strcmp(buff, config[j].name);
                if (cmp == 0)
                {
                    config[j].value = buff_int;
                    break;
                }
            }
        }
        fclose(config_file);
    }
}


void write_config()
{
    config_file = fopen(SERVER_CONFIG_FILENAME, "w");
    int n = sizeof(config)/sizeof(config[0]);
    for (int i=0; i<n; i++)
        fprintf(config_file, "%s %d\n",
                config[i].name, config[i].value);
    fclose(config_file);
}
