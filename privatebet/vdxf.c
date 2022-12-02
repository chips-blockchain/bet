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
	contentmultimap  = cJSON_GetObjectItem(argjson, "contentmultimap");
	cashiers = cJSON_CreateArray();
	cashiers = cJSON_GetObjectItem(contentmultimap,"iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP");

	for(int32_t i=0; i<cJSON_GetArraySize(cashiers); i++){		
		dlg_info("IP :: %s\n", cJSON_Print(cJSON_GetObjectItem(cJSON_GetArrayItem(cashiers,i),STRING_VDXF_ID)));
	}	

	end:
		bet_dealloc_args(argc,&argv);
	
}

void update_cashiers()
{
	cJSON *cashiers_info = NULL;

	cashiers_info  = cJSON_CreateObject();
	cashiers_info = get_cashiers();
}

