#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"

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
	jaddstr(player_deck_hex,BYTEVECTOR_VDXF_ID,hexstr);
	
	cmm = cJSON_CreateArray();
	jaddi(cmm,player_deck_hex);
	dlg_info("%s::%dcmm::%s\n", __func__, __LINE__, cJSON_Print(cmm));

	cJSON *temp = NULL;
	temp = cJSON_CreateObject();
	cJSON_AddItemToObject(temp,T_PLAYER_KEYS[player_id-1], cmm);
	cJSON *out = update_cmm(player_config.table_id, temp);
	dlg_info("%s::%d::%s", __func__, __LINE__, cJSON_Print(out));

end:
	return retval;
}