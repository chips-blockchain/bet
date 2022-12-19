#include "common.h"
#include "player.h"

int32_t bet_init_player_deck(int32_t player_id)
{
	int32_t retval = OK;
	char str[65];
	struct pair256 key;
	cJSON *init_p = NULL, *cjson_player_cards = NULL;
	
	key = deckgen_player(player_info.cardprivkeys, player_info.cardpubkeys, player_info.permis, poker_deck_size);
	player_info.player_key = key;
	
	init_p = cJSON_CreateObject();

	jaddstr(init_p, "method", "init_p");
	jaddnum(init_p, "peerid", player_id);
	jaddbits256(init_p, "pubkey", player_info.player_key.prod);
	cJSON_AddItemToObject(init_p, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int i = 0; i < poker_deck_size; i++) {
		jaddistr(cjson_player_cards,bits256_str(str, player_info.cardpubkeys[i]));
	}
	dlg_info("%s::%d::%s\n", __func__, __LINE__, cJSON_Print(init_p));	
	return retval;
}

