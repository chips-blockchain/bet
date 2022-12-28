#include "bet.h"
#include "dealer.h"
#include "vdxf.h"

cJSON* add_dealer(char *dealer_id)
{
	cJSON *dealers_info = NULL, *dealers = NULL, *out = NULL;

	if(!dealer_id)
		return NULL;
	
	dealers_info = cJSON_CreateObject();
	dealers = cJSON_CreateArray();
	jaddistr(dealers, dealer_id);

	cJSON_AddItemToObject(dealers_info,"dealers",dealers);
	out = update_cmm_from_id_key_data_cJSON("dealers",DEALERS_KEY,dealers_info);

	dlg_info("%s",cJSON_Print(out));
	return out;
}
