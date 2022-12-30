#include "bet.h"
#include "game.h"

struct t_game_info_struct t_game_info;


cJSON* append_game_state(char* table_id, int32_t game_state, cJSON* game_info)
{
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL, *out = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if(!game_id_str) {
		dlg_warn("Porbabaly the table is not initialized");
		return NULL;
	}	
	t_game_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(t_game_info, "game_state", game_state);
	if(game_info)
		cJSON_AddItemToObject(t_game_info, "game_info", game_info);

	out = append_cmm_from_id_key_data_cJSON(table_id,get_key_data_vdxf_id(T_GAME_INFO_KEY,game_id_str),t_game_info,true);
	return out;
}

int32_t get_game_state(char *table_id)
{	
	int32_t game_state = -1;
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL;
	
	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if(!game_id_str)
		return game_state;

	t_game_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY,game_id_str));
	if(!t_game_info)
		return game_state;
	
	game_state = jint(t_game_info, "game_state");
	return game_state; 
}
