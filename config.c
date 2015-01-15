/* needed for getline() and getdelim() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "config.h"

/* Duplicated in another file.
 * TODO Do something about it */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

FILE *config_file = NULL;
FILE *a_config_file = NULL;

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

struct a_config_item abilities[] = {
    {"double_shot", 3, 50},
    {"move", 3, 5},
    {"sniper", 4, 0},
};

const int config_count = ARRAY_SIZE(config);
const int a_config_count = ARRAY_SIZE(abilities);

char *read_line(FILE *stream);
char *read_delimited(FILE *stream, int delim);

/*
 * server.conf file format:
 * name1 value1
 * name2 value2
 * ...
 *
 * (Newline at end of file mandatory)
 */

/*
 * abilities.conf file format:
 * name1 type1 cooldown1 value1
 * name2 type2 cooldown2 value2
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

void a_read_config()
{
    /* TODO error checking */
    a_config_file = fopen(ABILITIES_CONFIG_FILENAME, "r");
    if (a_config_file != NULL)
    {
        for (int i = 0; i < a_config_count; i++)
        {
            char *name;
            char *value_s;
            config_value_t value1;
            config_value_t value2;

            /* read name */
            name = read_delimited(a_config_file, ' ');
            if (!name)
                break;

            /* read value */
            value_s = read_line(a_config_file);
            if (!value_s)
                break;
            /* TODO what if the number is huge?
             * Change value type to int16_t
             */
            if (sscanf(value_s, "%d %d", &value1, &value2) != 1)
                break;
            free(value_s);

            debug_s(0, "a_config: read name", name);
            debug_d(0, "a_config: read value1", value1);
            debug_d(0, "a_config: read value2", value2);
            a_config_set(name, value1, value2);

            free(name);
        }
        fclose(a_config_file);
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

void a_write_config()
{
    a_config_file = fopen(ABILITIES_CONFIG_FILENAME, "w");
    for (int i=0; i<a_config_count; i++)
        fprintf(a_config_file, "%s %d %d\n",
                abilities[i].name, abilities[i].cooldown, abilities[i].param);
    fclose(a_config_file);
}

config_value_t config_get(char *name)
{
    /* TODO Do something about the linear search. Use a hash table? */
    for (int i = 0; i < config_count; i++)
        if (strcmp(config[i].name, name) == 0)
            return config[i].value;
    return 0; /* If nothing found. Not the best way to show it */
}

struct a_config_item *a_config_get(char *name)
{
    /* TODO Do something about the linear search. Use a hash table? */
    for (int i = 0; i < a_config_count; i++)
        if (strcmp(abilities[i].name, name) == 0)
            return &abilities[i];
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

void a_config_set(char *name, config_value_t value1, config_value_t value2)
{
    /* find a config item whose name matches this name
     * and place the value there */
    for (int i = 0; i < a_config_count; i++)
    {
        if (strcmp(name, abilities[i].name) == 0)
        {
            abilities[i].cooldown = value1;
            abilities[i].param = value2;
            return;
        }
    }
    /* No such name found. Doing nothing about it for now. */
    debug_s(5, "config_set: No option found in config", name);
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

    ssize_t len = getdelim(&str, &buffer_len, delim, stream);

    if (len == -1) {
        free(str);
        return NULL;
    }

    /* remove delimiter at the end */
    if (str[len - 1] == delim)
        str[len - 1] = '\0';
    /* TODO shrink allocated size after this? */

    return str;
}
