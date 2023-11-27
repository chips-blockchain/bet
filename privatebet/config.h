#include "bet.h"
cJSON *bet_read_json_file(char *file_name);
void bet_parse_dealer_config_ini_file();
void bet_parse_player_config_ini_file();
void bet_parse_cashier_config_ini_file();
void bet_display_cashier_hosted_gui();
int32_t bet_parse_bets();
void bet_parse_blockchain_config_ini_file();
bool bet_is_new_block_set();
int32_t bet_parse_verus_dealer();
int32_t bet_parse_verus_player();