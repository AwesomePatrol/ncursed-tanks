#ifndef SERVER_ABILITIES_H
#define SERVER_ABILITIES_H

#include "common.h"
#include "server_net.h"
#include "server_data.h"

typedef int16_t ability_id_t;


/* if writing ability_type_to_string, change this to use some sort of
 * [(type, str_type)*] list */
AbilityType string_to_ability_type(char *str);

void ability_move(struct client *cl, struct shot *shot);

#endif /* SERVER_ABILITIES_H */
