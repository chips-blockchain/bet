#include "bet.h"
#include "common.h"
#include "misc.h"
#include "vdxf.h"
#include "print.h"


static char* get_table_full_key(char *key)
{
	int32_t no_of_keys = 11;	
	char *key_name = NULL;
	char all_t_keys[11][128] = { T_TABLE_INFO_KEY, T_PLAYER_INFO_KEY, T_PLAYER1_KEY, T_PLAYER2_KEY,
				     T_PLAYER3_KEY,    T_PLAYER4_KEY,     T_PLAYER5_KEY, T_PLAYER6_KEY,
				     T_PLAYER7_KEY,    T_PLAYER8_KEY,     T_PLAYER9_KEY };
	
	char all_t_key_names[11][128] = { "t_table_info", "t_player_info", "t_player1", "t_player2",
				     "t_player3",    "t_player4",     "t_player5", "t_player6",
				     "t_player7",    "t_player8",     "t_player9" };
	
	for(int32_t i=0; i<no_of_keys; i++){
		if(strcmp(key, all_t_key_names[i]) == 0) {
			key_name = calloc(1, 128);
			strcpy(key_name, all_t_keys[i]);
			return key_name;
		}	
	}
	
	return NULL;
}

void print_struct_table(struct table *t)
{
	if (t) {
		dlg_info("Table Info\n");
		dlg_info("max_players :: %d", t->max_players);
		dlg_info("big_blind :: %f", uint32_s_to_float(t->big_blind));
		dlg_info("min_stake :: %f", uint32_s_to_float(t->min_stake));
		dlg_info("max_stake :: %f", uint32_s_to_float(t->max_stake));
		dlg_info("table_id :: %s", t->table_id);
		dlg_info("dealer_id :: %s", t->dealer_id);
	}
}

void print_cashiers_id(char *id)
{
	int32_t no_of_keys = 1;
	char all_c_keys[1][128] = { CASHIERS_KEY };

	for (int32_t i = 0; i < no_of_keys; i++) {
		cJSON *temp = get_cJSON_from_id_key_vdxfid(id, get_vdxf_id(all_c_keys[i]));
		if (temp)
			dlg_info("%s", cJSON_Print(temp));
	}
}

void print_dealers_id(char *id)
{
	int32_t no_of_keys = 1;
	char all_ds_keys[1][128] = { DEALERS_KEY };

	for (int32_t i = 0; i < no_of_keys; i++) {
		cJSON *temp = get_cJSON_from_id_key_vdxfid(id, get_vdxf_id(all_ds_keys[i]));
		if (temp)
			dlg_info("%s", cJSON_Print(temp));
	}
}

void print_dealer_id(char *id)
{
	int32_t no_of_keys = 1;
	char all_d_keys[1][128] = { T_TABLE_INFO_KEY };

	for (int32_t i = 0; i < no_of_keys; i++) {
		cJSON *temp = get_cJSON_from_id_key_vdxfid(id, get_vdxf_id(all_d_keys[i]));
		if (temp)
			dlg_info("%s", cJSON_Print(temp));
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
	if (game_id) {
		dlg_info("game_id::%s", game_id);
		for (int32_t i = 0; i < no_of_keys; i++) {
			cJSON *temp = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(all_t_keys[i], game_id));
			if (temp)
				dlg_info("%s", cJSON_Print(temp));
		}
	}
}


void print_table_key_info(int argc, char **argv)
{
	char *game_id_str = NULL, *key_name = NULL;
	cJSON *key_info = NULL;

	if(argc == 4) {
		game_id_str = get_str_from_id_key(argv[2], get_vdxf_id(T_GAME_ID_KEY)); 
		key_name = get_table_full_key(argv[3]);
		if(key_name) {
			dlg_info("%s::%s", game_id_str, get_key_data_vdxf_id(key_name,game_id_str));
			key_info = get_cJSON_from_id_key_vdxfid(argv[2], get_key_data_vdxf_id(key_name, game_id_str));
			dlg_info("%s", cJSON_Print(key_info));	
		}		
	}
}

void print_id_info(int argc, char **argv)
{
	if (argc == 4) {
		if ((strcmp(argv[3], "t") == 0) || (strcmp(argv[3], "table") == 0)) {
			print_table_id(argv[2]);
		} else if ((strcmp(argv[3], "d") == 0) || (strcmp(argv[3], "dealer") == 0)) {
			print_dealer_id(argv[2]);
		} else if (strcmp(argv[3], "dealers") == 0) {
			print_dealers_id(argv[2]);
		} else if (strcmp(argv[3], "cashiers") == 0) {
			print_dealers_id(argv[2]);
		} else {
			dlg_info("Print is not supported for this ID::%s of type::%s", argv[2], argv[3]);
		}
	}
}

void print_vdxf_info(int argc, char **argv)
{
	char *str = NULL;
	cJSON *cmm = NULL;

	if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_TABLE_INFO_KEY)) == 0) {
		cmm = get_cJSON_from_id_key_vdxfid(argv[2], get_key_vdxf_id(argv[3]));
		dlg_info("%s", cJSON_Print(cmm));
	} else if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_PLAYER_INFO_KEY)) == 0) {
		cmm = get_cJSON_from_id_key_vdxfid(argv[2], get_key_vdxf_id(argv[3]));
		dlg_info("%s", cJSON_Print(cmm));
	} else if (strcmp(get_key_vdxf_id(argv[3]), get_vdxf_id(T_GAME_ID_KEY)) == 0) {
		str = get_str_from_id_key(argv[2], get_full_key(argv[3]));
		dlg_info("%s", str);
	} else {
		dlg_info("Print operation is not supported for the given ID ::%s and key ::%s", argv[2], argv[3]);
	}
}
