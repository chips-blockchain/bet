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
int32_t sqlite3_get_game_details(int32_t opt);
void bet_handle_game(int argc, char **argv);
