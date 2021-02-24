#include "commands.h"
#include "storage.h"
#include "misc.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define no_of_tables 8

char *db_name = NULL;

const char *table_names[no_of_tables] = { "dcv_tx_mapping",	"player_tx_mapping", "cashier_tx_mapping",
					  "c_tx_addr_mapping",	"dcv_game_state",    "player_game_state",
					  "cashier_game_state", "dealers_info" };

const char *schemas[no_of_tables] = {
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int)",
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int,  payout_tx_id varchar(100))",
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int)",
	"(payin_tx_id varchar(100) primary key,msig_addr varchar(100), min_notaries int, table_id varchar(100), msig_addr_nodes varchar(100), payin_tx_id_status int, payout_tx_id varchar(100))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(dealer_ip varchar(100) primary key)"
};

void sqlite3_init_db_name()
{
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;
	db_name = calloc(1, 200);
	sprintf(db_name, "%s/.bet/db/pangea.db", homedir);
	printf("%s::%d::db_name::%s\n", __FUNCTION__, __LINE__, db_name);
}

int32_t sqlite3_check_if_table_id_exists(const char *table_id)
{
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	char *sql_query = NULL;
	int32_t rc, retval = 0;

	db = bet_get_db_instance();
	sql_query = calloc(1, 200);

	sprintf(sql_query, "select count(table_id) from c_tx_addr_mapping where table_id = \"%s\";", table_id);
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const int count = sqlite3_column_int(stmt, 0);
		if (count > 0) {
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

int32_t sqlite3_check_if_table_exists(sqlite3 *db, const char *table_name)
{
	sqlite3_stmt *stmt = NULL;
	char *sql_query = NULL;
	int rc, retval = 0;

	db = bet_get_db_instance();
	sql_query = calloc(1, sql_query_size);

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

	rc = sqlite3_open(db_name, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return (0);
	}
	return db;
}

void bet_make_insert_query(int argc, char **argv, char **sql_query)
{
	sprintf(*sql_query, "INSERT INTO %s values(", argv[0]);
	for (int32_t i = 1; i < argc; i++) {
		strcat(*sql_query, argv[i]);
		if ((i + 1) < argc)
			strcat(*sql_query, ",");
		else
			strcat(*sql_query, ");");
	}
}

int32_t bet_run_query(char *sql_query)
{
	sqlite3 *db;
	char *err_msg = NULL;
	int32_t rc = -1;

	if (sql_query == NULL)
		return rc;
	else {
		db = bet_get_db_instance();
		/* Execute SQL statement */
		rc = sqlite3_exec(db, sql_query, NULL, 0, &err_msg);

		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s::in running ::%s\n", err_msg, sql_query);
			sqlite3_free(err_msg);
		}
		sqlite3_close(db);
	}
	return rc;
}

void bet_create_schema()
{
	sqlite3 *db = NULL;
	int rc;
	char *sql_query = NULL, *err_msg = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(1, 2000);
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
	sqlite3_init_db_name();
	bet_create_schema();
}

int32_t sqlite3_delete_dealer(char *dealer_ip)
{
	char *sql_query = NULL;
	int rc;

	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query, "DELETE FROM dealers_info where dealer_ip = \'%s\';", dealer_ip);
	rc = bet_run_query(sql_query);

	if (sql_query)
		free(sql_query);
	return rc;
}
cJSON *sqlite3_get_dealer_info_details()
{
	sqlite3_stmt *stmt = NULL;
	int rc;
	sqlite3 *db;
	char *sql_query = NULL;
	cJSON *dealers_info = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query, "SELECT dealer_ip FROM dealers_info;");
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}
	dealers_info = cJSON_CreateArray();
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON_AddItemToArray(dealers_info, cJSON_CreateString(sqlite3_column_text(stmt, 0)));
	}
	sqlite3_finalize(stmt);
end:
	if (sql_query)
		free(sql_query);
	sqlite3_close(db);
	return dealers_info;
}
cJSON *sqlite3_get_game_details(int32_t opt)
{
	sqlite3_stmt *stmt = NULL, *sub_stmt = NULL;
	char *sql_query = NULL, *sql_sub_query = NULL;
	int rc;
	sqlite3 *db;
	cJSON *game_info = NULL;

	game_info = cJSON_CreateArray();
	db = bet_get_db_instance();
	sql_query = calloc(1, sql_query_size);
	sql_sub_query = calloc(1, sql_query_size);
	if (opt == -1)
		sprintf(sql_query, "select * from player_tx_mapping;");
	else
		sprintf(sql_query, "select * from player_tx_mapping where status = %d;", opt);
	printf("%s::%d::sql_query::%s\n", __FUNCTION__, __LINE__, sql_query);
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", sqlite3_column_text(stmt, 1));
		cJSON_AddStringToObject(game_obj, "tx_id", sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "player_id", sqlite3_column_text(stmt, 2));
		cJSON_AddStringToObject(game_obj, "msig_addr_nodes", sqlite3_column_text(stmt, 3));
		cJSON_AddNumberToObject(game_obj, "status", sqlite3_column_int(stmt, 4));
		cJSON_AddNumberToObject(game_obj, "min_cashiers", sqlite3_column_int(stmt, 5));
		cJSON_AddStringToObject(game_obj, "addr", chips_get_wallet_address());
		sprintf(sql_sub_query, "select * from player_game_state where table_id = \'%s\';",
			sqlite3_column_text(stmt, 1));

		rc = sqlite3_prepare_v2(db, sql_sub_query, -1, &sub_stmt, NULL);
		if (rc != SQLITE_OK) {
			printf("error: %s::%s", sqlite3_errmsg(db), sql_sub_query);
			goto end;
		}
		while ((rc = sqlite3_step(sub_stmt)) == SQLITE_ROW) {
			cJSON_AddItemToObject(game_obj, "game_state", cJSON_Parse(sqlite3_column_text(sub_stmt, 1)));
		}
		sqlite3_finalize(sub_stmt);
		memset(sql_sub_query, 0x00, sql_query_size);
		cJSON_AddItemToArray(game_info, game_obj);
	}
	sqlite3_finalize(stmt);
end:
	if (sql_query)
		free(sql_query);
	if (sql_sub_query)
		free(sql_sub_query);
	sqlite3_close(db);
	return game_info;
}

cJSON *bet_show_fail_history()
{
	sqlite3_stmt *stmt = NULL;
	int rc;
	sqlite3 *db;
	char *sql_query = NULL, *data = NULL, *hex_data = NULL;
	cJSON *game_fail_info = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query, "SELECT table_id,tx_id FROM player_tx_mapping WHERE payout_tx_id is null;");
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}

	game_fail_info = cJSON_CreateArray();

	hex_data = calloc(1, tx_data_size * 2);
	data = calloc(1, tx_data_size);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "tx_id", sqlite3_column_text(stmt, 1));

		memset(hex_data, 0x00, 2 * tx_data_size);
		memset(data, 0x00, tx_data_size);

		chips_extract_data(jstr(game_obj, "tx_id"), &hex_data);
		hexstr_to_str(hex_data, data);
		cJSON_AddItemToObject(game_obj, "game_details", cJSON_Parse(data));
		cJSON_AddItemToArray(game_fail_info, game_obj);
	}
	sqlite3_finalize(stmt);
end:
	if (sql_query)
		free(sql_query);
	if (data)
		free(data);
	if (hex_data)
		free(hex_data);
	sqlite3_close(db);

	return game_fail_info;
}

cJSON *bet_show_success_history()
{
	sqlite3_stmt *stmt = NULL;
	int rc;
	sqlite3 *db;
	char *sql_query = NULL;
	cJSON *game_success_info = NULL;
	char *hex_data = NULL, *data = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(1, sql_query_size);
	sprintf(sql_query, "SELECT table_id,payout_tx_id FROM player_tx_mapping WHERE payout_tx_id is not null;");

	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s::%s", sqlite3_errmsg(db), sql_query);
		goto end;
	}

	game_success_info = cJSON_CreateArray();

	hex_data = calloc(1, tx_data_size * 2);
	data = calloc(1, tx_data_size);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "payout_tx_id", sqlite3_column_text(stmt, 1));

		memset(hex_data, 0x00, 2 * tx_data_size);
		memset(data, 0x00, tx_data_size);

		chips_extract_data(jstr(game_obj, "payout_tx_id"), &hex_data);
		hexstr_to_str(hex_data, data);
		cJSON *temp = cJSON_CreateObject();
		temp = cJSON_Parse(data);
		cJSON_AddItemToObject(game_obj, "game_details", cJSON_Parse(data));
		cJSON_AddItemToArray(game_success_info, game_obj);
	}
	sqlite3_finalize(stmt);
end:
	if (sql_query)
		free(sql_query);
	if (data)
		free(data);
	if (hex_data)
		free(hex_data);
	sqlite3_close(db);

	return game_success_info;
}
