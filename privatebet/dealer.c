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
	int32_t retval = OK;
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

	if (!out)
		retval = ERR_DECK_BLINDING_DEALER;
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

void dealer_init_deck()
{
	bet_permutation(d_deck_info.d_permi, CARDS777_MAXCARDS);
	gen_deck(d_deck_info.dealer_r, CARDS777_MAXCARDS);
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
			return ERR_GAME_STATE_UPDATE;
		dlg_info("%s", cJSON_Print(out));

		out = append_cmm_from_id_key_data_cJSON(
			t.table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, bits256_str(hexstr, game_id)),
			struct_table_to_cJSON(&t), true);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		out = append_game_state(t.table_id, G_TABLE_STARTED, NULL);
		if (!out)
			return ERR_GAME_STATE_UPDATE;
		dlg_info("%s", cJSON_Print(out));
	} else {
		dlg_info("Table is in game, at state ::%s", game_state_str(game_state));
	}
	return OK;
}

bool is_players_shuffled_deck(char *table_id)
{
	int32_t game_state, num_players = 0, count = 0;
	;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL;

	game_state = get_game_state(table_id);

	if (game_state == G_DECK_SHUFFLING_P) {
		return true;
	} else if (game_state == G_PLAYERS_JOINED) {
		game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
		t_player_info =
			get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
		num_players = jint(t_player_info, "num_players");
		for (int32_t i = 0; i < num_players; i++) {
			if (get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(all_t_p_keys[i], game_id_str)))
				count++;
		}
		if (count == num_players)
			return true;
	}
	return false;
}

int32_t dealer_shuffle_deck(char *id)
{
	int32_t num_players = 0, retval = OK;
	char *game_id_str = NULL, str[65];
	cJSON *t_player = NULL, *t_p_cardinfo = NULL, *t_d_deck_info = NULL;
	cJSON *t_player_info = NULL;
	bits256 t_p_r[CARDS777_MAXCARDS];

	dealer_init_deck();
	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);

	t_player_info = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));
	num_players = jint(t_player_info, "num_players");

	for (int32_t i = 0; i < num_players; i++) {
		cJSON *t_player = get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(all_t_p_keys[i], game_id_str));
		cJSON *t_p_cardinfo = cJSON_GetObjectItem(t_player, "cardinfo");
		for (int32_t j = 0; j < cJSON_GetArraySize(t_p_cardinfo); j++) {
			t_p_r[j] = jbits256i(t_p_cardinfo, j);
		}
		retval = dealer_sb_deck(id, t_p_r, (i + 1));
		if (retval)
			return retval;
	}

	t_d_deck_info = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(t_d_deck_info, bits256_str(str, d_deck_info.dealer_r[i].prod));
	}

	cJSON *out = append_cmm_from_id_key_data_cJSON(id, get_key_data_vdxf_id(T_D_DECK_KEY, game_id_str),
						       t_d_deck_info, true);
	if (!out)
		retval = ERR_DECK_BLINDING_DEALER;
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

int32_t handle_game_state(char *table_id)
{
	int32_t game_state, retval = OK;
	cJSON *out = NULL;

	game_state = get_game_state(table_id);
	switch (game_state) {
	case G_TABLE_STARTED:
		if (is_table_full(table_id))
			append_game_state(table_id, G_PLAYERS_JOINED, NULL);
		break;
	case G_PLAYERS_JOINED:
		if (is_players_shuffled_deck(table_id))
			append_game_state(table_id, G_DECK_SHUFFLING_P, NULL);
		break;
	case G_DECK_SHUFFLING_P:
		retval = dealer_shuffle_deck(table_id);
		append_game_state(table_id, G_DECK_SHUFFLING_D, NULL);
		break;
	case G_DECK_SHUFFLING_D:
		//Do nothing;
		break;
	default:
		dlg_info("%s", game_state_str(game_state));
	}
	return retval;
}

int32_t update_t_info_at_dealer(struct table t)
{
	int32_t retval = OK;
	cJSON *d_table_info = NULL, *out = NULL;

	d_table_info = get_cJSON_from_id_key(t.dealer_id, T_TABLE_INFO_KEY);
	if (d_table_info == NULL) {
		out = update_cmm_from_id_key_data_cJSON(t.dealer_id, get_vdxf_id(T_TABLE_INFO_KEY),
							struct_table_to_cJSON(&t), true);
		if (!out)
			retval = ERR_RESERVED;
	}
	return retval;
}

int32_t dealer_init(struct table t)
{
	int32_t retval = OK, game_state;
	cJSON *out = NULL;

	//Updating the dealer id with t_table_info
	retval = update_t_info_at_dealer(t);
	if (retval)
		return retval;

	game_state = get_game_state(t.table_id);
	if (game_state == G_ZEROIZED_STATE) {
		retval = dealer_table_init(t);
		if (retval)
			return retval;
	}

	while (1) {
		retval = handle_game_state(t.table_id);
		if (retval)
			return retval;
		sleep(2);
	}
	return retval;
}
