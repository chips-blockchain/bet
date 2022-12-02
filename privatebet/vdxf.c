#include "vdxf.h"
#include "commands.h"

cJSON* get_cashiers()
{
	int argc = 3;
	char **argv = NULL;
	cJSON *argjson = NULL, *contentmultimap = NULL, * cashiers = NULL;
	
	bet_alloc_args(argc,&argv);
	strncpy(blockchain_cli, verus_chips_cli, strlen(verus_chips_cli));
	argv = bet_copy_args(argc,verus_chips_cli, "getidentity", CASHIERS_ID);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);	
	contentmultimap  = cJSON_GetObjectItem(cJSON_GetObjectItem(argjson,"identity"), "contentmultimap");

	dlg_info("\ncontentmultimap :: %s\n", cJSON_Print(contentmultimap));
	
	cashiers = cJSON_CreateArray();
	cashiers = cJSON_GetObjectItem(contentmultimap,"iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP");

	dlg_info("\ncashiers :: %s\n", cJSON_Print(cashiers));
	
	for(int32_t i=0; i<cJSON_GetArraySize(cashiers); i++){		
		dlg_info("IP :: %s\n", cJSON_Print(cJSON_GetObjectItem(cJSON_GetArrayItem(cashiers,i),STRING_VDXF_ID)));
	}	

	end:
		bet_dealloc_args(argc,&argv);
	
}


void update_cashiers(char *ip)
{
	cJSON *cashiers_info = NULL, *update_info = NULL, *ip_obj = NULL, *argjson = NULL;
	int argc;
	char **argv = NULL;

	cashiers_info  = cJSON_CreateObject();
	update_info = cJSON_CreateObject();

	cJSON_AddStringToObject(update_info, "name", "cashiers");
	cJSON_AddStringToObject(update_info, "parent", CASHIERS_CASHIERS_VDXF_ID);

	ip_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(ip_obj, STRING_VDXF_ID, ip);
	cJSON_AddItemToObject(cashiers_info,CASHIERS_KEY,ip_obj);
	
	cJSON_AddItemToObject(update_info,"contentmultimap",cashiers_info);

	dlg_info("%s::%d::\n%s\n", __FUNCTION__, __LINE__, cJSON_Print(update_info));

	argc = 3;
	bet_alloc_args(argc,&argv);

	
	
	dlg_info("\nstr::%s\n", stringify(cJSON_Print(update_info)));
	


	
	argv = bet_copy_args(argc,verus_chips_cli, "updateidentity", stringify(cJSON_Print(update_info)));
	
	argjson = cJSON_CreateObject();
	make_command(argc,argv,&argjson);
	dlg_info("\n%s\n", cJSON_Print(argjson));
	
	end:
		bet_dealloc_args(argc, &argv);

}

