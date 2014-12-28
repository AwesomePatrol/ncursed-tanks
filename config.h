#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

struct config_item
{
    char *name;
    int32_t value;
    int32_t min;
    int32_t max;
};

#define SERVER_CONFIG_FILENAME "server.conf"

extern struct config_item config[];

void read_config();
void write_config();

int config_get(char *name);

#endif /* CONFIG_H */
