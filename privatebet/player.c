#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"
#include "commands.h"

cJSON *append_t_key(char *id, char *key, cJSON *key_info)
{
	int32_t argc;
	const int32_t no_of_t_keys = 11;
	char **argv = NULL, params[arg_size] = { 0 };
	char all_t_keys[11][128] = { T_TABLE_INFO_KEY, T_PLAYER_INFO_KEY, T_PLAYER1_KEY, T_PLAYER2_KEY,
				     T_PLAYER3_KEY,    T_PLAYER4_KEY,     T_PLAYER5_KEY, T_PLAYER6_KEY,
				     T_PLAYER7_KEY,    T_PLAYER8_KEY,     T_PLAYER9_KEY };
	cJSON *id_info = NULL, *argjson = NULL, *cmm = NULL, *temp_obj = NULL;

	if ((NULL == id) || (NULL == key) || (NULL == key_info) || (NULL == verus_chips_cli)) {
		return NULL;
	}

	id_info = cJSON_CreateObject();
	cJSON_AddStringToObject(id_info, "name", id);
	cJSON_AddStringToObject(id_info, "parent", get_vdxf_id(POKER_CHIPS_VDXF_ID));

	cmm = cJSON_CreateObject();
	cJSON_AddItemToObject(cmm, key, key_info);

	for (int32_t i = 0; i < no_of_t_keys; i++) {
		if (strcmp(all_t_keys[i], key) != 0) {
			cJSON_AddItemToObject(cmm, all_t_keys[i], get_cmm_key_data(id, 0, all_t_keys[i]));
		}
	}

	cJSON_AddItemToObject(id_info, "contentmultimap", cmm);
	dlg_info("%s::%d::id_info::%s\n", __func__, __LINE__, cJSON_Print(id_info));
	argc = 3;
	bet_alloc_args(argc, &argv);
	snprintf(params, arg_size, "\'%s\'", cJSON_Print(id_info));
	argv = bet_copy_args(argc, verus_chips_cli, "updateidentity", params);

	//This is a temporary wait until we have an API to spend the tx's from mempool
	while (!chips_is_mempool_empty()) {
		sleep(1);
	}

	argjson = cJSON_CreateObject();
	make_command(argc, argv, &argjson);

end:
	bet_dealloc_args(argc, &argv);
	return argjson;
}

int32_t bet_init_player_deck(int32_t player_id)
{
	int32_t retval = OK;
	char str[129], *hexstr = NULL;
	cJSON *cjson_player_cards = NULL, *player_deck = NULL, *cmm = NULL;

	if ((player_id < 1) && (player_id > 9)) {
		retval = ERR_INVALID_PLAYER_ID;
		goto end;
	}
	player_info.player_key = deckgen_player(player_info.cardprivkeys, player_info.cardpubkeys, player_info.permis,
						CARDS777_MAXCARDS);

	player_deck = cJSON_CreateObject();
	jaddnum(player_deck, "id", player_id);
	jaddbits256(player_deck, "pubkey", player_info.player_key.prod);
	jadd(player_deck, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(cjson_player_cards, bits256_str(str, player_info.cardpubkeys[i]));
	}

	cJSON_hex(player_deck, &hexstr);
	if (hexstr == NULL) {
		retval = ERR_PLAYER_DECK_SHUFFLING;
		goto end;
	}
	cJSON *player_deck_hex = NULL;
	player_deck_hex = cJSON_CreateObject();
	jaddstr(player_deck_hex, get_vdxf_id(BYTEVECTOR_VDXF_ID), hexstr);

	cmm = cJSON_CreateArray();
	jaddi(cmm, player_deck_hex);
	dlg_info("%s::%dcmm::%s\n", __func__, __LINE__, cJSON_Print(cmm));

	//cJSON *out = append_t_key(player_config.table_id, T_PLAYER_KEYS[player_id - 1], cmm);
	//dlg_info("%s::%d::%s", __func__, __LINE__, cJSON_Print(out));

end:
	return retval;
}
