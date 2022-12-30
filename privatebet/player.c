#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"
#include "commands.h"
#include "deck.h"

char all_t_p_keys[all_t_p_keys_no][128] = { T_PLAYER1_KEY, T_PLAYER2_KEY,    T_PLAYER3_KEY,    T_PLAYER4_KEY,
					    T_PLAYER5_KEY, T_PLAYER6_KEY,    T_PLAYER7_KEY,    T_PLAYER8_KEY,
					    T_PLAYER9_KEY, T_TABLE_INFO_KEY, T_PLAYER_INFO_KEY };

char all_t_p_key_names[all_t_p_keys_no][128] = { "t_player1", "t_player2",    "t_player3",    "t_player4",
						 "t_player5", "t_player6",    "t_player7",    "t_player8",
						 "t_player9", "t_table_info", "t_player_info" };

struct p_deck_info_struct p_deck_info;

int32_t player_init_deck()
{
	int32_t retval = OK;
	char str[65];
	cJSON *cjson_player_cards = NULL, *player_deck = NULL;

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

	dlg_info("player_key::%s",
		 get_key_data_vdxf_id(all_t_p_keys[p_deck_info.player_id - 1], bits256_str(str, p_deck_info.game_id)));

	cJSON *out = append_cmm_from_id_key_data_cJSON(player_config.table_id,
						       get_key_data_vdxf_id(all_t_p_keys[p_deck_info.player_id - 1],
									    bits256_str(str, p_deck_info.game_id)),
						       player_deck, true);
	dlg_info("%s", cJSON_Print(out));

	return retval;
}
