#include "server_data.h"

pthread_key_t thread_data;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

struct dyn_arr clients = { sizeof(struct client) };

struct map_info map_info = {.length = 160, .height = 36};
map_t map = NULL;

client_id_t player_id_counter = 0;


void all_uq_append(struct update upd)
{
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);
        uq_append(cl->updates, upd);
    }
}

void add_client(struct client *cl)
{
    dyn_arr_append(&clients, cl);
}

struct client *find_client_by(int (*test)(struct client *))
{
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);
        if (test(cl))
        {
            return cl;
        }
    }
    return NULL;
}

struct client *find_client(client_id_t id)
{
    int test(struct client *cl)
    {
        return cl->id == id;
    }

    return find_client_by(test);
}

struct client *find_client_by_nickname(char *nickname)
{
    int test(struct client *cl)
    {
        return strcmp(cl->player->nickname, nickname) == 0;
    }

    return find_client_by(test);
}


struct client *new_client(char *nickname)
{
    struct client *result = malloc(sizeof(*result));
    client_id_t id = new_client_id();

    *result = (struct client) {
        .id = id,
        .player = new_player(nickname, id),
        .updates = new_uq(),
    };

    return result;
}

client_id_t new_client_id(void)
{
    client_id_t result = player_id_counter++;
    if (player_id_counter == 0)
        debug_s( 5, "player id",
"Player ID counter overflowed to 0! Hope there won't be collisions \
when the next ID is needed.");

    return result;
}

void clear_client(struct client *cl)
{
    if (uq_is_nonempty(cl->updates))
        free_uq(cl->updates);
    free(cl->player->nickname);
    free(cl->player);
}

struct player *new_player(char *nickname, client_id_t id)
{
    int player_x = new_player_x();
    struct player *result = malloc(sizeof(*result));

    *result = (struct player) {
        .state = PS_WAIT,
        .id = id,
        .nickname = nickname,
        .hitpoints = INITIAL_HP,
        .pos_x = player_x,
        .pos_y = map[player_x] - 1,
    };

    return result;
}

int new_player_x(void)
{
    return MAP_NOTANK_MARGIN
        + rand() % (map_info.length - 2 * MAP_NOTANK_MARGIN);
}
