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

#define SERVER_CONFIG_FILENAME "server.conf"

extern struct config_item config[];
extern const int config_count;

void read_config();
void write_config();

int config_get(char *name);
void config_set(char *name, config_value_t value);

#endif /* CONFIG_H */
