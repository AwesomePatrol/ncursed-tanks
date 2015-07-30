#ifndef ABILITIES_CONFIG_H
#define ABILITIES_CONFIG_H

/* for struct ability, AbilityType. Move them to other file if needed. */
#include "common.h"
#include "server_abilities.h"

#include "file_io.h"
#include "dyn_arr.h"
#include "debug.h"

#define ABILITIES_CONFIG_FILENAME "abilities.conf"


extern struct dyn_arr abilities;

void read_abilities(void);

ability_id_t new_ability_id(void);
struct ability *find_ability(ability_id_t id);

#endif /* ABILITIES_CONFIG_H */
