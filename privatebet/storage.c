#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "storage.h"

#define no_of_tables 3

char *db_name = NULL;

const char *table_names[no_of_tables] = { "dcv_tx_mapping", "player_tx_mapping", "cashier_tx_mapping" };

const char *schemas[no_of_tables] = { "(tx_id varchar(100) primary key,table_id varchar(100), status bool)",
				      "(tx_id varchar(100) primary key,table_id varchar(100), status bool)",
				      "(tx_id varchar(100) primary key,table_id varchar(100), status bool)" };

void sqlite3_init_db_name()
{
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;
	db_name = calloc(1, 200);
	sprintf(db_name, "%s/.bet/db/pangea.db", homedir);
}
int32_t sqlite3_check_if_table_exists(sqlite3 *db, const char *table_name)
{
	sqlite3_stmt *stmt = NULL;
	char *sql_query = NULL;
	int rc, retval = 0;

	db = bet_get_db_instance();
	sql_query = calloc(1, 200);

	sprintf(sql_query, "select name from sqlite_master where type = \"table\" and name =\"%s\";", table_name);
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *name = sqlite3_column_text(stmt, 0);
		if (strcmp(name, table_name) == 0) {
			retval = 1;
			break;
		}
	}
	sqlite3_finalize(stmt);
end:
	if (sql_query)
		free(sql_query);
	return retval;
}

sqlite3 *bet_get_db_instance()
{
	sqlite3 *db = NULL;
	int rc;

	sqlite3_init_db_name();
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, db_name);
	rc = sqlite3_open(db_name, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return (0);
	}
	return db;
}

void bet_run_query(char *sql_query)
{
	sqlite3 *db;
	char *err_msg = NULL;
	int rc;

	db = bet_get_db_instance();
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql_query, NULL, 0, &err_msg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s::%s\n", err_msg, sql_query);
		sqlite3_free(err_msg);
	}
	sqlite3_close(db);
}

void bet_create_schema()
{
	sqlite3 *db = NULL;
	int rc;
	char *sql_query = NULL, *err_msg = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(1, 200);
	for (int32_t i = 0; i < no_of_tables; i++) {
		if (sqlite3_check_if_table_exists(db, table_names[i]) == 0) {
			sprintf(sql_query, "CREATE TABLE %s %s;", table_names[i], schemas[i]);

			rc = sqlite3_exec(db, sql_query, NULL, 0, &err_msg);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "SQL error: %s::%s\n", err_msg, sql_query);
				sqlite3_free(err_msg);
			}
			memset(sql_query, 0x00, 200);
		}
	}
	if (sql_query)
		free(sql_query);
	sqlite3_close(db);
}

void bet_sqlite3_init()
{
	bet_create_schema();
}
