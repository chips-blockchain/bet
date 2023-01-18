#include "bet.h"

int32_t cashier_sb_deck(char *id, bits256 *d_blinded_deck, int32_t player_id);
void cashier_init_deck(char *table_id);
int32_t cashier_shuffle_deck(char *id);
int32_t reveal_bv(char *table_id);
int32_t handle_game_state_cashier(char *table_id);
int32_t cashier_game_init(char *table_id);
