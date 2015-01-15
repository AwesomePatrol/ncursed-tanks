#ifndef ABILITIES_CONFIG_H
#define ABILITIES_CONFIG_H

#include <stdint.h>

typedef int32_t config_value_t;

struct a_config_item
{
    char *name;
    config_value_t cooldown;
    config_value_t param;
};

#define ABILITIES_CONFIG_FILENAME "abilities.conf"

extern struct a_config_item abilities[];
extern const int a_config_count;

void a_read_config();
void a_write_config();

struct a_config_item *a_config_get(char *name);
void a_config_set(char *name, config_value_t value1, config_value_t value2);

#endif /* ABILITIES_CONFIG_H */
