#include "bet.h"
#include <sqlite3.h>

void sqlite3_init_db_name();
int32_t sqlite3_check_if_table_exists(sqlite3 *db, const char *table_name);
sqlite3 *bet_get_db_instance();
int32_t bet_run_query(char *sql_query);
void bet_create_schema();
void bet_sqlite3_init();
