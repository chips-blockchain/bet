#include "bet.h"
#include "common.h"
#include "misc.h"
#include "vdxf.h"
#include "display.h"

void print_struct_table(struct table *t)
{
	if(t) {
		dlg_info("max_players :: %d", t->max_players);
		dlg_info("big_blind :: %f", uint32_s_to_float(t->big_blind));
		dlg_info("min_stake :: %f", uint32_s_to_float(t->min_stake));
		dlg_info("max_stake :: %f", uint32_s_to_float(t->max_stake));
		dlg_info("table_id :: %s", t->table_id);
		dlg_info("dealer_id :: %s", t->dealer_id);
	}	
}

void print_table_id(char *id)
{
	int32_t no_of_keys = 11;
	char *game_id = NULL;
	char all_t_keys[11][128] = { T_TABLE_INFO_KEY, T_PLAYER_INFO_KEY, T_PLAYER1_KEY, T_PLAYER2_KEY,
				     T_PLAYER3_KEY,    T_PLAYER4_KEY,     T_PLAYER5_KEY, T_PLAYER6_KEY,
				     T_PLAYER7_KEY,    T_PLAYER8_KEY,     T_PLAYER9_KEY };

	game_id = get_str_from_id_key(id, get_vdxf_id(T_GAME_ID_KEY));
	if(game_id) {
		for(int32_t i=0; i<no_of_keys; i++){
			cJSON *temp = get_cJSON_from_id_key(id,get_key_data_vdxf_id(all_t_keys[i], game_id));
			if(temp) {
				dlg_info("%s::%d::key::%s::value::%s\n", __func__, __LINE__, all_t_keys[i], temp);
			}
		}	
		
	}
	
}

void print_vdxf_info(int argc, char **argv)
{
	cJSON *cmm = NULL;
	
	if(argc == 3) {
		print_table_id(argv[2]);
	} else if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_TABLE_INFO_KEY)) == 0) {
		cmm = get_cmm_key_data(argv[2], 0, get_key_vdxf_id(argv[3]));
		if (cmm) {
			print_struct_table(decode_table_info(cmm));
		} else {
			dlg_info("There isn't any data with the key ::%s(%s) on the ID::%s\n", argv[3],
				 get_key_vdxf_id(argv[3]), argv[2]);
		}
	} else if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_PLAYER_INFO_KEY)) == 0) {
		cmm = get_t_player_info(argv[2]);
		dlg_info("%s::%d::id::%s::t_player_info::%s\n", __FUNCTION__, __LINE__, argv[2],
			 cJSON_Print(cmm));
	} else if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_GAME_ID_KEY)) == 0) {
		char *str = get_str_from_id_key(argv[2], get_full_key(argv[3]));
		dlg_info("%s::%d::%s\n", __func__, __LINE__, str);
	} else if (strcmp(get_key_vdxf_id(argv[3]), T_PLAYER1_KEY) == 0) {
		cmm = get_cJSON_from_id_key(argv[2], argv[3]);
		dlg_info("%s::%d::%s\n", __func__, __LINE__, cJSON_Print(cmm));
	} else {
		dlg_info("The key::%s(%s), is not present in the ID::%s\n", argv[3], get_key_vdxf_id(argv[3]),
			 argv[2]);
	}
}

