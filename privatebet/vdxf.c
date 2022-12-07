#include "vdxf.h"
#include "commands.h"
#include "misc.h"

cJSON *get_cashiers()
{
	int argc = 3;
	char **argv = NULL;
	cJSON *argjson = NULL, *contentmultimap = NULL, *cashiers = NULL, *cashier_ips = NULL;

	bet_alloc_args(argc, &argv);
	strncpy(blockchain_cli, verus_chips_cli, strlen(verus_chips_cli));
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", CASHIERS_ID);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	contentmultimap = cJSON_GetObjectItem(cJSON_GetObjectItem(argjson, "identity"), "contentmultimap");

	cashiers = cJSON_CreateArray();
	cashiers = cJSON_GetObjectItem(contentmultimap, CASHIERS_KEY);

	cashier_ips = cJSON_CreateArray();
	for (int32_t i = 0; i < cJSON_GetArraySize(cashiers); i++) {
		cJSON_AddItemToArray(cashier_ips, cJSON_GetObjectItem(cJSON_GetArrayItem(cashiers, i), STRING_VDXF_ID));
	}
	dlg_info("%s::%d::cashier ips::%s\n", __FUNCTION__, __LINE__, cJSON_Print(cashier_ips));

end:
	bet_dealloc_args(argc, &argv);
}

cJSON *get_onchain_cashiers()
{
	int argc = 3;
	char **argv = NULL;
	cJSON *argjson = NULL, *contentmultimap = NULL, *cashiers = NULL;

	bet_alloc_args(argc, &argv);
	strncpy(blockchain_cli, verus_chips_cli, strlen(verus_chips_cli));
	argv = bet_copy_args(argc, verus_chips_cli, "getidentity", CASHIERS_ID);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	contentmultimap = cJSON_GetObjectItem(cJSON_GetObjectItem(argjson, "identity"), "contentmultimap");

	dlg_info("\ncontentmultimap :: %s\n", cJSON_Print(contentmultimap));

	cashiers = cJSON_CreateArray();
	cashiers = cJSON_GetObjectItem(contentmultimap, CASHIERS_KEY);

	dlg_info("\ncashiers :: %s\n", cJSON_Print(cashiers));

	for (int32_t i = 0; i < cJSON_GetArraySize(cashiers); i++) {
		dlg_info("IP :: %s\n",
			 cJSON_Print(cJSON_GetObjectItem(cJSON_GetArrayItem(cashiers, i), STRING_VDXF_ID)));
	}

end:
	bet_dealloc_args(argc, &argv);

	return cashiers;
}

void update_cashiers(char *ip)
{
	cJSON *cashiers_info = NULL, *update_info = NULL, *ip_obj = NULL, *argjson = NULL, *cashier_ips = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 };

	cashiers_info = cJSON_CreateObject();
	cashier_ips = get_onchain_cashiers();

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

	update_info = cJSON_CreateObject();
	cJSON_AddStringToObject(update_info, "name", "cashiers");
	cJSON_AddStringToObject(update_info, "parent", POKER_CHIPS_VDXF_ID);
	cJSON_AddItemToObject(update_info, "contentmultimap", cashiers_info);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(update_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	dlg_info("\n%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));

end:
	bet_dealloc_args(argc, &argv);
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
	cJSON *id_info = NULL, *argjson = NULL, *cmm = NULL;

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

cJSON *get_cmm_key_data(char *id, int16_t full_id, char *key)
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

cJSON *update_dealers_config_table(char *dealer_id, struct table t)
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
	cJSON_AddItemToObject(dealer_cmm_key, DEALERS_KEY, dealer_cmm);

	out = cJSON_CreateObject();
	out = update_cmm(dealer_ID, dealer_cmm_key);

	return out;
}

struct table *get_dealers_config_table(char *dealer_id)
{
	cJSON *dealer_cmm_data = NULL;
	char *str = NULL;
	uint8_t *table_data = NULL;
	struct table *t = NULL;

	if (NULL == dealer_id)
		goto end;

	dealer_cmm_data = cJSON_CreateObject();
	dealer_cmm_data = get_cmm_key_data(dealer_id, 0, DEALERS_KEY);

	str = jstr(cJSON_GetArrayItem(dealer_cmm_data, 0), STRING_VDXF_ID);

	table_data = calloc(1, (strlen(str) + 1) / 2);
	decode_hex(table_data, (strlen(str) + 1) / 2, str);

	t = calloc(1, sizeof(struct table));
	t = (struct table *)table_data;

	dlg_info("max players::%d\n", t->max_players);
	dlg_info("bb::%f\n", uint32_s_to_float(t->big_blind));
	dlg_info("min_stake::%f\n", uint32_s_to_float(t->min_stake));
	dlg_info("max_stake::%f\n", uint32_s_to_float(t->max_stake));

end:
	return t;
}
