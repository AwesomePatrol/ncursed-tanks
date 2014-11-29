#ifndef CONFIG_H
#define CONFIG_H

struct config_item
{
    char *name;
    int value;
    int min;
    int max;
};

#define SERVER_CONFIG_FILENAME "server.conf"

extern struct config_item config[];
void read_config();
void write_config();

#endif /* CONFIG_H */
