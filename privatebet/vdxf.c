#include "bet.h"
#include "vdxf.h"
#include "commands.h"
#include "misc.h"
#include "err.h"
#include "game.h"
#include "storage.h"

struct table player_t = { 0 };
char T_PLAYER_KEYS[9][128] = { T_PLAYER1_KEY, T_PLAYER2_KEY, T_PLAYER3_KEY, T_PLAYER4_KEY, T_PLAYER5_KEY,
			       T_PLAYER6_KEY, T_PLAYER7_KEY, T_PLAYER8_KEY, T_PLAYER9_KEY };

char *get_vdxf_id(char *key_name)
{
	int argc = 3;
	char **argv = NULL;
	cJSON *argjson = NULL;

	if (!key_name)
		return NULL;

	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getvdxfid", key_name);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

	bet_dealloc_args(argc, &argv);
	return jstr(argjson, "vdxfid");
}

char *get_key_vdxf_id(char *key_name)
{
	if (!key_name)
		return NULL;

	return get_vdxf_id(get_full_key(key_name));
}

char *get_full_key(char *key_name)
{
	char *full_key = NULL;

	if (!key_name)
		return NULL;

	full_key = calloc(1, 128);
	strcpy(full_key, "chips.vrsc::poker.");
	strncat(full_key, key_name, strlen(key_name));

	return full_key;
}

char *get_key_data_type(char *key_name)
{
	if (!key_name)
		return BYTEVECTOR_VDXF_ID;

	if (strcmp(key_name, T_GAME_ID_KEY) == 0) {
		return BYTEVECTOR_VDXF_ID;
	}
	return BYTEVECTOR_VDXF_ID;
}

char *get_key_data_vdxf_id(char *key_name, char *data)
{
	char full_key[256] = { 0 };

	if ((!key_name) || (!data))
		return NULL;

	strcpy(full_key, key_name);
	strncat(full_key, data, strlen(data));

	return get_vdxf_id(full_key);
}

cJSON *update_with_retry(int argc, char **argv)
{
	int32_t retries = 3, i = 0;
	cJSON *argjson = NULL;

	do {
		argjson = cJSON_CreateObject();
		make_command(argc, argv, &argjson);
		if (jint(argjson, "error") == 0) {
			wait_for_a_blocktime();
			if (check_if_tx_exists(jstr(argjson, "tx")))
				break;
		}
		dlg_warn("Retrying the updateidentity");
		sleep(1);
		i++;
	} while (i < retries);

	if (jint(argjson, "error")) {
		dlg_error("Error in doing the updateidentity");
	}
	return argjson;
}

cJSON *update_cmm(char *id, cJSON *cmm)
{
	cJSON *id_info = NULL, *argjson = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 };

	if ((NULL == id) || (NULL == cmm) || (NULL == verus_chips_cli)) {
		return NULL;
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", get_vdxf_id(POKER_CHIPS_VDXF_ID));
	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = update_with_retry(argc, argv);

#if 0
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
#endif

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

cJSON *append_pa_to_cmm(char *id, char *pa)
{
	int argc;
	char **argv = NULL, params[arg_size] = { 0 };
	cJSON *id_info = NULL, *argjson = NULL, *pa_arr = NULL, *cmm = NULL;

	if ((!id) || (!pa) || (!verus_chips_cli))
		return NULL;

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", get_vdxf_id(POKER_CHIPS_VDXF_ID));

	cmm = get_cmm(id, 0);
	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);

	pa_arr = cJSON_CreateArray();
	pa_arr = get_primaryaddresses(id, 0);
	jaddistr(pa_arr, pa);
	dlg_info("%s", cJSON_Print(pa_arr));
	cJSON_AddItemToObject(id_info, "primaryaddresses", pa_arr);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = update_with_retry(argc, argv);

#if 0
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
#endif

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

cJSON *get_cmm(char *id, int16_t full_id)
{
	int32_t retval = OK, argc;
	char **argv = NULL, params[128] = { 0 };
	cJSON *argjson = NULL, *cmm = NULL;

	if (NULL == id) {
		return NULL;
	}

	strncpy(params, id, strlen(id));
	if (0 == full_id) {
		strcat(params, ".poker.chips10sec@");
	}
	argc = 4;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params, "-1");

	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval != OK) {
		dlg_info("%s", bet_err_str(retval));
		goto end;
	}

	cmm = cJSON_CreateObject();
	cmm = cJSON_GetObjectItem(cJSON_GetObjectItem(argjson, "identity"), "contentmultimap");

	if (cmm) {
		cmm->next = NULL;
	}
end:
	bet_dealloc_args(argc, &argv);
	return cmm;
}

cJSON *update_primaryaddresses(char *id, cJSON *primaryaddress)
{
	cJSON *id_info = NULL, *argjson = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 };

	if ((NULL == id) || (NULL == primaryaddress) || (NULL == verus_chips_cli)) {
		return NULL;
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", get_vdxf_id(POKER_CHIPS_VDXF_ID));
	cJSON_AddItemToObject(id_info, "primaryaddresses", primaryaddress);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = update_with_retry(argc, argv);
#if 0
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
#endif

end:
	bet_dealloc_args(argc, &argv);
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	return argjson;
}

cJSON *get_primaryaddresses(char *id, int16_t full_id)
{
	int32_t argc, retval = OK;
	char **argv = NULL, params[128] = { 0 };
	cJSON *argjson = NULL, *pa = NULL, *id_obj = NULL;

	if (NULL == id) {
		return NULL;
	}
	strncpy(params, id, strlen(id));
	if (0 == full_id) {
		strcat(params, ".poker.chips10sec@");
	}
	argc = 4;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params, "-1");

	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval != OK) {
		dlg_error("%s::%d::%s\n", __func__, __LINE__, bet_err_str(retval));
		goto end;
	}

	id_obj = cJSON_GetObjectItem(argjson, "identity");
	pa = cJSON_CreateArray();
	pa = cJSON_GetObjectItem(id_obj, "primaryaddresses");

	pa->next = NULL;

end:
	bet_dealloc_args(argc, &argv);
	return pa;
}

cJSON *get_cmm_key_data(char *id, int16_t full_id, char *key)
{
	cJSON *cmm = NULL, *cmm_key_data = NULL;

	if ((id == NULL) || (key == NULL))
		return NULL;

	cmm = cJSON_CreateObject();
	if ((cmm = get_cmm(id, full_id)) == NULL)
		return NULL;
	cmm_key_data = cJSON_CreateObject();
	if ((cmm_key_data = cJSON_GetObjectItem(cmm, key)) == NULL)
		return NULL;
	cmm_key_data->next = NULL;
	return cmm_key_data;
}

cJSON *get_id_key_data(char *id, int16_t full_id, char *key)
{
	cJSON *cmm = NULL, *cmm_key_data = NULL;

	cmm = get_cmm(id, full_id);

	if (NULL == cmm) {
		return NULL;
	}
	cmm_key_data = cJSON_CreateObject();
	cmm_key_data = cJSON_GetObjectItem(cmm, key);

	return cmm_key_data;
}

cJSON *update_t_game_ids(char *id)
{
	char hexstr[65], *game_ids_info_hex = NULL;
	cJSON *game_ids_info = NULL, *t_game_ids = NULL, *cmm = NULL;

	game_ids_info = cJSON_CreateArray();
	jaddistr(game_ids_info, bits256_str(hexstr, game_id));
	cJSON_hex(game_ids_info, &game_ids_info_hex);

	t_game_ids = cJSON_CreateObject();
	jaddstr(t_game_ids, get_vdxf_id(BYTEVECTOR_VDXF_ID), game_ids_info_hex);

	cmm = cJSON_CreateObject();
	cJSON_AddItemToObject(cmm, get_vdxf_id(T_GAME_ID_KEY), t_game_ids);
	cJSON *out = update_cmm(id, cmm);

	return out;
}

struct table *get_dealers_config_table(char *dealer_id, char *table_id)
{
	cJSON *dealer_cmm_data = NULL;
	char *str = NULL;
	uint8_t *table_data = NULL;
	struct table *t = NULL;

	if (NULL == dealer_id)
		goto end;

	dealer_cmm_data = cJSON_CreateObject();
	dealer_cmm_data = get_cmm_key_data(dealer_id, 0, get_vdxf_id(T_TABLE_INFO_KEY));
	if (dealer_cmm_data == NULL) {
		dlg_info("%s::%d::The key ::%s is not found in the cmm of id ::%s\n", __FUNCTION__, __LINE__,
			 get_vdxf_id(T_TABLE_INFO_KEY), dealer_id);
		goto end;
	}

	//TODO:Right now we dealing with single table, when multi table support comes,
	// we need to make checks whether the table with the specific name exists or not.

	str = jstr(cJSON_GetArrayItem(dealer_cmm_data, 0), get_vdxf_id(STRING_VDXF_ID));

	table_data = calloc(1, (strlen(str) + 1) / 2);
	decode_hex(table_data, (strlen(str) + 1) / 2, str);

	t = calloc(1, sizeof(struct table));
	t = (struct table *)table_data;

end:
	return t;
}

cJSON *get_cashiers_info(char *cashier_id)
{
	cJSON *cashier_cmm_data = NULL;

	cashier_cmm_data = cJSON_CreateObject();
	cashier_cmm_data = get_cmm_key_data(cashier_id, 0, get_vdxf_id(CASHIERS_KEY));

end:
	return cashier_cmm_data;
}

cJSON *update_cashiers(char *ip)
{
	cJSON *cashiers_info = NULL, *out = NULL, *ip_obj = NULL, *cashier_ips = NULL;

	cashiers_info = cJSON_CreateObject();
	cashier_ips = get_cashiers_info("cashiers");

	ip_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(ip_obj, get_vdxf_id(STRING_VDXF_ID), ip);

	if (NULL == cashier_ips)
		cashier_ips = cJSON_CreateArray();
	else {
		for (int i = 0; i < cJSON_GetArraySize(cashier_ips); i++) {
			if (0 == strcmp(jstr(cJSON_GetArrayItem(cashier_ips, i), get_vdxf_id(STRING_VDXF_ID)), ip)) {
				dlg_info("%s::%d::The latest data of this ID contains this %s\n", __FUNCTION__,
					 __LINE__, ip);
				goto end;
			}
		}
	}

	cJSON_AddItemToArray(cashier_ips, ip_obj);
	cJSON_AddItemToObject(cashiers_info, get_vdxf_id(CASHIERS_KEY), cashier_ips);
	out = update_cmm("cashiers", cashiers_info);

end:
	return out;
}

bool is_dealer_exists(char *dealer_id)
{
	cJSON *dealers_info = NULL, *dealer_ids = NULL;

	if (!dealer_id)
		return false;

	dealers_info = get_cJSON_from_id_key("dealers", DEALERS_KEY);
	if (!dealers_info) {
		dlg_info("Unable to fetch the dealers info");
		return false;
	}
	dealer_ids = cJSON_CreateArray();
	dealer_ids = cJSON_GetObjectItem(dealers_info, "dealers");

	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
		if (0 == strcmp(dealer_id, jstri(dealer_ids, i))) {
			return true;
		}
	}
	return false;
}

int32_t get_player_id(int *player_id)
{
	char *game_id_str = NULL, hexstr[65];
	cJSON *t_player_info = NULL, *player_info = NULL;

	game_id_str = get_str_from_id_key(player_config.table_id, get_vdxf_id(T_GAME_ID_KEY));
	if (!game_id_str)
		return ERR_GAME_ID_NOT_FOUND;

	p_deck_info.game_id = bits256_conv(game_id_str);
	dlg_info("%s::%s", game_id_str, bits256_str(hexstr, p_deck_info.game_id));
	t_player_info = get_cJSON_from_id_key_vdxfid(player_config.table_id,
						     get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	if (!t_player_info)
		return ERR_T_PLAYER_INFO_NULL;

	player_info = cJSON_CreateArray();
	player_info = jobj(t_player_info, "player_info");
	if (!player_info)
		return ERR_T_PLAYER_INFO_CORRUPTED;
	dlg_info("%s", cJSON_Print(player_info));
	for (int32_t i = 0; i < cJSON_GetArraySize(player_info); i++) {
		if (strstr(jstri(player_info, i), player_config.primaryaddress)) {
			strtok(jstri(player_info, i), "_");
			strtok(NULL, "_");
			*player_id = atoi(strtok(NULL, "_"));
			dlg_info("player id::%d", *player_id);
			return OK;
		}
	}
	return ERR_PLAYER_NOT_EXISTS;
}

int32_t join_table()
{
	int32_t retval = OK;
	cJSON *data = NULL, *op_id = NULL, *op_id_info = NULL;

	data = cJSON_CreateObject();
	cJSON_AddStringToObject(data, "dealer_id", player_config.dealer_id);
	cJSON_AddStringToObject(data, "table_id", player_config.table_id);
	cJSON_AddStringToObject(data, "primaryaddress", player_config.primaryaddress);

	op_id = verus_sendcurrency_data(data);
	if (op_id) {
		op_id_info = get_z_getoperationstatus(jstr(op_id, "op_id"));
		if (op_id_info) {
			while (0 == strcmp(jstr(jitem(op_id_info, 0), "status"), "executing")) {
				sleep(1);
				op_id_info = get_z_getoperationstatus(jstr(op_id, "op_id"));
			}
			if (0 != strcmp(jstr(jitem(op_id_info, 0), "status"), "success")) {
				return ERR_SENDCURRENCY;
			}

			char *txid = jstr(jobj(jitem(op_id_info, 0), "result"), "txid");
			strcpy(player_config.txid, txid);
			dlg_info("payin_tx::%s", txid);
			retval = check_player_join_status(player_config.table_id, player_config.primaryaddress);
			if (retval)
				return retval;
#if 0
			retval = insert_player_deck_info_txid_pa_t_d(txid, player_config.primaryaddress,
								     player_config.table_id, player_config.dealer_id);
			if (retval)
				return retval;
#endif
		}
	}
end:
	return retval;
}

static void copy_table_to_struct_t(cJSON *t_table_info)
{
	player_t.max_players = jint(t_table_info, "max_players");
	float_to_uint32_s(&player_t.big_blind, jdouble(t_table_info, "big_blind"));
	float_to_uint32_s(&player_t.min_stake, jdouble(t_table_info, "min_stake"));
	float_to_uint32_s(&player_t.max_stake, jdouble(t_table_info, "max_stake"));
	strcpy(player_t.table_id, jstr(t_table_info, "table_id"));
	strcpy(player_t.dealer_id, jstr(t_table_info, "dealer_id"));
}

int32_t find_table()
{
	int32_t retval = OK;
	cJSON *t_table_info = NULL, *dealer_ids = NULL;

	if (!is_id_exists("dealers", 0)) {
		return ERR_NO_DEALERS_FOUND;
	}
	/*
	* Check if the player wallet has suffiecient funds to join the table 
	*/
	if (!check_if_enough_funds_avail(player_config.table_id)) {
		return ERR_CHIPS_INSUFFICIENT_FUNDS;
	}
	/*
	* Check if the configured table meets the preconditions for the player to join the table
	*/
	t_table_info = cJSON_CreateObject();
	retval = check_if_d_t_available(player_config.dealer_id, player_config.table_id, &t_table_info);
	if (retval == OK) {
		copy_table_to_struct_t(t_table_info);
		return retval;
	}
	dlg_info("Unable to join preconfigured table ::%s, checking for any other available tables...", bet_err_str(retval));
	dealer_ids = cJSON_CreateArray();
	dealer_ids = get_cJSON_from_id_key("dealers", DEALERS_KEY);
	if (!dealer_ids) {
		return ERR_NO_DEALERS_FOUND;
	}
	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
		t_table_info = get_available_t_of_d(jstri(dealer_ids, i));
		if (t_table_info) {
			strncpy(player_config.dealer_id, jstri(dealer_ids, i), sizeof(player_config.dealer_id));
			strncpy(player_config.table_id, jstr(t_table_info, "table_id"), sizeof(player_config.table_id));
			copy_table_to_struct_t(t_table_info);
			return OK;
		}
	}
	return ERR_NO_TABLES_FOUND;
}

bool is_id_exists(char *id, int16_t full_id)
{
	int32_t argc = 3, retval = OK, id_exists = false;
	char **argv = NULL;
	char params[128] = { 0 };
	cJSON *argjson = NULL;

	strncpy(params, id, strlen(id));
	if (0 == full_id) {
		strcat(params, ".poker.chips10sec@");
	}
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params);

	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval == OK)
		id_exists = true;

	bet_dealloc_args(argc, &argv);
	return id_exists;
}

int32_t check_player_join_status(char *table_id, char *pa)
{
	int32_t block_count = 0, retval = ERR_PA_NOT_ADDED_TO_TABLE;
	int32_t block_wait_time =
		5; //This is the wait time in number of blocks upto which player can look for its table joining update

	block_count = chips_get_block_count() + block_wait_time;
	do {
		cJSON *pa_arr = cJSON_CreateArray();
		pa_arr = get_primaryaddresses(table_id, 0);
		for (int32_t i = 0; i < cJSON_GetArraySize(pa_arr); i++) {
			if (0 == strcmp(jstri(pa_arr, i), pa)) {
				retval = OK;
				goto end;
			}
		}
		sleep(2);
	} while (chips_get_block_count() <= block_count);
end:
	return retval;
}

cJSON *get_z_getoperationstatus(char *op_id)
{
	int argc = 3;
	char **argv = NULL, op_param[arg_size] = { 0 };
	cJSON *argjson = NULL, *op_id_arr = NULL;

	if (NULL == op_id) {
		return NULL;
	}
	bet_alloc_args(argc, &argv);
	op_id_arr = cJSON_CreateArray();
	jaddistr(op_id_arr, op_id);
	snprintf(op_param, arg_size, "\'%s\'", cJSON_Print(op_id_arr));
	argv = bet_copy_args(argc, verus_chips_cli, "z_getoperationstatus", op_param);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

	bet_dealloc_args(argc, &argv);
	return argjson;
}

cJSON *verus_sendcurrency_data(cJSON *data)
{
	int32_t hex_data_len, argc, minconf = 1;
	double fee = 0.0001;
	char *hex_data = NULL, **argv = NULL, params[4][arg_size] = { 0 };
	cJSON *currency_detail = NULL, *argjson = NULL, *tx_params = NULL;

	hex_data_len = 2 * strlen(cJSON_Print(data)) + 1;
	hex_data = calloc(hex_data_len, sizeof(char));
	str_to_hexstr(cJSON_Print(data), hex_data);

	currency_detail = cJSON_CreateObject();
	cJSON_AddStringToObject(currency_detail, "currency", "chips10sec");
	cJSON_AddNumberToObject(currency_detail, "amount", 0.0001);
	cJSON_AddStringToObject(currency_detail, "address", "cashiers.poker.chips10sec@");

	tx_params = cJSON_CreateArray();
	cJSON_AddItemToArray(tx_params, currency_detail);

	snprintf(params[0], arg_size, "\'*\'");
	snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(tx_params));
	snprintf(params[2], arg_size, "%d %f false", minconf, fee);
	snprintf(params[3], arg_size, "\'%s\'", hex_data);

	argc = 6;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "sendcurrency", params[0], params[1], params[2], params[3]);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	return argjson;
}

cJSON *getaddressutxos(char verus_addresses[][100], int n)
{
	int argc;
	char **argv = NULL, params[arg_size] = { 0 };
	cJSON *addresses = NULL, *addr_info = NULL, *argjson = NULL;

	addresses = cJSON_CreateArray();
	addr_info = cJSON_CreateObject();

	for (int32_t i = 0; i < n; i++) {
		cJSON_AddItemToArray(addresses, cJSON_CreateString(verus_addresses[i]));
	}

	cJSON_AddItemToObject(addr_info, "addresses", addresses);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(addr_info));

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getaddressutxos", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

struct table *decode_table_info_from_str(char *str)
{
	uint8_t *table_data = NULL;
	struct table *t = NULL;

	if (str == NULL)
		return NULL;

	table_data = calloc(1, (strlen(str) + 1) / 2);
	decode_hex(table_data, (strlen(str) + 1) / 2, str);

	t = calloc(1, sizeof(struct table));
	t = (struct table *)table_data;

end:
	return t;
}

struct table *decode_table_info(cJSON *dealer_cmm_data)
{
	char *str = NULL;
	uint8_t *table_data = NULL;
	struct table *t = NULL;

	if (!dealer_cmm_data)
		return NULL;

	str = jstr(cJSON_GetArrayItem(dealer_cmm_data, 0), get_vdxf_id(STRING_VDXF_ID));

	if (!str)
		return NULL;

	table_data = calloc(1, (strlen(str) + 1) / 2);
	decode_hex(table_data, (strlen(str) + 1) / 2, str);

	t = calloc(1, sizeof(struct table));
	t = (struct table *)table_data;

end:
	return t;
}

cJSON *get_available_t_of_d(char *dealer_id)
{
	int32_t game_state;
	cJSON *t_table_info = NULL;

	if (!dealer_id)
		return NULL;

	t_table_info = get_cJSON_from_id_key(dealer_id, T_TABLE_INFO_KEY);
	if (!t_table_info)
		return NULL;

	game_state = get_game_state(jstr(t_table_info, "table_id"));

	if ((game_state == G_TABLE_STARTED) && (!is_table_full(jstr(t_table_info, "table_id"))) &&
	    (!check_if_pa_exists(jstr(t_table_info, "table_id")))) {
		return t_table_info;
	}
	return NULL;
}

bool is_table_full(char *table_id)
{
	int32_t game_state;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL, *t_table_info = NULL;

	game_state = get_game_state(table_id);
	if (game_state == G_TABLE_STARTED) {
		game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);

		t_player_info =
			get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
		t_table_info =
			get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str));

		if ((t_player_info) && (t_table_info) &&
		    (jint(t_player_info, "num_players") >= jint(t_table_info, "max_players"))) {
			dlg_error("Table is full");
			return true;
		}
	}
	return false;
}

int32_t check_if_pa_exists(char *table_id)
{
	int32_t retval = OK;
	cJSON *pa_arr = NULL;

	pa_arr = cJSON_CreateArray();
	pa_arr = get_primaryaddresses(table_id, 0);
	if (pa_arr) {
		for (int32_t i = 0; i < cJSON_GetArraySize(pa_arr); i++) {
			if (0 == strcmp(jstri(pa_arr, i), player_config.primaryaddress)) {
				return !retval;
			}
		}
	}
	return retval;
}

bool check_if_enough_funds_avail(char *table_id)
{
	double balance = 0.0, min_stake = 0.0;
	char *game_id_str = NULL;
	cJSON *t_table_info = NULL;

	game_id_str = get_str_from_id_key(table_id, get_vdxf_id(T_GAME_ID_KEY));
	if (!game_id_str)
		false;

	t_table_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str));
	if (t_table_info) {
		min_stake = jdouble(t_table_info, "min_stake");
		balance = chips_get_balance();
		if (balance > min_stake + RESERVE_AMOUNT)
			return true;
	}
	dlg_error("Insufficient Funds");
	return false;
}

int32_t check_if_d_t_available(char *dealer_id, char *table_id, cJSON **t_table_info)
{
	int32_t retval = OK;
	int32_t game_state;

	if ((!dealer_id) || (!table_id) || (!is_dealer_exists(dealer_id)) || (!is_id_exists(table_id, 0))) {
		return ERR_CONFIG_PLAYER_ARGS;
	}

	/*
	* Check if the dealer added the table info to the dealers ID
	*/
	*t_table_info = get_cJSON_from_id_key(dealer_id, T_TABLE_INFO_KEY);
	if (*t_table_info == NULL) {
		return ERR_T_TABLE_INFO_NULL;
	}

	if ((0 == strcmp(jstr(*t_table_info, "table_id"), table_id))) {
		/*
		* Check if the table is started
		*/
		game_state = get_game_state(table_id);
		if (game_state != G_TABLE_STARTED) {
			return ERR_TABLE_IS_NOT_STARTED;
		}
		/*
		* Check if the table is full
		*/
		if (is_table_full(table_id)) {
			return ERR_TABLE_IS_FULL;
		}
		/*
		* Check is the Primary Address of the player join request is already been added to the table
		*/
		if (check_if_pa_exists(table_id)) {
			return ERR_PA_EXISTS;
		}
	}
	return retval;
}

/*
key --> Full key
*/
char *get_str_from_id_key(char *id, char *key)
{
	cJSON *cmm = NULL;

	cmm = get_cmm_key_data(id, 0, get_vdxf_id(key));
	if (cmm) {
		return jstr(cJSON_GetArrayItem(cmm, 0), get_vdxf_id(get_key_data_type(key)));
	}
	return NULL;
}

char *get_str_from_id_key_vdxfid(char *id, char *key_vdxfid)
{
	cJSON *cmm = NULL;

	cmm = get_cmm_key_data(id, 0, key_vdxfid);
	if (cmm) {
		return jstr(cJSON_GetArrayItem(cmm, 0), get_vdxf_id(get_key_data_type(key_vdxfid)));
	}
	return NULL;
}

cJSON *get_cJSON_from_id_key(char *id, char *key)
{
	cJSON *cmm = NULL;

	cmm = get_cmm_key_data(id, 0, get_vdxf_id(key));
	if (cmm) {
		return hex_cJSON(jstr(cJSON_GetArrayItem(cmm, 0), get_vdxf_id(get_key_data_type(key))));
	}
	return NULL;
}

cJSON *get_cJSON_from_table_id_key(char *table_id, char *key)
{
	char *game_id_str = NULL;
	cJSON *cmm = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str)
		return NULL;

	cmm = get_cmm_key_data(table_id, 0, get_key_data_vdxf_id(key, game_id_str));
	if (cmm) {
		return hex_cJSON(jstr(cJSON_GetArrayItem(cmm, 0), get_vdxf_id(get_key_data_type(key))));
	}
	return NULL;
}

cJSON *get_cJSON_from_id_key_vdxfid(char *id, char *key_vdxfid)
{
	cJSON *cmm = NULL;

	cmm = get_cmm_key_data(id, 0, key_vdxfid);
	if (cmm) {
		return hex_cJSON(jstr(cJSON_GetArrayItem(cmm, 0), get_vdxf_id(get_key_data_type(key_vdxfid))));
	}
	return NULL;
}

cJSON *append_cmm_from_id_key_data_hex(char *id, char *key, char *hex_data, bool is_key_vdxf_id)
{
	char *data_type = NULL, *data_key = NULL;
	cJSON *data_obj = NULL, *cmm_obj = NULL;

	cmm_obj = cJSON_CreateObject();
	cmm_obj = get_cmm(id, 0);

	//dlg_info("cmm_old::%s", cJSON_Print(cmm_obj));
	if (is_key_vdxf_id) {
		data_type = get_vdxf_id(BYTEVECTOR_VDXF_ID);
		data_key = key;
	} else {
		data_type = get_vdxf_id(get_key_data_type(key));
		data_key = get_vdxf_id(key);
	}

	data_obj = cJSON_CreateObject();
	jaddstr(data_obj, data_type, hex_data);
	//dlg_info("data::%s", cJSON_Print(data_obj));

	cJSON_DeleteItemFromObject(cmm_obj, data_key);
	cJSON_AddItemToObject(cmm_obj, data_key, data_obj);
	//dlg_info("cmm_new::%s", cJSON_Print(cmm_obj));

	return update_cmm(id, cmm_obj);
}

cJSON *append_cmm_from_id_key_data_cJSON(char *id, char *key, cJSON *data, bool is_key_vdxf_id)
{
	char *hex_data = NULL;

	if (!data)
		return NULL;

	cJSON_hex(data, &hex_data);
	if (!hex_data)
		return NULL;

	return append_cmm_from_id_key_data_hex(id, key, hex_data, is_key_vdxf_id);
}

cJSON *update_cmm_from_id_key_data_hex(char *id, char *key, char *hex_data, bool is_key_vdxf_id)
{
	char *data_type = NULL, *data_key = NULL;
	cJSON *data_obj = NULL, *data_key_obj = NULL;

	if (is_key_vdxf_id) {
		data_type = get_vdxf_id(BYTEVECTOR_VDXF_ID);
		data_key = key;
	} else {
		data_type = get_vdxf_id(get_key_data_type(key));
		data_key = get_vdxf_id(key);
	}
	data_obj = cJSON_CreateObject();
	jaddstr(data_obj, data_type, hex_data);

	data_key_obj = cJSON_CreateObject();
	cJSON_AddItemToObject(data_key_obj, data_key, data_obj);

	return update_cmm(id, data_key_obj);
}

cJSON *update_cmm_from_id_key_data_cJSON(char *id, char *key, cJSON *data, bool is_key_vdxf_id)
{
	char *hex_data = NULL;

	cJSON_hex(data, &hex_data);
	if (!hex_data) {
		dlg_error("%s::%d::Error occured in conversion of cJSON to HEX\n", __func__, __LINE__);
		return NULL;
	}
	return update_cmm_from_id_key_data_hex(id, key, hex_data, is_key_vdxf_id);
}

cJSON *get_t_player_info(char *table_id)
{
	cJSON *cmm_t_player_info = NULL, *t_player_info = NULL, *cmm = NULL;
	char *hexstr = NULL, *t_player_info_str = NULL;

	cmm = cJSON_CreateObject();
	cmm = get_cmm(table_id, 0);
	if (cmm) {
		cmm_t_player_info = cJSON_CreateObject();
		cmm_t_player_info = cJSON_GetObjectItem(cmm, get_vdxf_id(T_PLAYER_INFO_KEY));
		if (cmm_t_player_info) {
			hexstr = jstr(cJSON_GetArrayItem(cmm_t_player_info, 0), get_vdxf_id(STRING_VDXF_ID));
			t_player_info_str = calloc(1, strlen(hexstr));
			hexstr_to_str(hexstr, t_player_info_str);
			t_player_info = cJSON_CreateObject();
			t_player_info = cJSON_Parse(t_player_info_str);
		}
	}
	free(t_player_info_str);
	return t_player_info;
}

int32_t do_payin_tx_checks(char *txid, cJSON *payin_tx_data)
{
	int32_t retval = OK, game_state;
	double amount = 0;
	char pa[128] = { 0 }, *game_id_str = NULL, *table_id = NULL;
	cJSON *t_player_info = NULL, *player_info = NULL, *t_table_info = NULL;

	if ((!txid) || (!payin_tx_data))
		return ERR_NO_PAYIN_DATA;

	dlg_info("Payin TX Data ::%s", cJSON_Print(payin_tx_data));
	table_id = jstr(payin_tx_data, "table_id");

	/*
		Check is the table exists
	*/
	if (!is_id_exists(table_id, 0))
		return ERR_ID_NOT_FOUND;
	/*
		Check if the table is started, table is started by the dealer during dealer init.
	*/
	game_state = get_game_state(table_id);
	if (game_state != G_TABLE_STARTED)
		return ERR_INVALID_TABLE_STATE;
	/*
		Check whether if the table is FULL.
	*/
	if (is_table_full(table_id))
		return ERR_TABLE_IS_FULL;

	/*
		Check the amount of funds that the player deposited at Cashier and see if these funds are with in the range of [min-max] stake.
	*/
	amount = chips_get_balance_on_address_from_tx(get_vdxf_id(CASHIERS_ID), txid);
	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	t_table_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id_str));
	if ((amount < jdouble(t_table_info, "min_stake")) && (amount > jdouble(t_table_info, "max_stake"))) {
		dlg_error("funds deposited ::%f should be in the range %f::%f\n", amount,
			  jdouble(t_table_info, "min_stake"), jdouble(t_table_info, "max_stake"));
		return ERR_PAYIN_TX_INVALID_FUNDS;
	}

	/*
		Check whether if the player is already joined and make duplicate request, check if the players primary address is already been matching to any of the players joined.
	*/
	t_player_info = cJSON_CreateObject();
	t_player_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	if (!t_player_info)
		return OK; //Means no one joined yet

	player_info = cJSON_CreateArray();
	player_info = cJSON_GetObjectItem(t_player_info, "player_info");
	strncpy(pa, jstr(payin_tx_data, "primaryaddress"), sizeof(pa));
	for (int32_t i = 0; i < cJSON_GetArraySize(player_info); i++) {
		if ((strstr(jstri(player_info, i), pa)) && (strstr(jstri(player_info, i), txid))) {
			return ERR_DUP_PAYIN_UPDATE_REQ;
		} else if (strstr(jstri(player_info, i), pa)) {
			return ERR_PA_EXISTS;
		}
	}
	return retval;
}

static cJSON *compute_updated_t_player_info(char *txid, cJSON *payin_tx_data)
{
	int32_t num_players = 0;
	char *game_id_str = NULL, pa_tx_id[256] = { 0 };
	cJSON *t_player_info = NULL, *player_info = NULL, *updated_t_player_info = NULL;

	game_id_str = get_str_from_id_key_vdxfid(jstr(payin_tx_data, "table_id"), get_vdxf_id(T_GAME_ID_KEY));
	if (!game_id_str)
		return NULL;

	t_player_info = get_cJSON_from_id_key_vdxfid(jstr(payin_tx_data, "table_id"),
						     get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	player_info = cJSON_CreateArray();
	if (t_player_info) {
		num_players = jint(t_player_info, "num_players");
		player_info = cJSON_GetObjectItem(t_player_info, "player_info");
		if (!player_info)
			return NULL;
	}
	num_players++;
	sprintf(pa_tx_id, "%s_%s_%d", jstr(payin_tx_data, "primaryaddress"), txid, num_players);
	jaddistr(player_info, pa_tx_id);

	updated_t_player_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(updated_t_player_info, "num_players", num_players);
	cJSON_AddItemToObject(updated_t_player_info, "player_info", player_info);

	return updated_t_player_info;
}

int32_t process_payin_tx_data(char *txid, cJSON *payin_tx_data)
{
	int32_t retval = OK;
	char *game_id_str = NULL;
	cJSON *updated_t_player_info = NULL, *out = NULL;

	retval = do_payin_tx_checks(txid, payin_tx_data);
	if (retval == ERR_DUP_PAYIN_UPDATE_REQ) {
		dlg_warn("Duplicate update request");
		retval = OK;
		return retval;
	} else if (retval != OK) {
		/*
			Reverse the payin TX for any of the scenarios where player is unable to join the table.
		*/
		dlg_error("%s::%d::Err:: %s, Reversing the tx\n", __FUNCTION__, __LINE__, bet_err_str(retval));
		double amount = chips_get_balance_on_address_from_tx(get_vdxf_id(CASHIERS_ID), txid);
		cJSON *tx = chips_transfer_funds(amount, jstr(payin_tx_data, "primaryaddress"));
		dlg_info("%s::%d::Tx deposited back to the players primaryaddress::%s\n", __func__, __LINE__,
			 cJSON_Print(tx));
		return retval;
	}

	game_id_str = get_str_from_id_key_vdxfid(jstr(payin_tx_data, "table_id"), get_vdxf_id(T_GAME_ID_KEY));
	if (!game_id_str)
		return ERR_GAME_ID_NOT_FOUND;

	/*
		Prepare the cJSON object with the new player details
	*/
	updated_t_player_info = compute_updated_t_player_info(txid, payin_tx_data);
	if (!updated_t_player_info)
		return ERR_T_PLAYER_INFO_UPDATE;

	/*
		Update the t_table_info.<game_id> key of the table id with newly join requested player details.
	*/
	dlg_info("%s", cJSON_Print(updated_t_player_info));
	out = append_cmm_from_id_key_data_cJSON(jstr(payin_tx_data, "table_id"),
						get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str),
						updated_t_player_info, true);
	dlg_info("%s", cJSON_Print(out));
	/*
		Add the players primary address to the list of the primary addresses of the table id so that player can able to perform the updates to the table ID. 
	*/
	out = append_pa_to_cmm(jstr(payin_tx_data, "table_id"), jstr(payin_tx_data, "primaryaddress"));
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

void process_block(char *blockhash)
{
	int32_t blockcount = 0, retval = OK;
	char verus_addr[1][100] = { CASHIERS_ID };
	cJSON *blockjson = NULL, *payin_tx_data = NULL;

	blockjson = cJSON_CreateObject();
	blockjson = chips_get_block_from_block_hash(blockhash);

	if (blockjson == NULL)
		goto end;

	blockcount = jint(blockjson, "height");
	if (blockcount <= 0)
		goto end;

	dlg_info("received blockhash, found at height = %d", blockcount);
	cJSON *argjson = cJSON_CreateObject();
	argjson = getaddressutxos(verus_addr, 1);

	for (int32_t i = 0; i < cJSON_GetArraySize(argjson); i++) {
		if (jint(cJSON_GetArrayItem(argjson, i), "height") == blockcount) {
			dlg_info("tx_id::%s", jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			payin_tx_data = chips_extract_tx_data_in_JSON(jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			if (!payin_tx_data)
				continue;
			retval = process_payin_tx_data(jstr(cJSON_GetArrayItem(argjson, i), "txid"), payin_tx_data);
			if (retval != OK) {
				dlg_error("%s", bet_err_str(retval));
				retval = OK;
			}
		}
	}
end:
	dlg_info("Done\n");
}
