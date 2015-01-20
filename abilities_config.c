#include "abilities_config.h"

FILE *a_config_file = NULL;

struct dyn_arr abilities = { sizeof(struct ability) };

/*
 * abilities.conf file format:
 * name1 type1 cooldown1 params_count1 param11 param12 ... 
 * name2 type2 cooldown2 params_count2 param21 param22 ... 
 * ...
 *
 * (For every line, a space before the newline is required)
 * (Newline at end of file mandatory)
 */

void read_abilities(void)
{
    /* TODO error checking */
    a_config_file = fopen(ABILITIES_CONFIG_FILENAME, "r");

    if (!a_config_file) return;

    while (true) /* exit with break when no more lines left */
    {
        struct ability a = {0};

        char *type_s;

        /* read name, type_s, cooldown_s */
        /* if can't read name, it means file ended */
        if (!(a.name = read_delimited(a_config_file, ' ')))
            break;
        debug_s(0, "abilities: read name", a.name);

        type_s = read_delimited(a_config_file, ' ');
        debug_s(0, "abilities: read type (string)", type_s);
        a.type = string_to_ability_type(type_s);
        free(type_s);

        a.cooldown = read_int_delimited(a_config_file, ' ');
        debug_d(0, "abilities: read cooldown", a.cooldown);

        a.params_count = read_int_delimited(a_config_file, ' ');
        debug_d(0, "abilities: read params_count", a.params_count);

        a.params = malloc(a.params_count * sizeof(*a.params));

        for (int i = 0; i < a.params_count; i++)
        {
            a.params[i] = read_int_delimited(a_config_file, ' ');
            debug_d(0, "abilities: read param", a.params[i]);
        }
        /* Get rid of the newline at the end of current line */
        /* TODO make this better */
        free(read_line(a_config_file));

        /* generate id after checking if name read (the check is above),
         * we don't need ids for abilities that won't be in the array */
        a.id = new_ability_id();

        dyn_arr_append(&abilities, &a);
    }

    fclose(a_config_file);
}

/*
void a_write_config()
{
    a_config_file = fopen(ABILITIES_CONFIG_FILENAME, "w");
    for (int i=0; i<a_config_count; i++)
        fprintf(a_config_file, "%s %d %d\n",
                abilities[i].name, abilities[i].cooldown, abilities[i].param);
    fclose(a_config_file);
}
*/

ability_id_t new_ability_id(void)
{
    static ability_id_t id_counter = 0;

    /* Assume id never overflows */
    return ++id_counter;
}

struct ability *find_ability(ability_id_t id)
{
    for (int i = 0; i < abilities.count; i++)
    {
        struct ability *a = dyn_arr_get(&abilities, i);

        if (id == a->id)
            return a;
    }
    return NULL;
}
