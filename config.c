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

/*
 * Config file format:
 * name1 value1
 * name2 value2
 * ...
 *
 * (Newline at end of file mandatory)
 */

void read_config()
{
    /* TODO error checking */
    config_file = fopen(SERVER_CONFIG_FILENAME, "r");
    if (config_file != NULL)
    {
        int n = sizeof(config)/sizeof(config[0]);
        for (int i = 0; i < n; i++)
        {
            char buff[50], buff_int;

            char *name = NULL;
            int name_size = 0;
            int name_len;
            char *value_s = NULL;
            int value_s_size = 0;
            int value;

            /* read name */
            /* getline() and getdelim()
             * return -1 in case of {error or EOF} */
            if ((name_len = getdelim(&name, &name_size, ' ', config_file))
                == -1)
            {
                free(name);
                break;
            }
            /* remove ' ' at the end */
            name[name_len] = '\0';

            /* read value */
            if (getline(&value_s, &value_s_size, config_file)
                == -1)
            {
                free(value_s);
                break;
            }
            if (sscanf(value_s, "%d", &value) != 1)
                break;

            debug_s(1, "config: string read", buff);
            debug_d(1, "config: int read", buff_int);
            /* find a config item whose name matches current
             * and place the value there */
            for (int j=0; j<n; j++)
            {
                int cmp = strcmp(name, config[j].name);
                if (cmp == 0)
                {
                    config[j].value = value;
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
