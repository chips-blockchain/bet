#include "bet.h"
#include "misc.h"
#include "print.h"
#include "game.h"
#include "poker_vdxf.h"

static char *fetch_str_at_height(const char *id, const char *key, int32_t height_start)
{
	char *vdxfid = NULL;
	cJSON *cmm = NULL, *vals = NULL, *last = NULL;
	int n = 0;

	vdxfid = get_vdxf_id(key);
	cmm = get_cmm_from_height_filtered(id, vdxfid, height_start);
	if (!cmm)
		return NULL;
	vals = cJSON_GetObjectItem(cmm, vdxfid);
	n = cJSON_GetArraySize(vals);
	if (n <= 0)
		return NULL;
	last = cJSON_GetArrayItem(vals, n - 1);
	if (!last)
		return NULL;
	if (last->type == cJSON_String)
		return last->valuestring;
	if (last->type == cJSON_Object)
		return jstr(last, get_vdxf_id(BYTEVECTOR_VDXF_ID));
	return NULL;
}

static cJSON *fetch_cjson_at_height(const char *id, const char *key_vdxfid, int32_t height_start)
{
	cJSON *cmm = NULL, *vals = NULL, *last = NULL;
	const char *hex = NULL;
	int n = 0;

	cmm = get_cmm_from_height_filtered(id, key_vdxfid, height_start);
	if (!cmm)
		return NULL;
	vals = cJSON_GetObjectItem(cmm, key_vdxfid);
	n = cJSON_GetArraySize(vals);
	if (n <= 0)
		return NULL;
	last = cJSON_GetArrayItem(vals, n - 1);
	if (!last)
		return NULL;
	if (last->type == cJSON_String)
		hex = last->valuestring;
	else if (last->type == cJSON_Object)
		hex = jstr(last, get_vdxf_id(BYTEVECTOR_VDXF_ID));
	if (!hex)
		return NULL;
	return hex_cJSON(hex);
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

void print_cashier_id(char *id)
{
	char *game_id = NULL;
	int32_t start_block = 0;
	cJSON *temp = NULL;

	game_id = fetch_str_at_height(id, T_GAME_ID_KEY, 0);
	if (!game_id)
		return;
	dlg_info("game_id::%s", game_id);

	start_block = find_cmm_key_write_block(id, get_vdxf_id(T_GAME_ID_KEY), game_id);
	if (start_block <= 0) {
		dlg_info("could not locate T_GAME_ID_KEY write block on cashier id");
		return;
	}
	dlg_info("start_block::%d", start_block);

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(C_DISPUTE_RESULT_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", C_DISPUTE_RESULT_KEY, cJSON_Print(temp));

	for (int32_t i = 0; i < all_t_b_p_keys_no; i++) {
		temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(all_c_b_p_keys[i], game_id), start_block);
		if (temp)
			dlg_info("%s :: %s", all_c_b_p_key_names[i], cJSON_Print(temp));
	}

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(C_CARD_BV_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", C_CARD_BV_KEY, cJSON_Print(temp));
}

void print_dealers_id(char *id)
{
	cJSON *temp = fetch_cjson_at_height(id, get_vdxf_id(DEALERS_KEY), 0);
	if (temp)
		dlg_info("%s :: %s", DEALERS_KEY, cJSON_Print(temp));
}

void print_dealer_id(char *id)
{
	cJSON *temp = fetch_cjson_at_height(id, get_vdxf_id(T_TABLE_INFO_KEY), 0);
	if (temp)
		dlg_info("%s :: %s", T_TABLE_INFO_KEY, cJSON_Print(temp));

	temp = fetch_cjson_at_height(id, get_vdxf_id("registration_info"), 0);
	if (temp)
		dlg_info("registration_info :: %s", cJSON_Print(temp));
}

void print_table_id(char *id)
{
	char *game_id = NULL;
	cJSON *t_table_info = NULL;
	int32_t start_block = 0;
	cJSON *temp = NULL;

	game_id = fetch_str_at_height(id, T_GAME_ID_KEY, 0);
	if (!game_id)
		return;
	dlg_info("game_id::%s", game_id);

	t_table_info = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id), 0);
	if (!t_table_info) {
		dlg_info("T_TABLE_INFO_KEY missing for game_id %s", game_id);
		return;
	}
	dlg_info("%s :: %s", T_TABLE_INFO_KEY, cJSON_Print(t_table_info));

	start_block = jint(t_table_info, "start_block");
	if (start_block <= 0) {
		dlg_info("start_block missing/invalid in T_TABLE_INFO_KEY");
		return;
	}
	dlg_info("start_block::%d", start_block);

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", T_PLAYER_INFO_KEY, cJSON_Print(temp));

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", T_SETTLEMENT_INFO_KEY, cJSON_Print(temp));

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_BOARD_CARDS_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", T_BOARD_CARDS_KEY, cJSON_Print(temp));

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_BETTING_STATE_KEY, game_id), start_block);
	if (temp)
		dlg_info("%s :: %s", T_BETTING_STATE_KEY, cJSON_Print(temp));

	for (int32_t i = 0; i < all_t_d_p_keys_no; i++) {
		temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(all_t_d_p_keys[i], game_id), start_block);
		if (temp)
			dlg_info("%s :: %s", all_t_d_p_key_names[i], cJSON_Print(temp));
	}

	temp = fetch_cjson_at_height(id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id), start_block);
	if (temp) {
		int game_state = jint(temp, "game_state");
		dlg_info("Game State: %s", game_state_str(game_state));
	}
}

void print_player_id(char *id)
{
	char *game_id = NULL;
	cJSON *p_game_history = NULL;
	int32_t start_block = 0;
	cJSON *temp = NULL;

	temp = fetch_cjson_at_height(id, get_vdxf_id(P_JOIN_REQUEST_KEY), 0);
	if (temp)
		dlg_info("%s :: %s", P_JOIN_REQUEST_KEY, cJSON_Print(temp));

	game_id = fetch_str_at_height(id, T_GAME_ID_KEY, 0);
	if (!game_id)
		return;
	dlg_info("game_id::%s", game_id);

	p_game_history = fetch_cjson_at_height(id, get_key_data_vdxf_id(P_GAME_HISTORY_KEY, game_id), 0);
	if (!p_game_history) {
		dlg_info("P_GAME_HISTORY_KEY missing for game_id %s", game_id);
		return;
	}
	dlg_info("%s :: %s", P_GAME_HISTORY_KEY, cJSON_Print(p_game_history));

	start_block = jint(p_game_history, "join_block");
	if (start_block <= 0) {
		dlg_info("join_block missing/invalid in P_GAME_HISTORY_KEY");
		return;
	}
	dlg_info("start_block::%d", start_block);

	const char *player_keys[] = {
		PLAYER_DECK_KEY,
		P_BETTING_ACTION_KEY,
		P_DECODED_CARD_KEY,
		P_DISPUTE_REQUEST_KEY
	};
	int num_keys = sizeof(player_keys) / sizeof(player_keys[0]);

	for (int i = 0; i < num_keys; i++) {
		temp = fetch_cjson_at_height(id, get_key_data_vdxf_id((char *)player_keys[i], game_id), start_block);
		if (temp)
			dlg_info("%s :: %s", player_keys[i], cJSON_Print(temp));
	}
}

void print_table_key_info(int argc, char **argv)
{
	char *game_id_str = NULL;
	cJSON *key_info = NULL;

	if (argc == 4) {
		game_id_str = poker_get_key_str(argv[2], get_vdxf_id(T_GAME_ID_KEY));
		if (argv[3]) {
			dlg_info("%s::%s", game_id_str, get_key_data_vdxf_id(argv[3], game_id_str));
			key_info = get_cJSON_from_id_key_vdxfid(argv[2], get_key_data_vdxf_id(argv[3], game_id_str));
			dlg_info("%s", cJSON_Print(key_info));
		}
	}
}

void print_id_info(int argc, char **argv)
{
	if (!is_id_exists(argv[2])) {
		dlg_info("ID doesn't exists\n");
	} else {
		if ((strcmp(argv[3], "t") == 0) || (strcmp(argv[3], "table") == 0)) {
			print_table_id(argv[2]);
		} else if ((strcmp(argv[3], "d") == 0) || (strcmp(argv[3], "dealer") == 0)) {
			print_dealer_id(argv[2]);
		} else if ((strcmp(argv[3], "p") == 0) || (strcmp(argv[3], "player") == 0)) {
			print_player_id(argv[2]);
		} else if (strcmp(argv[3], "dealers") == 0) {
			print_dealers_id(argv[2]);
		} else if ((strcmp(argv[3], "c") == 0) || (strcmp(argv[3], "cashier") == 0)) {
			print_cashier_id(argv[2]);
		} else {
			dlg_info("Print is not supported for this ID::%s of type::%s", argv[2], argv[3]);
		}
	}
}

void print_vdxf_info(int argc, char **argv)
{
	if (argc < 4 || !argv[2] || !argv[3]) {
		dlg_error("Invalid arguments for print_vdxf_info");
		return;
	}

	char *id = argv[2];
	char *key = argv[3];

	if (!is_id_exists(id)) {
		dlg_info("ID doesn't exist");
		return;
	}

	char *vdxf_id = get_key_vdxf_id(key);
	cJSON *json_data = NULL;
	char *str_data = NULL;

	if (strcmp(vdxf_id, get_vdxf_id(T_TABLE_INFO_KEY)) == 0 ||
	    strcmp(vdxf_id, get_vdxf_id(T_PLAYER_INFO_KEY)) == 0 || strcmp(vdxf_id, get_vdxf_id(DEALERS_KEY)) == 0) {
		json_data = get_cJSON_from_id_key_vdxfid(id, vdxf_id);
		if (json_data) {
			dlg_info("%s", cJSON_Print(json_data));
			cJSON_Delete(json_data);
		}
	} else if (strcmp(vdxf_id, get_vdxf_id(T_GAME_ID_KEY)) == 0) {
		str_data = poker_get_key_str(id, get_full_key(key));
		if (str_data) {
			dlg_info("%s", str_data);
			free(str_data);
		}
	} else if (strcmp(vdxf_id, get_vdxf_id(T_GAME_INFO_KEY)) == 0) {
		char *game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);
		if (game_id_str) {
			dlg_info("Game ID: %s", game_id_str);
			json_data =
				get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
			if (json_data) {
				int game_state = jint(json_data, "game_state");
				dlg_info("Game State: %s", game_state_str(game_state));
				cJSON_Delete(json_data);
			}
			free(game_id_str);
		}
	} else {
		dlg_info("Print operation is not supported for the given ID: %s and key: %s", id, key);
	}
}
