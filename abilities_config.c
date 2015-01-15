/* needed for getline() and getdelim() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "abilities_config.h"

/* Duplicated in another file.
 * TODO Do something about it */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

FILE *a_config_file = NULL;

struct a_config_item abilities[] = {
    {"double_shot", 3, 50},
    {"move", 3, 5},
    {"sniper", 4, 0},
};

const int a_config_count = ARRAY_SIZE(abilities);


char *read_line(FILE *stream);
char *read_delimited(FILE *stream, int delim);

/*
 * Config file format:
 * name1 value1_1 value1_2
 * name2 value2_1 value2_2
 * ...
 *
 * (Newline at end of file mandatory)
 */

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

void a_write_config()
{
    a_config_file = fopen(ABILITIES_CONFIG_FILENAME, "w");
    for (int i=0; i<a_config_count; i++)
        fprintf(a_config_file, "%s %d %d\n",
                abilities[i].name, abilities[i].cooldown, abilities[i].param);
    fclose(a_config_file);
}

struct a_config_item *a_config_get(char *name)
{
    /* TODO Do something about the linear search. Use a hash table? */
    for (int i = 0; i < a_config_count; i++)
        if (strcmp(abilities[i].name, name) == 0)
            return &abilities[i];
    return 0; /* If nothing found. Not the best way to show it */
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
