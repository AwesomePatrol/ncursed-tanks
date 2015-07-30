#include "debug.h"
#include "config.h"

/* Duplicated in another file.
 * TODO Do something about it */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

FILE *config_file = NULL;

struct config_item config[] = {
    {"map_width", 128, 48, 1024},
    {"map_height", 64, 48, 512},
    {"tank_hp", 100, 1, 1000},
    {"dmg_radius", 4, 2, 16},
    {"dmg_cap", 50, 1, 1000},
    {"gravity", 50, 1, 10000},
    {"wind", 0, -10000, 10000},
    {"power_c", 40, 20, 100},
    {"map_margin", 4, 2, 128},
    {"tank_distance", 10, 1, 128},
};

const int config_count = ARRAY_SIZE(config);

/*
 * server.conf file format:
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
        for (int i = 0; i < config_count; i++)
        {
            char *name;
            char *value_s;
            config_value_t value;

            /* read name */
            name = read_delimited(config_file, ' ');
            if (!name)
                break;

            /* read value */
            value_s = read_line(config_file);
            if (!value_s)
                break;
            /* TODO what if the number is huge?
             * Change value type to int16_t
             */
            if (sscanf(value_s, "%d", &value) != 1)
                break;
            free(value_s);

            debug_s(0, "config: read name", name);
            debug_d(0, "config: read value", value);
            config_set(name, value);

            free(name);
        }
        fclose(config_file);
    }
}

void write_config()
{
    config_file = fopen(SERVER_CONFIG_FILENAME, "w");
    for (int i=0; i<config_count; i++)
        fprintf(config_file, "%s %d\n",
                config[i].name, config[i].value);
    fclose(config_file);
}

config_value_t config_get(char *name)
{
    /* TODO Do something about the linear search. Use a hash table? */
    for (int i = 0; i < config_count; i++)
        if (strcmp(config[i].name, name) == 0)
            return config[i].value;
    return 0; /* If nothing found. Not the best way to show it */
}

void config_set(char *name, config_value_t value)
{
    /* find a config item whose name matches this name
     * and place the value there */
    for (int i = 0; i < config_count; i++)
    {
        if (strcmp(name, config[i].name) == 0)
        {
            config[i].value = value;
            return;
        }
    }
    /* No such name found. Doing nothing about it for now. */
    debug_s(5, "config_set: No option found in config", name);
}
