#include "bet.h"

#define G_ZEROIZED_STATE 0
#define G_TABLE_ACTIVE 1
#define G_TABLE_STARTED 2
#define G_PLAYERS_JOINED 3
#define G_DECK_SHUFFLING_P 4
#define G_DECK_SHUFFLING_D 5
#define G_DECK_SHUFFLING_B 6

struct t_game_info_struct {
	int32_t game_state;
	cJSON *game_info;
};

extern struct t_game_info_struct t_game_info;

const char *game_state_str(int32_t game_state);
cJSON *append_game_state(char *table_id, int32_t game_state, cJSON *game_info);
int32_t get_game_state(char *table_id);
