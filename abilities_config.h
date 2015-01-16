#ifndef ABILITIES_CONFIG_H
#define ABILITIES_CONFIG_H

#include "config.h"

#define ABILITIES_CONFIG_FILENAME "abilities.conf"

typedef int16_t ability_id_t;


extern struct dyn_arr abilities;

void read_abilities(void);

ability_id_t new_ability_id(void);
struct ability *find_ability(client_id_t id);

#endif /* ABILITIES_CONFIG_H */
