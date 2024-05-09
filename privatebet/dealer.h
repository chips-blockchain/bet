#include "bet.h"

int32_t add_dealer(char *dealer_id);
int32_t dealer_table_init(struct table t);
bool is_players_shuffled_deck(char *table_id);
int32_t dealer_shuffle_deck(char *id);
int32_t handle_game_state(char *table_id);
int32_t register_table(struct table t);
int32_t dealer_init(struct table t);
