#define _POSIX_C_SOURCE 200809L

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
    {"gravity", 5, 1, 10},
    {"power_c", 40, 20, 100},
    {"map_margin", 4, 2, 128},
    {"tank_distance", 10, 1, 128},
};

char *read_line(FILE *stream);
char *read_delimited(FILE *stream, int delim);

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
            char *name;
            char *value_s;
            int value;

            /* read name */
            name = read_delimited(config_file, ' ');
            if (!name)
                break;

            /* read value */
            value_s = read_line(config_file);
            if (!value_s)
                break;
            if (sscanf(value_s, "%d", &value) != 1)
                break;
            free(value_s);

            debug_s(1, "config: read name", name);
            debug_d(1, "config: read value", value);
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

            free(name);
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

/* Reads a whole line from stream, with trailing newline if it's present.
 * Returns NULL on {error or EOF} */
char *read_line(FILE *stream)
{
    char *line = NULL;
    size_t buffer_len = 0;

    ssize_t len = getline(&line, &buffer_len, stream);

    if (len == -1) {
        free(line);
        return NULL;
    } else {
        return line;
    }
}

/* Reads string with delimiter, removing delimiter from the end.
 * Returns NULL on {error or EOF} */
char *read_delimited(FILE *stream, int delim)
{
    char *str = NULL;
    size_t buffer_len = 0;

    ssize_t len = getline(&str, &buffer_len, stream);

    if (len == -1) {
        free(str);
        return NULL;
    }

    /* remove delimiter at the end */
    if (str[len] == delim)
        str[len] = '\0';

    return str;
}
