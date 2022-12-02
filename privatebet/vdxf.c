#include "vdxf.h"
#include "commands.h"

cJSON* get_cashiers()
{
	int argc = 3;
	char **argv = NULL;
	cJSON *argjson = NULL;
	
	bet_alloc_args(argc,&argv);
	bet_copy_args(argc,verus_chips_cli, "getidentity", CASHIERS_ID);
	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);
	dlg_info("%s::%d:: cashiers_info::\n%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));

	end:
		bet_dealloc_args(argc,&argv);
	
}

void update_cashiers()
{
	cJSON *cashiers_info = NULL;

	cashiers_info  = cJSON_CreateObject();
	cashiers_info = get_cashiers();
}

