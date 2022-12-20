#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"

int32_t bet_init_player_deck(int32_t player_id)
{
	int32_t retval = OK, deck_size = CARDS777_MAXCARDS * 32;
	char str[65], *hexstr = NULL;
	cJSON *cjson_player_cards = NULL, *player_deck = NULL, *cmm = NULL;
	uint8_t cards_info[64];
	struct pair256 temp_key;
	
	if ((player_id < 1) && (player_id > 9)) {
		retval = ERR_INVALID_PLAYER_ID;
		goto end;
	}
	player_info.player_key = deckgen_player(player_info.cardprivkeys, player_info.cardpubkeys, player_info.permis,
						CARDS777_MAXCARDS);

	memcpy(cards_info, player_info.player_key.priv.bytes, 32);
	memcpy(cards_info, player_info.player_key.prod.bytes, 32);
	dlg_info("%s::%s\n", bits256_str(str,player_info.player_key.priv), bits256_str(str,player_info.player_key.prod));
	for(int32_t i=0; i<64; i++) {
		printf("%x", cards_info[i]);
		if((i/32) == 0)
			temp_key.priv.bytes[i] = cards_info[i];
		else
			temp_key.prod.bytes[i-32] = cards_info[i];
	}
	dlg_info("%s::%s\n", bits256_str(str,temp_key.priv), bits256_str(str,temp_key.prod));
	
	player_deck = cJSON_CreateObject();
	jaddnum(player_deck, "id", player_id);
	jaddbits256(player_deck, "pubkey", player_info.player_key.prod);
	jadd(player_deck, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(cjson_player_cards, bits256_str(str, player_info.cardpubkeys[i]));
		//memcpy(cards_info+(i*32), player_info.cardpubkeys[i].bytes, 32);
	}

	cJSON_hex(player_deck, &hexstr);
	if (hexstr == NULL) {
		retval = ERR_PLAYER_DECK_SHUFFLING;
		goto end;
	}
	cJSON *player_deck_hex = NULL;
	player_deck_hex = cJSON_CreateObject();
	jaddstr(player_deck_hex,STRING_VDXF_ID,hexstr);
	
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
