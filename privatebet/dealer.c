#include "bet.h"
#include "dealer.h"
#include "vdxf.h"
#include "deck.h"
#include "cards777.h"
#include "game.h"
#include "err.h"
#include "misc.h"

struct d_deck_info_struct d_deck_info;

char all_t_d_p_keys[all_t_d_p_keys_no][128] = { T_D_DECK_KEY,    T_D_P1_DECK_KEY, T_D_P2_DECK_KEY, T_D_P3_DECK_KEY,
						T_D_P4_DECK_KEY, T_D_P5_DECK_KEY, T_D_P6_DECK_KEY, T_D_P7_DECK_KEY,
						T_D_P8_DECK_KEY, T_D_P9_DECK_KEY };

char all_t_d_p_key_names[all_t_d_p_keys_no][128] = { "t_d_deck",    "t_d_p1_deck", "t_d_p2_deck", "t_d_p3_deck",
						     "t_d_p4_deck", "t_d_p5_deck", "t_d_p6_deck", "t_d_p7_deck",
						     "t_d_p8_deck", "t_d_p9_deck" };

cJSON *add_dealer(char *dealer_id)
{
	cJSON *dealers_info = NULL, *dealers = NULL, *out = NULL;

	if (!dealer_id)
		return NULL;

	dealers_info = cJSON_CreateObject();
	dealers = cJSON_CreateArray();
	jaddistr(dealers, dealer_id);

	cJSON_AddItemToObject(dealers_info, "dealers", dealers);
	out = update_cmm_from_id_key_data_cJSON("dealers", DEALERS_KEY, dealers_info, false);

	dlg_info("%s", cJSON_Print(out));
	return out;
}

int32_t dealer_sb_deck(char *id, bits256 *player_r, int32_t player_id)
{
	char str[65], *game_id_str = NULL;
	cJSON *d_blinded_deck = NULL;

	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}
	shuffle_deck_db(player_r, CARDS777_MAXCARDS, d_deck_info.d_permi);
	blind_deck_d(player_r, CARDS777_MAXCARDS, d_deck_info.dealer_r);

	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}

	d_blinded_deck = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(d_blinded_deck, bits256_str(str, player_r[i]));
	}

	cJSON *out = append_cmm_from_id_key_data_cJSON(id, get_key_data_vdxf_id(all_t_d_p_keys[player_id], game_id_str),
						       d_blinded_deck, true);

	dlg_info("%s", cJSON_Print(out));
}

void dealer_init_deck()
{
	bet_permutation(d_deck_info.d_permi, CARDS777_MAXCARDS);
	gen_deck(d_deck_info.dealer_r, CARDS777_MAXCARDS);
}

void test_dealer_sb(char *id)
{
	char *game_id_str = NULL, str[65];
	cJSON *t_player1 = NULL, *t_player2 = NULL, *t_p1_cardinfo = NULL, *t_p2_cardinfo = NULL, *t_d_deck_info = NULL;
	bits256 t_p1_r[CARDS777_MAXCARDS], t_p2_r[CARDS777_MAXCARDS];

	dealer_init_deck();
	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);

	t_player1 = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_PLAYER1_KEY, game_id_str));
	t_p1_cardinfo = cJSON_GetObjectItem(t_player1, "cardinfo");
	for (int32_t i = 0; i < cJSON_GetArraySize(t_p1_cardinfo); i++) {
		t_p1_r[i] = jbits256i(t_p1_cardinfo, i);
	}
	dealer_sb_deck(id, t_p1_r, 1);

	t_player2 = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_PLAYER2_KEY, game_id_str));
	t_p2_cardinfo = cJSON_GetObjectItem(t_player2, "cardinfo");
	for (int32_t i = 0; i < cJSON_GetArraySize(t_p2_cardinfo); i++) {
		t_p2_r[i] = jbits256i(t_p2_cardinfo, i);
	}
	dealer_sb_deck(id, t_p2_r, 2);

	t_d_deck_info = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(t_d_deck_info, bits256_str(str, d_deck_info.dealer_r[i].prod));
	}

	cJSON *out = append_cmm_from_id_key_data_cJSON(id, get_key_data_vdxf_id(T_D_DECK_KEY, game_id_str),
						       t_d_deck_info, true);
	dlg_info("%s", cJSON_Print(out));
}

int32_t dealer_table_init(struct table t)
{
	int32_t game_state = G_ZEROIZED_STATE, retval = OK;
	char hexstr[65];
	cJSON *out = NULL;

	if (!is_id_exists(t.table_id, 0))
		return ERR_ID_NOT_FOUND;

	game_state = get_game_state(t.table_id);
	if (game_state == G_ZEROIZED_STATE) {
		game_id = rand256(0);
		out = append_cmm_from_id_key_data_hex(t.table_id, T_GAME_ID_KEY, bits256_str(hexstr, game_id), false);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		out = append_game_state(t.table_id, G_TABLE_ACTIVE, NULL);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		out = append_cmm_from_id_key_data_cJSON(
			t.table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, bits256_str(hexstr, game_id)),
			struct_table_to_cJSON(&t), true);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		out = append_game_state(t.table_id, G_TABLE_STARTED, NULL);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));
	} else {
		dlg_info("Table is in game, at state ::%d", game_state);
	}
	return OK;
}
