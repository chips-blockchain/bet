#include "bet.h"
#include "blinder.h"
#include "vdxf.h"
#include "deck.h"
#include "cards777.h"
#include "err.h"
#include "game.h"

char all_t_b_p_keys[all_t_b_p_keys_no][128] = { T_B_P1_DECK_KEY, T_B_P2_DECK_KEY, T_B_P3_DECK_KEY, T_B_P4_DECK_KEY,
						T_B_P5_DECK_KEY, T_B_P6_DECK_KEY, T_B_P7_DECK_KEY, T_B_P8_DECK_KEY,
						T_B_P9_DECK_KEY, T_B_DECK_KEY };

char all_t_b_p_key_names[all_t_b_p_keys_no][128] = { "t_b_p1_deck", "t_b_p2_deck", "t_b_p3_deck", "t_b_p4_deck",
						     "t_b_p5_deck", "t_b_p6_deck", "t_b_p7_deck", "t_b_p8_deck",
						     "t_b_p9_deck", "t_b_deck" };

struct b_deck_info_struct b_deck_info;

int32_t cashier_sb_deck(char *id, bits256 *d_blinded_deck, int32_t player_id)
{
	int32_t retval = OK;
	char str[65], *game_id_str = NULL;
	cJSON *b_blinded_deck = NULL;

	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, d_blinded_deck[i]));
	}
	shuffle_deck_db(d_blinded_deck, CARDS777_MAXCARDS, b_deck_info.b_permi);
	blind_deck_b(d_blinded_deck, CARDS777_MAXCARDS, b_deck_info.cashier_r[player_id]);

	b_blinded_deck = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%d::%s", i, bits256_str(str, d_blinded_deck[i]));
		jaddibits256(b_blinded_deck, d_blinded_deck[i]);
	}
	dlg_info("b_blinded_deck::%s", cJSON_Print(b_blinded_deck));
	cJSON *out = append_cmm_from_id_key_data_cJSON(id, get_key_data_vdxf_id(all_t_b_p_keys[player_id], game_id_str),
						       b_blinded_deck, true);

	if (!out)
		retval = ERR_DECK_BLINDING_CASHIER;

	dlg_info("%s", cJSON_Print(out));

	return retval;
}

void cashier_init_deck(char *table_id)
{
	int32_t num_players;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL;

	bet_permutation(b_deck_info.b_permi, CARDS777_MAXCARDS);

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	num_players = jint(t_player_info, "num_players");
	for (int32_t i = 0; i < num_players; i++) {
		gen_deck(b_deck_info.cashier_r[i], CARDS777_MAXCARDS);
	}
}

int32_t cashier_shuffle_deck(char *id)
{
	int32_t num_players = 0, retval = OK;
	char *game_id_str = NULL;
	cJSON *t_d_p_deck_info = NULL, *t_player_info = NULL;
	bits256 t_d_p_deck[CARDS777_MAXCARDS];

	cashier_init_deck(id);
	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	num_players = jint(t_player_info, "num_players");

	for (int32_t i = 0; i < num_players; i++) {
		t_d_p_deck_info =
			get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(all_t_d_p_keys[(i + 1)], game_id_str));
		for (int32_t j = 0; j < cJSON_GetArraySize(t_d_p_deck_info); j++) {
			t_d_p_deck[j] = jbits256i(t_d_p_deck_info, j);
		}
		retval = cashier_sb_deck(id, t_d_p_deck, i);
		if (retval)
			return retval;
	}

	return retval;
}

int32_t reveal_bv(char *table_id)
{
	int32_t player_id, card_id, num_players, retval = OK;
	char *game_id_str = NULL, hexstr[65];
	cJSON *bv_info = NULL, *game_state_info = NULL, *t_player_info = NULL, *out = NULL, *card_bv = NULL;

	game_state_info = get_game_state_info(table_id);
	player_id = jint(game_state_info, "player_id");
	card_id = jint(game_state_info, "card_id");

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);

	bv_info = cJSON_CreateArray();
	if (player_id == -1) {
		t_player_info =
			get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
		num_players = jint(t_player_info, "num_players");
		for (int32_t i = 0; i < num_players; i++) {
			jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[i][card_id].priv));
		}
	} else {
		jaddistr(bv_info, bits256_str(hexstr, b_deck_info.cashier_r[player_id][card_id].priv));
	}

	card_bv = cJSON_CreateObject();
	jaddnum(card_bv, "player_id", player_id);
	jaddnum(card_bv, "card_id", card_id);
	jadd(card_bv, "bv", bv_info);

	dlg_info("bv_info::%s", cJSON_Print(card_bv));
	out = append_cmm_from_id_key_data_cJSON(table_id, get_key_data_vdxf_id(T_CARD_BV_KEY, game_id_str), card_bv,
						true);

	if (!out) {
		retval = ERR_BV_UPDATE;
	}
	return retval;
}

static int32_t handle_bv_reveal_card(char *table_id)
{
	int32_t retval = OK;
	char *game_id_str = NULL;
	cJSON *card_bv = NULL, *game_state_info = NULL;

	game_state_info = get_game_state_info(table_id);
	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	card_bv = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_CARD_BV_KEY, game_id_str));

	if (card_bv && ((jint(game_state_info, "player_id") == jint(card_bv, "player_id")) &&
			(jint(game_state_info, "card_id") == jint(card_bv, "card_id")))) {
		dlg_info("Cashier revealed its Blinding Value for the player_id::%d, card_id::%d...",
			 jint(card_bv, "player_id"), jint(card_bv, "card_id"));
		return retval;
	}
	retval = reveal_bv(table_id);
	return retval;
}

int32_t handle_game_state_cashier(char *table_id)
{
	int32_t game_state, retval = OK;

	game_state = get_game_state(table_id);
	dlg_info("%s", game_state_str(game_state));
	switch (game_state) {
	case G_ZEROIZED_STATE:
	case G_TABLE_ACTIVE:
	case G_TABLE_STARTED:
	case G_PLAYERS_JOINED:
	case G_DECK_SHUFFLING_P:
	case G_DECK_SHUFFLING_B:
		break;
	case G_DECK_SHUFFLING_D:
		retval = cashier_shuffle_deck(table_id);
		if (!retval)
			append_game_state(table_id, G_DECK_SHUFFLING_B, NULL);
		break;
	case G_REVEAL_CARD:
		retval = handle_bv_reveal_card(table_id);
		break;
	}
	return retval;
}

int32_t cashier_game_init(char *table_id)
{
	int32_t retval = OK;

	while (1) {
		retval = handle_game_state_cashier(table_id);
		if (retval)
			return retval;
		sleep(2);
	}
}
