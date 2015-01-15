#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef int32_t config_value_t;

struct config_item
{
    char *name;
    config_value_t value;
    config_value_t min;
    config_value_t max;
};

struct a_config_item
{
    char *name;
    config_value_t cooldown;
    config_value_t param;
};

#define SERVER_CONFIG_FILENAME "server.conf"
#define ABILITIES_CONFIG_FILENAME "abilities.conf"

extern struct config_item config[];
extern const int config_count;

void read_config();
void write_config();

int config_get(char *name);
void config_set(char *name, config_value_t value);

extern struct a_config_item abilities[];
extern const int a_config_count;

void a_read_config();
void a_write_config();

struct a_config_item *a_config_get(char *name);
void a_config_set(char *name, config_value_t value1, config_value_t value2);

#endif /* CONFIG_H */
