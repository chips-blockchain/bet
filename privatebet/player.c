#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"

int32_t bet_init_player_deck(int32_t player_id)
{
	int32_t retval = OK;
	char str[65], *hexstr = NULL;
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

	cmm = cJSON_CreateObject();
	jaddstr(cmm, T_PLAYER_KEYS[player_id], hexstr);

	dlg_info("%s::%d::%s\ncmm::%s", __func__, __LINE__, cJSON_Print(player_deck), cJSON_Print(cmm));
	cJSON *out = update_cmm(player_config.table_id, cmm);
	dlg_info("%s::%d::%s", __func__, __LINE__, cJSON_Print(out));

end:
	return retval;
}
