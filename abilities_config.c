FILE *a_config_file = NULL;

struct dyn_arr abilities = { sizeof(struct ability) };

void read_abilities(void)
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
