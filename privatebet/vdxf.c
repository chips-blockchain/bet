#include "bet.h"
#include "vdxf.h"
#include "commands.h"
#include "misc.h"

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

struct table *find_table()
{
	cJSON *dealer_ids = NULL;
	char *preferred = "sg777_d";
	struct table *t;

	dealer_ids = cJSON_CreateArray();
	dealer_ids = get_dealers();

	for (int32_t i = 0; i < cJSON_GetArraySize(dealer_ids); i++) {
		if (0 == strcmp(preferred, cJSON_Print(cJSON_GetArrayItem(dealer_ids, i)))) {
			dlg_info("%s::%d::The preferred dealer id exists::%s\n", __FUNCTION__, __LINE__,
				 cJSON_Print(cJSON_GetArrayItem(dealer_ids, i)));
			break;
		}
	}
	t = get_dealers_config_table(preferred);
	dlg_info("max_players :: %d", t->max_players);
	dlg_info("big_blind :: %f", uint32_s_to_float(t->big_blind));
	dlg_info("min_stake :: %f", uint32_s_to_float(t->min_stake));
	dlg_info("max_stake :: %f", uint32_s_to_float(t->max_stake));
	dlg_info("table_id :: %s", t->table_id);
	return t;
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

cJSON* getaddressutxos(char verus_addresses[][], int n)
{
	int argc;
	char **argv = NULL, params[arg_size] = {0};
	cJSON *addresses = NULL, *addr_info = NULL, *argjson = NULL;
	
	addresses = cJSON_CreateArray();
	addr_info = cJSON_CreateObject();

	for(int32_t i=0; i<n; i++) {
		cJSON_AddItemToArray(addresses,cJSON_CreateString(verus_addresses[i]));
	}

	cJSON_AddItemToObject(addr_info, "addresses", addresses);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(addr_info));

	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, verus_chips_cli, "getaddressutxos", params);

	argjson = cJSON_CreateObject();
	make_command(argc,argv,&argjson);

	end:
		bet_dealloc_args(argc,&argv);
		return argjson;
	
}

void test_loop()
{
	char[1][] verus_addr = ["cashiers.poker.chips10sec@"];
	int32_t blockcount = 149267;
	
	while(1) {
		sleep(5);
		cJSON *argjson = cJSON_CreateObject();
		argjson = getaddressutxos(verus_addr,1);

		for(int32_t i=0; i<cJSON_GetArraySize(argjson); i++){
			if(jint(cJSON_GetArrayItem(argjson,i),"height")>blockcount){
				dlg_info("%s::%d::tx_to_process::%s\n", __FUNCTION__, __LINE__, cJSON_Print(cJSON_GetArrayItem(argjson,i)));
			}
			
		}
		
	}
}
