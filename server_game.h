#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "server_data.h"

/* more -> better */
#define COLLISION_X_PRECISION 16


int16_t new_player_x(void);
struct map_position get_impact_pos(struct player *player, struct shot *shot,
                                   double *impact_t);
map_t map_with_tanks(void);
int16_t damage_to_player(struct f_pair impact_pos, struct f_pair player_pos);


void init_game(void);
void game_cleanup(void);

void start_game(void);
void next_turn(void);
bool end_game_if_needed(void);
void end_game(void);
void reset_game(void);

void shot_update_map(struct map_position impact_pos);
void shot_deal_damage(struct map_position impact_pos);
void process_impact(struct map_position impact_pos);

#endif /* SERVER_GAME_H */
