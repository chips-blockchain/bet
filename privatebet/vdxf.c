#include "bet.h"
#include "vdxf.h"
#include "commands.h"
#include "misc.h"
#include "err.h"

struct table player_t = { 0 };

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
	int argc;
	char **argv = NULL;
	char params[128] = { 0 };
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
	make_command(argc, argv, &argjson);

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
	int argc;
	char **argv = NULL;
	char params[128] = { 0 };
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
	make_command(argc, argv, &argjson);

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

	cmm = get_cmm(id, full_id);

	if (NULL == cmm) {
		dlg_info("%s::%d::cmm for id::%s is null", __FUNCTION__, __LINE__, id);
		return NULL;
	}
	cmm_key_data = cJSON_CreateObject();
	cmm_key_data = cJSON_GetObjectItem(cmm, key);
	if (NULL == cmm_key_data) {
		dlg_info("%s::%d:: The data of key ::%s for the id::%s is null", __FUNCTION__, __LINE__, key, id);
	}
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
			dlg_info("%s::%d::The preferred dealer id exists::%s\n", __FUNCTION__, __LINE__,
				 jstri(dealer_ids, i));
			dealer_exists = true;
			break;
		}
	}
	return dealer_exists;
}

int32_t find_table()
{
	cJSON *dealer_ids = NULL;
	struct table *t = NULL;
	int32_t retval = OK;

	//If the user didn't configured any dealer_id, then take the first dealer id available.
	if (!is_dealer_exists(player_config.dealer_id)) {
		dealer_ids = cJSON_CreateArray();
		dealer_ids = get_dealers();
		if (dealer_ids == NULL) {
			dlg_error("%s::%d::No dealers found\n", __FUNCTION__, __LINE__);
			retval = ERR_NO_DEALERS_FOUND;
			goto end;
		}
		for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
			//TODO: Need to check if the dealer tables are empty or not.
			strncpy(player_config.dealer_id, jstri(dealer_ids, i), sizeof(player_config.dealer_id));
			dlg_info("%s::%d::Dealer chosen::%s\n", __FUNCTION__, __LINE__, player_config.dealer_id);
			break;
		}
	}
	t = get_dealers_config_table(player_config.dealer_id, player_config.table_id);
	if (t == NULL) {
		retval = ERR_NO_TABLES_FOUND;
		goto end;
	}
	memcpy((void *)&player_t, (void *)t, sizeof(player_t));

	dlg_info(
		"max_players :: %d,  big_blind :: %f, min_stake :: %f, max_stake :: %f, table_id :: %s, dealer_id :: %s\n",
		player_t.max_players, uint32_s_to_float(player_t.big_blind), uint32_s_to_float(player_t.min_stake),
		uint32_s_to_float(player_t.max_stake), player_t.table_id, player_t.dealer_id);

end:
	return retval;
}

bool is_id_exists(char *id, int16_t full_id)
{
	int argc = 3, retval = 1;
	char **argv = NULL;
	char params[128] = { 0 };
	cJSON *argjson = NULL, *out = NULL;

	strncpy(params, id, strlen(id));
	if (0 == full_id) {
		strcat(params, ".poker.chips10sec@");
	}
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

	if (NULL == cJSON_GetObjectItem(argjson, "identity")) {
		retval = !retval;
	}
	return retval;
}

void verus_sendcurrency_data(cJSON *data)
{
	int32_t hex_data_len, argc, minconf = 1;
	double fee = 0.0001;
	char *hex_data = NULL, **argv = NULL, params[4][arg_size] = { 0 };
	cJSON *currency_detail = NULL, *argjson = NULL;

	hex_data_len = 2 * strlen(cJSON_Print(data)) + 1;
	hex_data = calloc(hex_data_len, sizeof(char));
	str_to_hexstr(cJSON_Print(data), hex_data);

	currency_detail = cJSON_CreateObject();
	cJSON_AddStringToObject(currency_detail, "currency", "chips10sec");
	cJSON_AddNumberToObject(currency_detail, "amount", 0.0001);
	cJSON_AddStringToObject(currency_detail, "address", "cashiers.poker.chips10sec@");

	cJSON *temp = cJSON_CreateArray();
	cJSON_AddItemToArray(temp, currency_detail);

	dlg_info("%s::%d::%s", __FUNCTION__, __LINE__, cJSON_Print(temp));

	snprintf(params[0], arg_size, "\'*\'");
	snprintf(params[1], arg_size, "\'%s\'", cJSON_Print(temp));
	snprintf(params[2], arg_size, "%d %f false", minconf, fee);
	snprintf(params[3], arg_size, "\'%s\'", hex_data);

	argc = 6;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "sendcurrency", params[0], params[1], params[2], params[3]);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
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

static cJSON *get_t_player_info(char *table_id)
{
	cJSON *t_player_info = NULL, *player_info = NULL, *cmm = NULL;
	char *hexstr = NULL, *t_player_info_str = NULL;

	cmm = cJSON_CreateObject();
	cmm = get_cmm(table_id, 0);
	if (cmm) {
		t_player_info = cJSON_CreateObject();
		t_player_info = cJSON_GetObjectItem(cmm, T_PLAYER_INFO_KEY);
		if (t_player_info) {
			hexstr = jstr(cJSON_GetArrayItem(t_player_info, 0), STRING_VDXF_ID);
			t_player_info_str = calloc(1, strlen(hexstr));
			hexstr_to_str(hexstr, t_player_info_str);
			player_info = cJSON_CreateObject();
			player_info = cJSON_Parse(t_player_info_str);
		}
	}
	dlg_info("%s::%d::t_player_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(player_info));
	free(t_player_info_str);
	return player_info;
}

static cJSON *update_t_player_info(char *id, cJSON *t_player_info)
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
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
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

	
	dlg_info("%s::%d::t_player_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(t_player_info));	
	cJSON_hex(t_player_info, &hexstr);
	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, STRING_VDXF_ID, hexstr);

	cmm = cJSON_CreateObject();
	cJSON_AddItemToObject(cmm, T_PLAYER_INFO_KEY, player_info);

	dlg_info("%s::%d::cmm::%s\n", __FUNCTION__, __LINE__, cJSON_Print(cmm));	
	/*
		Reupdating t_table_info
	*/
	t_table_info = get_cmm_key_data(id,0,T_TABLE_INFO_KEY);
	if(t_table_info) {
		cJSON_AddItemToObject(cmm, T_TABLE_INFO_KEY, t_table_info);		
	}

	dlg_info("%s::%d::cmm::%s\n", __FUNCTION__, __LINE__, cJSON_Print(cmm));	

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
	dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	return argjson;
}

int32_t do_payin_tx_checks(cJSON *payin_tx_data, char *txid)
{
	int32_t retval = 1;
	double amount = 0;
	char pa_tx_hash[10] = { 0 }, pa[128] = {0};
	cJSON *t_table_info = NULL, *primaryaddresses = NULL, *t_player_info = NULL, *pa_arr = NULL;
	struct table *t = NULL;

	if ((!txid) || (!payin_tx_data)) {
		retval = 0;
		goto end;
	}

	amount = chips_get_balance_on_address_from_tx(VDXF_CASHIERS_ID, txid);
	t_table_info = cJSON_CreateObject();
	t_table_info = get_cmm_key_data(jstr(payin_tx_data, "table_id"), 0, T_TABLE_INFO_KEY);
	if (t_table_info == NULL) {
		dlg_error("%s::%d::The info of table %s of key %s is not avaialble\n", __FUNCTION__, __LINE__,
			  jstr(payin_tx_data, "table_id"), T_TABLE_INFO_KEY);
		retval = 0;
		goto end;
	}
	t = decode_table_info(t_table_info);
	if (t == NULL) {
		dlg_error("%s::%d::Decoding of table info into table struct is failed\n", __FUNCTION__, __LINE__);
		retval = 0;
		goto end;
	}
	if ((amount < uint32_s_to_float(t->min_stake)) && (amount > uint32_s_to_float(t->max_stake))) {
		retval = 0;
		dlg_error(
			"%s::%d::Checks on funds deposit is failed, funds deposited ::%f should be in the range %f::%f\n",
			__FUNCTION__, __LINE__, amount, uint32_s_to_float(t->min_stake),
			uint32_s_to_float(t->max_stake));
		goto end;
	}
	t_player_info = cJSON_CreateObject();
	t_player_info = get_cmm_key_data(jstr(payin_tx_data, "table_id"), 0, T_PLAYER_INFO_KEY);
	if(t_player_info) {
		dlg_info("%s::%d::Players joined so far::%d\n", __FUNCTION__,__LINE__,jint(t_player_info,"num_players"));
		if(jint(t_player_info,"num_players")>t->max_players) {
			dlg_error("%s::%d::Table ::%s is full\n", __FUNCTION__, __LINE__,jstr(payin_tx_data, "table_id"));
			retval = 0;
			goto end;
		}
		
		pa_arr = cJSON_CreateArray();
		pa_arr = cJSON_GetObjectItem(pa_arr, "player_info");
		strncpy(pa, jstr(payin_tx_data,"primaryaddress"), sizeof(pa));
		for(int32_t i=0; i<cJSON_GetArraySize(pa_arr); i++) {
			if(0 == strncmp(jstri(pa_arr,i),pa,strlen(pa))) {
				dlg_info("%s::%d ::Primaryaddress is exists on the ID %s::%s\n", __FUNCTION__, __LINE__, jstri(pa_arr, i),pa);
				retval =0;
				goto end;
				
			}
		}
		
	}
	
	
	#if 0
	primaryaddresses = get_primaryaddresses(jstr(payin_tx_data, "table_id"), 0);
	if (primaryaddresses) {
		for (int32_t i = 0; i < cJSON_GetArraySize(primaryaddresses); i++) {
			if (strncmp(jstri(primaryaddresses, i), jstr(payin_tx_data, "primaryaddress"),
				    strlen(jstr(payin_tx_data, "primaryaddress"))) == 0) {
				dlg_info("%s::%d ::%s::%s\n", __FUNCTION__, __LINE__, jstri(primaryaddresses, i),
					 jstr(payin_tx_data, "primaryaddress"));
				if (strtok(jstri(primaryaddresses, i), "_")) {
					strcpy(pa_tx_hash, strtok(NULL, "_"));
					dlg_info("%s::%d ::%s::%s\n", __FUNCTION__, __LINE__, pa_tx_hash, txid);
					if (strncmp(pa_tx_hash, txid, strlen(pa_tx_hash)) == 0) {
						dlg_warn("%s::%d::This tx details are already updated\n", __FUNCTION__,
							 __LINE__);
						retval = 2; // Do nothing
						break;
					} else {
						retval = 0; //
						dlg_error("%s::%d::The primaryaddress is already exists\n",
							  __FUNCTION__, __LINE__);
						break;
					}
				} else {
					retval = 0;
					dlg_error("%s::%d::Probably the format of pa::%s might be wrong\n",
						  __FUNCTION__, __LINE__, jstri(primaryaddresses, i));
					break;
				}
			}
		}
	}
	#endif
end:
	return retval;
}

void test_loop(char *blockhash)
{
	dlg_info("%s called!", __FUNCTION__);
	char verus_addr[1][100] = { CASHIERS_ID }, pa_tx_hash[128] = { 0 };
	int32_t blockcount = 0, retval = 0;
	cJSON *blockjson = NULL, *t_player_info = NULL, *primaryaddress = NULL;

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
			cJSON *payin_tx_data =
				chips_extract_tx_data_in_JSON(jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			dlg_info("%s::%d::tx_data::%s\n", __FUNCTION__, __LINE__, cJSON_Print(payin_tx_data));
			retval = do_payin_tx_checks(payin_tx_data, jstr(cJSON_GetArrayItem(argjson, i), "txid"));
			if (retval == 0) {
				dlg_info("%s::%d::Checks on player payin_tx got failed\n", __FUNCTION__, __LINE__);
				goto end;
			}



			


			
			t_player_info = get_t_player_info(jstr(payin_tx_data, "table_id"));
			//get_cmm_key_data(jstr(payin_tx_data, "table_id"), 0, T_PLAYER_INFO_KEY);
			cJSON *updated_player_info = cJSON_CreateObject();
			cJSON *player_info = cJSON_CreateArray();
			int32_t num_players = 0;
			if(t_player_info) {
				dlg_info("%s::%d::t_player_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(t_player_info));
				num_players = jint(t_player_info, "num_players");
				player_info = cJSON_GetObjectItem(t_player_info,"player_info");				
				if(player_info == NULL){
					dlg_error("%s::%d::Players info was not updated properly\n", __FUNCTION__, __LINE__);
				}
			}
			num_players++;
			char hash[10] = { 0 };
			strncpy(hash, jstr(cJSON_GetArrayItem(argjson, i), "txid"), 4);
			sprintf(pa_tx_hash, "%s_%s_%d", jstr(payin_tx_data, "primaryaddress"), hash, num_players);
			if(player_info == NULL)
				player_info = cJSON_CreateArray();
			jaddistr(player_info, pa_tx_hash);
			dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(player_info));
			cJSON_AddNumberToObject(updated_player_info,"num_players",num_players);
			cJSON_AddItemToObject(updated_player_info,"player_info",player_info);

			dlg_info("%s::%d::updated_player_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(updated_player_info));

			primaryaddress = cJSON_CreateArray();
			primaryaddress = get_primaryaddresses(jstr(payin_tx_data, "table_id"), 0);

			cJSON *t_pa = cJSON_CreateArray();
			for (int32_t i = 0; i < cJSON_GetArraySize(primaryaddress); i++) {
				jaddistr(t_pa, jstri(primaryaddress, i));
			}
			jaddistr(t_pa, jstr(payin_tx_data, "primaryaddress"));
			dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(t_pa));

			cJSON *temp1 = update_t_player_info_pa(jstr(payin_tx_data, "table_id"), updated_player_info, t_pa);
			dlg_info("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(temp1));
		}
	}
end:
	dlg_info("at end, do nothing atm");
}
