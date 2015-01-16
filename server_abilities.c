#include "server_abilities.h"
#include "abilities_config.h"

AbilityType string_to_ability_type(char *str)
{
    bool str_eq(char *s1, char *s2)
    {
        return strcmp(s1, s2) == 0;
    }

    if      (str_eq(str, "NONE"))        return A_NONE;
    else if (str_eq(str, "DOUBLE_SHOT")) return A_DOUBLE_SHOT;
    else if (str_eq(str, "MOVE"))        return A_MOVE;
    else if (str_eq(str, "SNIPE"))       return A_SNIPE;
    else
        return A_NONE;
}

void ability_move(struct client *cl, struct shot *shot)
{
    /*TODO: move tank by cl->player->ability.parameter*/
}
