#include "vdxf.h"
#include "commands.h"

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

cJSON* update_cmm(char *id, cJSON *cmm)
{
	cJSON *id_info = NULL, *argjson = NULL;
	int argc;
	char **argv = NULL;
	char params[arg_size] = { 0 };

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", dealer_ID);
	cJSON_AddStringToObject(id_info, "parent", POKER_CHIPS_VDXF_ID);
	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);

	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

	end:
		bet_dealloc_args(argc,&argv);
		
	return argjson;
}
