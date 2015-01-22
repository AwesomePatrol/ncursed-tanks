#ifndef CLIENT_DRAW_H
#define CLIENT_DRAW_H

#include "colors.h"

/* client_draw.c */
void put_col_str(Color color, int y, int x, const char *str);
void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle);
void draw_map(map_t map, int pos_x, int pos_y, int width, int height);
ScreenMove draw_bullet(int pos_x, int pos_y, int x, int y);
void draw_blank_bullet(int pos_x, int pos_y, int x, int y);
void draw_shoot_menu();
void draw_bullet_explosion(int pos_x, int pos_y, int x, int y);
void draw_stats();
void draw_lobby();
void draw_post_game();

#endif /* CLIENT_DRAW_H */
