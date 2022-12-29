#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"
#include "commands.h"
#include "deck.h"

struct p_deck_info_struct p_deck_info;

int32_t player_init_deck()
{
	int32_t retval = OK;
	char str[65];
	cJSON *cjson_player_cards = NULL, *player_deck = NULL;
	
	char t_player_keys[9][128] = { T_PLAYER1_KEY, T_PLAYER2_KEY, T_PLAYER3_KEY, T_PLAYER4_KEY, T_PLAYER5_KEY,
						   T_PLAYER6_KEY, T_PLAYER7_KEY, T_PLAYER8_KEY, T_PLAYER9_KEY };
	
	if ((p_deck_info.player_id < 1) && (p_deck_info.player_id > 9))
		return ERR_INVALID_PLAYER_ID;
	
	p_deck_info.p_kp = gen_keypair();

	gen_deck(p_deck_info.player_r, CARDS777_MAXCARDS);
	
	player_deck = cJSON_CreateObject();
	jaddnum(player_deck, "id", p_deck_info.player_id);
	jaddbits256(player_deck, "pubkey", p_deck_info.p_kp.prod);
	jadd(player_deck, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(cjson_player_cards, bits256_str(str, p_deck_info.player_r[i].prod)); 
	}

	dlg_info("player_key::%s", get_key_data_vdxf_id(t_player_keys[p_deck_info.player_id - 1], bits256_str(str, p_deck_info.game_id)));

	cJSON *out = append_cmm_from_id_key_data_cJSON(
		player_config.table_id,
		get_key_data_vdxf_id(t_player_keys[p_deck_info.player_id - 1], bits256_str(str, p_deck_info.game_id)), player_deck, true);
	dlg_info("%s", cJSON_Print(out));

	return retval;	
}
