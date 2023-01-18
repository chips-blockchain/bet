#include "bet.h"
#include <sqlite3.h>

#define sql_query_size 1024

void sqlite3_init_db_name();
int32_t sqlite3_check_if_table_id_exists(const char *table_id);
int32_t sqlite3_check_if_table_exists(sqlite3 *db, const char *table_name);
sqlite3 *bet_get_db_instance();
void bet_make_insert_query(int argc, char **argv, char **sql_query);
int32_t bet_run_query(char *sql_query);
void bet_create_schema();
void bet_sqlite3_init();
int32_t sqlite3_delete_dealer(char *dealer_ip);
cJSON *sqlite3_get_dealer_info_details();
cJSON *sqlite3_get_game_details(int32_t opt);
cJSON *bet_show_fail_history();
cJSON *bet_show_success_history();
int32_t bet_store_game_info_details(char *tx_id, char *table_id);
int32_t bet_insert_sc_game_info(char *tx_id, char *table_id, int32_t bh, char *tx_type);
int32_t sqlite3_get_highest_bh();
int32_t insert_player_deck_info_txid_pa_t_d(char *tx_id, char *pa, char *table_id, char *dealer_id);
int32_t update_player_deck_info_a_rG(char *tx_id);
int32_t update_player_deck_info_game_id_p_id(char *tx_id);
int32_t insert_dealer_deck_info();
int32_t insert_cashier_deck_info(char *table_id);
