#include "bet.h"
#include "game.h"
#include "vdxf.h"

struct t_game_info_struct t_game_info;

const char *game_state_str(int32_t game_state)
{
	switch (game_state) {
	case G_ZEROIZED_STATE:
		return "Zeroized state, Table is not initialized yet...";
	case G_TABLE_ACTIVE:
		return "Table is active";
	case G_TABLE_STARTED:
		return "Table is started";
	case G_PLAYERS_JOINED:
		return "Players joined the table";
	case G_DECK_SHUFFLING_P:
		return "Deck shuffling by players are done";
	case G_DECK_SHUFFLING_D:
		return "Deck shuffling by dealer is done";
	case G_DECK_SHUFFLING_B:
		return "Deck shuffling by cashier is done";
	default:
		return "Invalid game state...";
	}
}

cJSON *append_game_state(char *table_id, int32_t game_state, cJSON *game_state_info)
{
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL, *out = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_warn("Porbabaly the table is not initialized");
		return NULL;
	}
	t_game_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(t_game_info, "game_state", game_state);
	if (game_state_info)
		cJSON_AddItemToObject(t_game_info, "game_state_info", game_state_info);

	out = append_cmm_from_id_key_data_cJSON(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str),
						t_game_info, true);
	return out;
}

int32_t get_game_state(char *table_id)
{
	int32_t game_state = G_ZEROIZED_STATE;
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str)
		return game_state;

	t_game_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
	if (!t_game_info)
		return game_state;

	game_state = jint(t_game_info, "game_state");
	return game_state;
}

cJSON* get_game_state_info(char *table_id)
{
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL, *game_state_info = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str)
		return NULL;

	t_game_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
	if (!t_game_info)
		return NULL;

	game_state_info = jobj(t_game_info, "game_state_info");
	return game_state_info;
}
