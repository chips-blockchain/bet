#include "commands.h"
#include "storage.h"
#include "misc.h"
#include "err.h"
#include "vdxf.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define no_of_tables 13

char *db_name = NULL;

const char *table_names[no_of_tables] = { "dcv_tx_mapping",     "player_tx_mapping", "cashier_tx_mapping",
					  "c_tx_addr_mapping",  "dcv_game_state",    "player_game_state",
					  "cashier_game_state", "dealers_info",      "game_info",
					  "sc_games_info",      "player_deck_info",  "dealer_deck_info",
					  "cashier_deck_info" };

const char *schemas[no_of_tables] = {
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int)",
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int,  payout_tx_id varchar(100))",
	"(tx_id varchar(100) primary key,table_id varchar(100), player_id varchar(100), msig_addr varchar(100), status bool, min_cashiers int)",
	"(payin_tx_id varchar(100) primary key,msig_addr varchar(100), min_notaries int, table_id varchar(100), msig_addr_nodes varchar(100), payin_tx_id_status int, payout_tx_id varchar(100))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(table_id varchar(100) primary key,game_state varchar(1000))",
	"(dealer_ip varchar(100) primary key)",
	"(tx_id varchar(100) primary key,table_id varchar(100))",
	"(tx_id varchar(100) primary key,table_id varchar(100), bh int, tx_type varchar(20))",
	"(game_id varchar(100) primary key, tx_id varchar(100), pa varchar(100), table_id varchar(100), dealer_id varchar(100), player_id int, player_priv varchar(100), player_deck_priv varchar(4000))",
	"(game_id varchar(100) primary key, perm varchar(100), dealer_deck_priv varchar(4000))",
	"(game_id varchar(100), player_id int, perm varchar(100), cashier_deck_priv varchar(4000), CONSTRAINT game_id PRIMARY KEY(game_id, player_id))"
};

void sqlite3_init_db_name()
{
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;
	db_name = calloc(sql_query_size, sizeof(char));
	sprintf(db_name, "%s/.bet/db/pangea.db", homedir);
	dlg_info("The DB to store game info is ::%s", db_name);
}

int32_t sqlite3_check_if_table_id_exists(const char *table_id)
{
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	char *sql_query = NULL;
	int32_t rc, retval = 0;

	db = bet_get_db_instance();
	sql_query = calloc(sql_query_size, sizeof(char));

	sprintf(sql_query, "select count(table_id) from c_tx_addr_mapping where table_id = \"%s\";", table_id);
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
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
	sqlite3_close(db);
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
	sql_query = calloc(sql_query_size, sizeof(char));

	sprintf(sql_query, "select name from sqlite_master where type = \"table\" and name =\"%s\";", table_name);
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
		goto end;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *name = (const char *)sqlite3_column_text(stmt, 0);
		if (strcmp(name, table_name) == 0) {
			retval = 1;
			break;
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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
		dlg_error("Can't open database: %s", sqlite3_errmsg(db));
		return (0);
	}
	sqlite3_busy_timeout(db, 1000);
	return db;
}

void bet_make_insert_query(int argc, char **argv, char **sql_query)
{
	char *null_str = "NULL";
	sprintf(*sql_query, "INSERT INTO %s values(", argv[0]);
	for (int32_t i = 1; i < argc; i++) {
		if (strlen(argv[i]) != 0) {
			strcat(*sql_query, argv[i]);
		} else {
			strcat(*sql_query, null_str);
			dlg_info("some arg is null");
		}
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
	int32_t rc = -1, retval = OK;

	if (sql_query == NULL)
		return rc;
	else {
		db = bet_get_db_instance();
		/* Execute SQL statement */
		rc = sqlite3_exec(db, sql_query, NULL, 0, &err_msg);

		if (rc != SQLITE_OK) {
			dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
			sqlite3_free(err_msg);
			retval = ERR_SQL;
		}
		sqlite3_close(db);
	}

	return retval;
}

void bet_create_schema()
{
	sqlite3 *db = NULL;
	int rc;
	char *sql_query = NULL, *err_msg = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(sql_query_size, sizeof(char));
	for (int32_t i = 0; i < no_of_tables; i++) {
		if (sqlite3_check_if_table_exists(db, table_names[i]) == 0) {
			sprintf(sql_query, "CREATE TABLE %s %s;", table_names[i], schemas[i]);

			rc = sqlite3_exec(db, sql_query, NULL, 0, &err_msg);
			if (rc != SQLITE_OK) {
				dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db),
					  sql_query);
				sqlite3_free(err_msg);
			}
			memset(sql_query, 0x00, 200);
		}
	}
	sqlite3_close(db);
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

	sql_query = calloc(sql_query_size, sizeof(char));
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
	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "SELECT dealer_ip FROM dealers_info;");
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
		goto end;
	}
	dealers_info = cJSON_CreateArray();
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON_AddItemToArray(dealers_info, cJSON_CreateString((const char *)sqlite3_column_text(stmt, 0)));
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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
	sql_query = calloc(sql_query_size, sizeof(char));
	sql_sub_query = calloc(sql_query_size, sizeof(char));
	if (opt == -1) {
		sprintf(sql_query, "select * from player_tx_mapping;");
	} else {
		sprintf(sql_query, "select * from player_tx_mapping where status = %d;", opt);
	}

	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
		goto end;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", (const char *)sqlite3_column_text(stmt, 1));
		cJSON_AddStringToObject(game_obj, "tx_id", (const char *)sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "player_id", (const char *)sqlite3_column_text(stmt, 2));
		cJSON_AddStringToObject(game_obj, "msig_addr_nodes", (const char *)sqlite3_column_text(stmt, 3));
		cJSON_AddNumberToObject(game_obj, "status", sqlite3_column_int(stmt, 4));
		cJSON_AddNumberToObject(game_obj, "min_cashiers", sqlite3_column_int(stmt, 5));
		cJSON_AddStringToObject(game_obj, "addr", chips_get_wallet_address());
		sprintf(sql_sub_query, "select * from player_game_state where table_id = \'%s\';",
			sqlite3_column_text(stmt, 1));

		rc = sqlite3_prepare_v2(db, sql_sub_query, -1, &sub_stmt, NULL);
		if (rc != SQLITE_OK) {
			dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
			goto end;
		}
		while ((rc = sqlite3_step(sub_stmt)) == SQLITE_ROW) {
			cJSON_AddItemToObject(game_obj, "game_state",
					      cJSON_Parse((const char *)sqlite3_column_text(sub_stmt, 1)));
		}
		sqlite3_finalize(sub_stmt);
		memset(sql_sub_query, 0x00, sql_query_size);
		cJSON_AddItemToArray(game_info, game_obj);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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
	int retval = OK;
	sqlite3 *db;
	char *sql_query = NULL, *data = NULL, *hex_data = NULL;
	cJSON *game_fail_info = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "SELECT table_id,tx_id FROM player_tx_mapping WHERE payout_tx_id is null;");
	retval = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (retval != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", retval, sqlite3_errmsg(db), sql_query);
		goto end;
	}

	game_fail_info = cJSON_CreateArray();

	hex_data = calloc(tx_data_size * 2, sizeof(char));
	data = calloc(tx_data_size, sizeof(char));

	while ((retval = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", (const char *)sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "tx_id", (const char *)sqlite3_column_text(stmt, 1));

		memset(hex_data, 0x00, 2 * tx_data_size);
		memset(data, 0x00, tx_data_size);

		retval = chips_extract_data(jstr(game_obj, "tx_id"), &hex_data);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
			goto end;
		}
		hexstr_to_str(hex_data, data);
		cJSON_AddItemToObject(game_obj, "game_details", cJSON_Parse(data));
		cJSON_AddItemToArray(game_fail_info, game_obj);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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
	int32_t retval = OK;
	char *sql_query = NULL, *hex_data = NULL, *data = NULL;
	cJSON *game_success_info = NULL;
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "SELECT table_id,payout_tx_id FROM player_tx_mapping WHERE payout_tx_id is not null;");

	retval = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (retval != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", retval, sqlite3_errmsg(db), sql_query);
		goto end;
	}

	game_success_info = cJSON_CreateArray();

	hex_data = calloc(tx_data_size * 2, sizeof(char));
	data = calloc(tx_data_size, sizeof(char));

	while ((retval = sqlite3_step(stmt)) == SQLITE_ROW) {
		cJSON *game_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(game_obj, "table_id", (const char *)sqlite3_column_text(stmt, 0));
		cJSON_AddStringToObject(game_obj, "payout_tx_id", (const char *)sqlite3_column_text(stmt, 1));

		memset(hex_data, 0x00, 2 * tx_data_size);
		memset(data, 0x00, tx_data_size);

		retval = chips_extract_data(jstr(game_obj, "payout_tx_id"), &hex_data);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
			goto end;
		}
		hexstr_to_str(hex_data, data);
		cJSON_AddItemToObject(game_obj, "game_details", cJSON_Parse(data));
		cJSON_AddItemToArray(game_success_info, game_obj);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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

int32_t bet_store_game_info_details(char *tx_id, char *table_id)
{
	int32_t argc, retval = OK;
	char **argv = NULL, *sql_query = NULL;

	argc = 3;
	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "game_info");
	sprintf(argv[1], "\'%s\'", tx_id);
	sprintf(argv[2], "\'%s\'", table_id);
	sql_query = calloc(sql_query_size, sizeof(char));
	bet_make_insert_query(argc, argv, &sql_query);
	retval = bet_run_query(sql_query);
	if (retval != SQLITE_OK)
		retval = ERR_SQL;

	bet_dealloc_args(argc, &argv);
	if (sql_query)
		free(sql_query);

	return retval;
}

int32_t bet_insert_sc_game_info(char *tx_id, char *table_id, int32_t bh, char *tx_type)
{
	int32_t argc, retval = OK;
	char **argv = NULL, *sql_query = NULL;

	argc = 5;

	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "sc_games_info");

	if (tx_id)
		sprintf(argv[1], "\'%s\'", tx_id);

	if (table_id)
		sprintf(argv[2], "\'%s\'", table_id);

	sprintf(argv[3], "%d", bh);

	if (tx_type)
		sprintf(argv[4], "\'%s\'", tx_type);

	sql_query = calloc(sql_query_size, sizeof(char));
	bet_make_insert_query(argc, argv, &sql_query);

	retval = bet_run_query(sql_query);
	if (retval != SQLITE_OK)
		retval = ERR_SQL;

	bet_dealloc_args(argc, &argv);
	if (sql_query)
		free(sql_query);

	return retval;
}

int32_t sqlite3_get_highest_bh()
{
	sqlite3_stmt *stmt = NULL;
	int32_t rc = OK, bh = 0;
	sqlite3 *db;
	char *sql_query = NULL;

	db = bet_get_db_instance();
	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "SELECT max(bh) FROM sc_games_info;");
	if ((rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL)) != SQLITE_OK) {
		dlg_error("error_code :: %d, error msg ::%s, \n query ::%s", rc, sqlite3_errmsg(db), sql_query);
		goto end;
	} else {
		if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			bh = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);
	}
	sqlite3_close(db);
end:
	if (sql_query)
		free(sql_query);
	sqlite3_close(db);
	return bh;
}

int32_t insert_player_deck_info_txid_pa_t_d(char *tx_id, char *pa, char *table_id, char *dealer_id)
{
	int32_t retval = OK;
	char *sql_query = NULL;

	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query,
		"insert into player_deck_info(tx_id, pa, table_id, dealer_id) values(\'%s\', \'%s\', \'%s\', \'%s\')",
		tx_id, pa, table_id, dealer_id);
	retval = bet_run_query(sql_query);

	return retval;
}

int32_t update_player_deck_info_a_rG(char *tx_id)
{
	int32_t retval = OK;
	char player_priv[65], str[65], *player_deck_priv = NULL, *sql_query = NULL;
	cJSON *cardinfo = NULL;

	bits256_str(player_priv, p_deck_info.p_kp.priv);

	cardinfo = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(cardinfo, bits256_str(str, p_deck_info.player_r[i].priv));
	}
	cJSON_hex(cardinfo, &player_deck_priv);

	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "update player_deck_info set player_priv = \'%s\', deck_priv = \'%s\' where tx_id = \'%s\'",
		player_priv, player_deck_priv, tx_id);
	retval = bet_run_query(sql_query);
	return retval;
}

int32_t update_player_deck_info_game_id_p_id(char *tx_id)
{
	int32_t retval = OK;
	char *sql_query = NULL, game_id_str[65];

	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "update player_deck_info set game_id = \'%s\', player_id = %d where tx_id = \'%s\'",
		bits256_str(game_id_str, p_deck_info.game_id), p_deck_info.player_id, tx_id);
	retval = bet_run_query(sql_query);
	return retval;
}

int32_t insert_dealer_deck_info()
{
	int32_t retval = OK;
	char *sql_query = NULL, game_id_str[65], str[65], *dealer_deck_priv = NULL, *perm = NULL;
	cJSON *d_perm = NULL, *d_blindinfo = NULL;

	d_perm = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddi64bits(d_perm, d_deck_info.d_permi[i]);
	}
	cJSON_hex(d_perm, &perm);

	d_blindinfo = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(d_blindinfo, bits256_str(str, d_deck_info.dealer_r[i].priv));
	}
	cJSON_hex(d_blindinfo, &dealer_deck_priv);

	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query,
		"insert into dealer_deck_info(game_id, perm, dealer_deck_priv) values(\'%s\', \'%s\', \'%s\')",
		game_id_str, perm, dealer_deck_priv);
	retval = bet_run_query(sql_query);
	return retval;
}

int32_t insert_cashier_deck_info(char *table_id)
{
	int32_t retval = OK, num_players;
	char *sql_query = NULL, *game_id_str = NULL, str[65], *cashier_deck_priv = NULL, *perm = NULL;
	cJSON *t_player_info = NULL, *b_perm = NULL, *b_blindinfo = NULL;

	b_perm = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddi64bits(b_perm, b_deck_info.b_permi[i]);
	}
	cJSON_hex(b_perm, &perm);

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	num_players = jint(t_player_info, "num_players");

	sql_query = calloc(sql_query_size, sizeof(char));
	for (int32_t i = 0; i < num_players; i++) {
		memset(sql_query, 0x00, sql_query_size);
		b_blindinfo = cJSON_CreateArray();
		for (int j = 0; j < CARDS777_MAXCARDS; j++) {
			jaddistr(b_blindinfo, bits256_str(str, b_deck_info.cashier_r[i][j].priv));
		}
		cJSON_hex(b_blindinfo, &cashier_deck_priv);
		sprintf(sql_query,
			"insert into cashier_deck_info(game_id, perm, player_id, cashier_deck_priv) values(\'%s\', \'%s\', %d, \'%s\')",
			game_id_str, perm, i, cashier_deck_priv);
		retval = bet_run_query(sql_query);
	}
	return retval;
}
