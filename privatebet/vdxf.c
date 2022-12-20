#include "bet.h"
#include "vdxf.h"
#include "commands.h"
#include "misc.h"
#include "err.h"

struct table player_t = { 0 };
char T_PLAYER_KEYS[9][128] = { T_PLAYER1_KEY, T_PLAYER2_KEY, T_PLAYER3_KEY, T_PLAYER4_KEY, T_PLAYER5_KEY,
			       T_PLAYER6_KEY, T_PLAYER7_KEY, T_PLAYER8_KEY, T_PLAYER9_KEY };

char *get_vdxf_id(char *key_name)
{
	int argc = 3;
	char full_key[128] = { 0 }, **argv = NULL;
	cJSON *argjson = NULL;

	strcpy(full_key, "chips.vrsc::poker.");
	strncat(full_key, key_name, strlen(key_name));

	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getvdxfid", full_key);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

	return jstr(argjson, "vdxfid");
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

	// This is temporary wait until we get an API to spend the tx in mempool
	while (!chips_is_mempool_empty()) {
		sleep(1);
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);
	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
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
	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params);

	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval != OK) {
		goto end;
	}

	cmm = cJSON_CreateObject();
	cmm = cJSON_GetObjectItem(cJSON_GetObjectItem(argjson, "identity"), "contentmultimap");

end:
	bet_dealloc_args(argc, &argv);
	return cmm;
}

cJSON *append_primaryaddresses(char *id, cJSON *primaryaddress)
{
	cJSON *id_info = NULL, *argjson = NULL, *pa = NULL, *final_pa = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 };

	if ((NULL == id) || (NULL == primaryaddress) || (NULL == verus_chips_cli)) {
		return NULL;
	}
	pa = get_primaryaddresses(id, 0);
	final_pa = cJSON_CreateArray();
	for (int32_t i = 0; i < cJSON_GetArraySize(pa); i++) {
		jaddistr(final_pa, jstri(pa, i));
	}
	int g_flag = 1;
	for (int32_t i = 0; i < cJSON_GetArraySize(primaryaddress); i++) {
		int flag = 1;
		for (int32_t j = 0; j < cJSON_GetArraySize(pa); j++) {
			if (strcmp(jstri(primaryaddress, i), jstri(pa, j)) == 0) {
				flag = 0;
				break;
			}
		}
		if (flag) {
			g_flag = 0;
			jaddistr(final_pa, jstri(primaryaddress, i));
		}
	}
	if (g_flag)
		return NULL;

	dlg_info("%s::%d::final_pa::%s\n", __FUNCTION__, __LINE__, cJSON_Print(final_pa));

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);
	cJSON_AddItemToObject(id_info, "primaryaddresses", final_pa);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	return argjson;
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
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);
	cJSON_AddItemToObject(id_info, "primaryaddresses", primaryaddress);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

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
	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params);

	argjson = cJSON_CreateObject();
	retval = make_command(argc, argv, &argjson);
	if (retval != OK) {
		dlg_error("%s::%d::%s\n", __func__, __LINE__, bet_err_str(retval));
		goto end;
	}

	id_obj = cJSON_GetObjectItem(argjson, "identity");
	pa = cJSON_CreateArray();
	pa = cJSON_GetObjectItem(id_obj, "primaryaddresses");

end:
	bet_dealloc_args(argc, &argv);
	return pa;
}

cJSON *get_cmm_key_data(char *id, int16_t full_id, char *key)
{
	cJSON *cmm = NULL, *cmm_key_data = NULL;

	if ((cmm = get_cmm(id, full_id)) == NULL)
		return NULL;

	cmm_key_data = cJSON_CreateObject();
	if ((cmm_key_data = cJSON_GetObjectItem(cmm, key)) == NULL)
		return NULL;

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

cJSON *update_t_table_info(char *dealer_id, char *key, struct table t)
{
	uint8_t *byte_arr = NULL;
	char hexstr[arg_size];
	cJSON *dealer_cmm = NULL, *dealer_cmm_key = NULL, *out = NULL;

	byte_arr = calloc(1, sizeof(t));
	struct_to_byte_arr(&t, sizeof(t), byte_arr);

	init_hexbytes_noT(hexstr, byte_arr, sizeof(t));

	dealer_cmm = cJSON_CreateObject();
	cJSON_AddStringToObject(dealer_cmm, STRING_VDXF_ID, hexstr);

	dealer_cmm_key = cJSON_CreateObject();
	cJSON_AddItemToObject(dealer_cmm_key, key, dealer_cmm);

	out = cJSON_CreateObject();
	out = update_cmm(dealer_id, dealer_cmm_key);

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
	dealer_cmm_data = get_cmm_key_data(dealer_id, 0, T_TABLE_INFO_KEY);
	if (dealer_cmm_data == NULL) {
		dlg_info("%s::%d::The key ::%s is not found in the cmm of id ::%s\n", __FUNCTION__, __LINE__,
			 T_TABLE_INFO_KEY, dealer_id);
		goto end;
	}

	//TODO:Right now we dealing with single table, when multi table support comes,
	// we need to make checks whether the table with the specific name exists or not.

	str = jstr(cJSON_GetArrayItem(dealer_cmm_data, 0), STRING_VDXF_ID);

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
	cashier_cmm_data = get_cmm_key_data(cashier_id, 0, CASHIERS_KEY);

end:
	return cashier_cmm_data;
}

cJSON *update_cashiers(char *ip)
{
	cJSON *cashiers_info = NULL, *out = NULL, *ip_obj = NULL, *cashier_ips = NULL;

	cashiers_info = cJSON_CreateObject();
	cashier_ips = get_cashiers_info("cashiers");

	ip_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(ip_obj, STRING_VDXF_ID, ip);

	if (NULL == cashier_ips)
		cashier_ips = cJSON_CreateArray();
	else {
		for (int i = 0; i < cJSON_GetArraySize(cashier_ips); i++) {
			if (0 == strcmp(jstr(cJSON_GetArrayItem(cashier_ips, i), STRING_VDXF_ID), ip)) {
				dlg_info("%s::%d::The latest data of this ID contains this %s\n", __FUNCTION__,
					 __LINE__, ip);
				goto end;
			}
		}
	}

	cJSON_AddItemToArray(cashier_ips, ip_obj);
	cJSON_AddItemToObject(cashiers_info, CASHIERS_KEY, cashier_ips);
	out = update_cmm("cashiers", cashiers_info);

end:
	return out;
}

cJSON *get_dealers()
{
	cJSON *dealers_cmm = NULL, *dealer_ids = NULL;

	dealers_cmm = cJSON_CreateObject();
	dealers_cmm = get_cmm_key_data("dealers", 0, DEALERS_KEY);

	dealer_ids = cJSON_CreateArray();
	for (int32_t i = 0; i < cJSON_GetArraySize(dealers_cmm); i++) {
		cJSON_AddItemToArray(dealer_ids,
				     cJSON_GetObjectItem(cJSON_GetArrayItem(dealers_cmm, i), STRING_VDXF_ID));
	}
	return dealer_ids;
}

bool is_dealer_exists(char *dealer_id)
{
	cJSON *dealer_ids = NULL;
	bool dealer_exists = false;

	if ((dealer_id == NULL) || (strlen(dealer_id) == 0)) {
		return dealer_exists;
	}

	dealer_ids = cJSON_CreateArray();
	dealer_ids = get_dealers();
	if (dealer_ids == NULL) {
		return dealer_exists;
	}

	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
		if (0 == strcmp(dealer_id, jstri(dealer_ids, i))) {
			dealer_exists = true;
			break;
		}
	}
	return dealer_exists;
}

int32_t get_player_id(int *player_id)
{
	int32_t retval = ERR_PLAYER_NOT_EXISTS;
	cJSON *t_player_info = NULL, *player_info = NULL;

	t_player_info = get_t_player_info(player_config.table_id);
	if (t_player_info == NULL) {
		retval = ERR_T_PLAYER_INFO_NULL;
		goto end;
	}
	player_info = cJSON_CreateArray();
	player_info = jobj(t_player_info, "player_info");
	if (player_info == NULL) {
		retval = ERR_T_PLAYER_INFO_CORRUPTED;
		goto end;
	}
	for (int32_t i = 0; i < cJSON_GetArraySize(player_info); i++) {
		if (strncmp(player_config.primaryaddress, jstri(player_info, i),
			    strlen(player_config.primaryaddress)) == 0) {
			strtok(jstri(player_info, i), "_");
			strtok(NULL, "_");
			*player_id = atoi(strtok(NULL, "_"));
			dlg_info("%s::%d::player id::%d\n", __func__, __LINE__, *player_id);
			retval = OK;
			break;
		}
	}

end:
	return retval;
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
				op_id_info = get_z_getoperationstatus(jstr(op_id, "op_id"));
				sleep(1);
			}
			if (0 != strcmp(jstr(jitem(op_id_info, 0), "status"), "success")) {
				retval = ERR_SENDCURRENCY;
				dlg_error("%s::%d:: sendcurrency operation is not success\n", __func__, __LINE__);
				goto end;
			}
			char *txid = jstr(jobj(jitem(op_id_info, 0), "result"), "txid");
			dlg_info("%s::%d::payin_tx::%s\n", __FUNCTION__, __LINE__, txid);
			if ((retval = check_player_join_status(player_config.table_id, player_config.primaryaddress)) !=
			    OK) {
				dlg_info("%s::%d::%s\n", __func__, __LINE__, bet_err_str(retval));
			}
		}
	}
end:
	return retval;
}

int32_t get_table()
{
	int32_t retval = OK;
	struct table *t = NULL;
	cJSON *dealer_ids = NULL;

	if (check_if_d_t_available(player_config.dealer_id, player_config.table_id)) {
		t = get_t_table_info(player_config.dealer_id);
		memcpy((void *)&player_t, (void *)t, sizeof(player_t));
		return retval;
	} else {
		dealer_ids = cJSON_CreateArray();
		dealer_ids = get_dealers();
		if (dealer_ids == NULL) {
			return ERR_NO_DEALERS_FOUND;
		}
		for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
			t = get_available_t_of_d(jstri(dealer_ids, i));
			if (t) {
				strncpy(player_config.dealer_id, jstri(dealer_ids, i), sizeof(player_config.dealer_id));
				strncpy(player_config.table_id, t->table_id, sizeof(player_config.table_id));
				memcpy((void *)&player_t, (void *)t, sizeof(player_t));
				return retval;
			}
		}
	}
	retval = ERR_NO_TABLES_FOUND;
	return retval;
}

int32_t find_table()
{
	cJSON *dealer_ids = NULL;
	struct table *t = NULL;
	int32_t retval = OK;

	//TODO: check for pa, check for funds, check for num_players
	//If the user didn't configured any dealer_id, then take the first dealer id available.
	if ((retval = get_table()) != OK) {
		return retval;
	}
	dlg_info(
		"%s::%d::Table_info:: max_players :: %d,  big_blind :: %f, min_stake :: %f, max_stake :: %f, table_id :: %s, dealer_id :: %s\n",
		__func__, __LINE__, player_t.max_players, uint32_s_to_float(player_t.big_blind),
		uint32_s_to_float(player_t.min_stake), uint32_s_to_float(player_t.max_stake), player_t.table_id,
		player_t.dealer_id);

end:
	return retval;
}

int32_t is_id_exists(char *id, int16_t full_id)
{
	int32_t argc = 3, retval = 1;
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

	bet_dealloc_args(argc, &argv);
	return retval;
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

struct table *decode_table_info(cJSON *dealer_cmm_data)
{
	char *str = NULL;
	uint8_t *table_data = NULL;
	struct table *t = NULL;

	str = jstr(cJSON_GetArrayItem(dealer_cmm_data, 0), STRING_VDXF_ID);

	table_data = calloc(1, (strlen(str) + 1) / 2);
	decode_hex(table_data, (strlen(str) + 1) / 2, str);

	t = calloc(1, sizeof(struct table));
	t = (struct table *)table_data;

end:
	return t;
}

struct table *get_available_t_of_d(char *dealer_id)
{
	int32_t retval = OK;
	struct table *t = NULL;
	cJSON *t_player_info = NULL;

	if (NULL == dealer_id) {
		return NULL;
	}
	t = get_t_table_info(dealer_id);
	if (t) {
		t_player_info = get_t_player_info(t->table_id);
		if ((t_player_info) && (jint(t_player_info, "num_players") < t->max_players) &&
		    (!check_if_pa_exists(t->table_id)) && (check_if_enough_funds_avail(t->table_id))) {
			return t;
		}
	}
	return NULL;
}

int32_t check_if_pa_exists(char *table_id)
{
	int32_t retval = 0;
	cJSON *pa_arr = NULL;

	pa_arr = cJSON_CreateArray();
	pa_arr = get_primaryaddresses(table_id, 0);
	if (pa_arr) {
		for (int32_t i = 0; i < cJSON_GetArraySize(pa_arr); i++) {
			if (0 == strcmp(jstri(pa_arr, i), player_config.primaryaddress)) {
				dlg_error("%s::%d::Primaryaddress already exists\n", __func__, __LINE__);
				return !retval;
			}
		}
	}
	return retval;
}

bool check_if_enough_funds_avail(char *table_id)
{
	double balance = 0.0, min_stake = 0.0;
	struct table *t = NULL;

	t = get_t_table_info(table_id);
	if (t) {
		min_stake = uint32_s_to_float(t->min_stake);
		balance = chips_get_balance();
		if (balance > min_stake + RESERVE_AMOUNT)
			return true;
	}
	dlg_error("%s::%d::Insufficient Funds\n", __func__, __LINE__);
	return false;
}

bool check_if_d_t_available(char *dealer_id, char *table_id)
{
	int32_t err_no = OK;
	struct table *t = NULL;
	cJSON *t_player_info = NULL;

	if ((NULL == dealer_id) || (NULL == table_id)) {
		return false;
	}
	if (is_dealer_exists(dealer_id)) {
		t = get_t_table_info(dealer_id);
		if ((t) && (0 == strcmp(t->table_id, table_id))) {
			t_player_info = get_t_player_info(t->table_id);
			if (t_player_info == NULL) {
				return true; //Table is available but empty.
			}
			if ((t_player_info) && (jint(t_player_info, "num_players") < t->max_players) &&
			    (!check_if_pa_exists(t->table_id)) && (check_if_enough_funds_avail(t->table_id))) {
				return true; //Table is available but some spots are free to join.
			}
		}
	}
	return false;
}

cJSON *get_t_playerx(char *id, char *key)
{
	cJSON *cmm = NULL;

	cmm = get_cmm_key_data(id, 0, get_vdxf_id(key));
	if (cmm) {
		return hex_cJSON(jstr(cJSON_GetArrayItem(cmm, 0), BYTEVECTOR_VDXF_ID));
	}
	return NULL;
}

struct table *get_t_table_info(char *id)
{
	cJSON *t_table_info = NULL;
	struct table *t = NULL;

	t_table_info = get_cmm_key_data(id, 0, T_TABLE_INFO_KEY);
	if (t_table_info) {
		t = decode_table_info(t_table_info);
	}
	return t;
}

cJSON *get_t_player_info(char *table_id)
{
	cJSON *cmm_t_player_info = NULL, *t_player_info = NULL, *cmm = NULL;
	char *hexstr = NULL, *t_player_info_str = NULL;

	cmm = cJSON_CreateObject();
	cmm = get_cmm(table_id, 0);
	if (cmm) {
		cmm_t_player_info = cJSON_CreateObject();
		cmm_t_player_info = cJSON_GetObjectItem(cmm, T_PLAYER_INFO_KEY);
		if (cmm_t_player_info) {
			hexstr = jstr(cJSON_GetArrayItem(cmm_t_player_info, 0), STRING_VDXF_ID);
			t_player_info_str = calloc(1, strlen(hexstr));
			hexstr_to_str(hexstr, t_player_info_str);
			t_player_info = cJSON_CreateObject();
			t_player_info = cJSON_Parse(t_player_info_str);
		}
	}
	free(t_player_info_str);
	return t_player_info;
}

cJSON *update_t_player_info(char *id, cJSON *t_player_info)
{
	cJSON *id_info = NULL, *argjson = NULL, *cmm = NULL, *player_info = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 }, *hexstr = NULL;

	if ((NULL == id) || (NULL == t_player_info) || (NULL == verus_chips_cli)) {
		return NULL;
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);

	cJSON_hex(t_player_info, &hexstr);
	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, STRING_VDXF_ID, hexstr);

	cmm = cJSON_CreateObject();
	cJSON_AddItemToObject(cmm, T_PLAYER_INFO_KEY, player_info);
	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

static cJSON *update_t_player_info_pa(char *id, cJSON *t_player_info, cJSON *primaryaddresses)
{
	cJSON *id_info = NULL, *argjson = NULL, *cmm = NULL, *player_info = NULL, *t_table_info = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 }, *hexstr = NULL;

	if ((NULL == id) || (NULL == t_player_info) || (NULL == verus_chips_cli)) {
		return NULL;
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);

	cJSON_hex(t_player_info, &hexstr);
	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, STRING_VDXF_ID, hexstr);

	cmm = cJSON_CreateObject();
	cJSON_AddItemToObject(cmm, T_PLAYER_INFO_KEY, player_info);

	/*
		Reupdating t_table_info
	*/
	t_table_info = get_cmm_key_data(id, 0, T_TABLE_INFO_KEY);
	if (t_table_info) {
		cJSON_AddItemToObject(cmm, T_TABLE_INFO_KEY, t_table_info);
	}

	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);
	cJSON_AddItemToObject(id_info, "primaryaddresses", primaryaddresses);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

int32_t do_payin_tx_checks(cJSON *payin_tx_data, char *txid)
{
	int32_t retval = OK;
	double amount = 0;
	char pa_tx_hash[10] = { 0 }, pa[128] = { 0 };
	cJSON *t_table_info = NULL, *primaryaddresses = NULL, *t_player_info = NULL, *player_info = NULL;
	struct table *t = NULL;

	if ((!txid) || (!payin_tx_data)) {
		retval = 0;
		goto end;
	}

	amount = chips_get_balance_on_address_from_tx(VDXF_CASHIERS_ID, txid);
	t_table_info = cJSON_CreateObject();
	t_table_info = get_cmm_key_data(jstr(payin_tx_data, "table_id"), 0, T_TABLE_INFO_KEY);
	if (t_table_info == NULL) {
		retval = ERR_T_TABLE_INFO_NULL;
		goto end;
	}
	t = decode_table_info(t_table_info);
	if (t == NULL) {
		retval = ERR_TABLE_DECODING_FAILED;
		retval = 0;
		goto end;
	}
	if ((amount < uint32_s_to_float(t->min_stake)) && (amount > uint32_s_to_float(t->max_stake))) {
		retval = ERR_PAYIN_TX_INVALID_FUNDS;
		dlg_error(
			"%s::%d::Checks on funds deposit is failed, funds deposited ::%f should be in the range %f::%f\n",
			__FUNCTION__, __LINE__, amount, uint32_s_to_float(t->min_stake),
			uint32_s_to_float(t->max_stake));
		goto end;
	}
	t_player_info = cJSON_CreateObject();
	t_player_info = get_t_player_info(jstr(payin_tx_data, "table_id"));
	if (t_player_info) {
		if (jint(t_player_info, "num_players") >= t->max_players) {
			dlg_error("%s::%d::Table ::%s is full\n", __FUNCTION__, __LINE__,
				  jstr(payin_tx_data, "table_id"));
			retval = ERR_TABLE_IS_FULL;
			goto end;
		}
		player_info = cJSON_CreateArray();
		player_info = cJSON_GetObjectItem(t_player_info, "player_info");
		strncpy(pa, jstr(payin_tx_data, "primaryaddress"), sizeof(pa));
		for (int32_t i = 0; i < cJSON_GetArraySize(player_info); i++) {
			if (0 == strncmp(jstri(player_info, i), pa, strlen(pa))) {
				if (strtok(jstri(player_info, i), "_")) {
					strcpy(pa_tx_hash, strtok(NULL, "_"));
					if (strncmp(pa_tx_hash, txid, strlen(pa_tx_hash)) == 0) {
						dlg_warn("%s::%d::Duplicate update request\n", __FUNCTION__, __LINE__);
						retval = OK;
						break;
					} else {
						retval = ERR_PA_EXISTS;
						break;
					}
				} else {
					retval = ERR_WRONG_PA_TX_ID_FORMAT;
					break;
				}
			}
		}
	}

end:
	return retval;
}

static cJSON *add_player_t_player_info(char *txid, cJSON *payin_tx_data)
{
	int32_t num_players = 0;
	char hash[10] = { 0 }, pa_tx_hash[128] = { 0 };
	cJSON *t_player_info = NULL, *player_info = NULL, *updated_t_player_info = NULL;

	updated_t_player_info = cJSON_CreateObject();
	player_info = cJSON_CreateArray();
	t_player_info = get_t_player_info(jstr(payin_tx_data, "table_id"));
	if (t_player_info) {
		dlg_info("%s::%d::t_player_info::%s\n", __func__, __LINE__, cJSON_Print(t_player_info));
		num_players = jint(t_player_info, "num_players");
		player_info = cJSON_GetObjectItem(t_player_info, "player_info");
		if (player_info == NULL) {
			dlg_error("%s::%d::Players info was not updated properly\n", __FUNCTION__, __LINE__);
		}
	}
	num_players++;
	strncpy(hash, txid, 4);
	sprintf(pa_tx_hash, "%s_%s_%d", jstr(payin_tx_data, "primaryaddress"), hash, num_players);
	if (player_info == NULL)
		player_info = cJSON_CreateArray();
	jaddistr(player_info, pa_tx_hash);
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(player_info));
	cJSON_AddNumberToObject(updated_t_player_info, "num_players", num_players);
	cJSON_AddItemToObject(updated_t_player_info, "player_info", player_info);

	dlg_info("%s::%d::updated_player_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(updated_t_player_info));
	return updated_t_player_info;
}

void test_loop(char *blockhash)
{
	dlg_info("%s called!", __FUNCTION__);
	char verus_addr[1][100] = { CASHIERS_ID };
	int32_t blockcount = 0, retval = OK;
	cJSON *blockjson = NULL, *t_player_info = NULL, *primaryaddress = NULL, *payin_tx_data = NULL;

	blockjson = cJSON_CreateObject();
	blockjson = chips_get_block_from_block_hash(blockhash);

	if (blockjson == NULL)
		goto end;

	blockcount = jint(blockjson, "height");
	if (blockcount <= 0)
		goto end;

	dlg_info("%s: received blockhash in test_loop, found at height = %d", __FUNCTION__, blockcount);
	cJSON *argjson = cJSON_CreateObject();
	argjson = getaddressutxos(verus_addr, 1);

	t_player_info = cJSON_CreateObject();
	for (int32_t i = 0; i < cJSON_GetArraySize(argjson); i++) {
		if (jint(cJSON_GetArrayItem(argjson, i), "height") == blockcount) {
			dlg_info("%s::%d::tx_id::%s\n", __FUNCTION__, __LINE__,
				 jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			payin_tx_data = chips_extract_tx_data_in_JSON(jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			if (payin_tx_data == NULL) {
				goto end;
			}
			retval = do_payin_tx_checks(payin_tx_data, jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			if (retval != OK) {
				dlg_error("%s::%d::Err:: %s, Reversing the tx\n", __FUNCTION__, __LINE__,
					  bet_err_str(retval));
				double amount = chips_get_balance_on_address_from_tx(
					VDXF_CASHIERS_ID, jstr(cJSON_GetArrayItem(argjson, i), "txid"));
				cJSON *tx = chips_transfer_funds(amount, jstr(payin_tx_data, "primaryaddress"));
				dlg_warn("%s::%d::Tx deposited back to the players primaryaddress::%s\n", __func__,
					 __LINE__, cJSON_Print(tx));
				goto end;
			}
			cJSON *updated_player_info =
				add_player_t_player_info(jstr(cJSON_GetArrayItem(argjson, i), "txid"), payin_tx_data);

			primaryaddress = cJSON_CreateArray();
			primaryaddress = get_primaryaddresses(jstr(payin_tx_data, "table_id"), 0);

			cJSON *t_pa = cJSON_CreateArray();
			for (int32_t i = 0; i < cJSON_GetArraySize(primaryaddress); i++) {
				jaddistr(t_pa, jstri(primaryaddress, i));
			}
			jaddistr(t_pa, jstr(payin_tx_data, "primaryaddress"));
			dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(t_pa));

			cJSON *temp1 =
				update_t_player_info_pa(jstr(payin_tx_data, "table_id"), updated_player_info, t_pa);
			dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp1));
		}
	}
end:
	dlg_info("at end, do nothing atm");
}
