#include "bet.h"

int32_t player_init_deck();
void decode_card(bits256 b_blinded_card, bits256 blinded_value, cJSON *dealer_blind_info);
void reveal_card(char *table_id);
int32_t handle_game_state_player(char *table_id);
int32_t handle_verus_player();
