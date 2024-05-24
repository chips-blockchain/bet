#include "bet.h"
#include "common.h"
#include "player.h"
#include "err.h"
#include "vdxf.h"
#include "misc.h"
#include "commands.h"
#include "deck.h"
#include "game.h"
#include "config.h"
#include "print.h"

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

	dlg_info("Updating %s key...", T_GAME_ID_KEY);
	cJSON *out = append_cmm_from_id_key_data_hex(player_config.verus_pid, T_GAME_ID_KEY,
						     bits256_str(str, p_deck_info.game_id), false);

	dlg_info("Updating %s key...", PLAYER_DECK_KEY);
	out = append_cmm_from_id_key_data_cJSON(
		player_config.verus_pid, get_key_data_vdxf_id(PLAYER_DECK_KEY, bits256_str(str, p_deck_info.game_id)),
		player_deck, true);
	dlg_info("%s", cJSON_Print(out));

	dlg_info("Updating game state...");
	out = append_game_state(player_config.verus_pid, G_DECK_SHUFFLING_P, NULL);
	if (!out)
		return ERR_GAME_STATE_UPDATE;
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

int32_t decode_card(bits256 b_blinded_card, bits256 blinded_value, cJSON *dealer_blind_info)
{
	int32_t card_value = -1;
	char str1[65], str2[65];
	bits256 blinded_value_inv, d_blinded_card;

	blinded_value_inv = crecip_donna(blinded_value);
	d_blinded_card = fmul_donna(blinded_value_inv, b_blinded_card);

	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		for (int32_t j = 0; j < CARDS777_MAXCARDS; j++) {
			if (strcmp(bits256_str(str1, d_blinded_card),
				   bits256_str(str2, curve25519(p_deck_info.player_r[i].priv,
								jbits256i(dealer_blind_info, j)))) == 0) {
				card_value = p_deck_info.player_r[i].priv.bytes[30];
				dlg_info("card::%x\n", p_deck_info.player_r[i].priv.bytes[30]);
			}
		}
	}
	return card_value;
}

int32_t reveal_card(char *table_id)
{
	int32_t retval = OK, player_id, card_id, card_value = -1;
	char *game_id_str = NULL;
	cJSON *game_state_info = NULL, *bv_info = NULL, *b_blinded_deck = NULL, *dealer_blind_info = NULL;
	bits256 b_blinded_card, blinded_value;

	game_state_info = get_game_state_info(table_id);
	player_id = jint(game_state_info, "player_id");
	card_id = jint(game_state_info, "card_id");

	if ((player_id == player_config.player_id) || (player_id == -1)) {
		game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
		bv_info = cJSON_CreateArray();
		bv_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_B_DECK_BV_KEY, game_id_str));
		//dlg_info("bv_info::%s", cJSON_Print(bv_info));
		b_blinded_deck = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(all_t_b_p_keys[player_id],
											     game_id_str));
		b_blinded_card = jbits256i(b_blinded_deck, card_id);
		if (player_id == -1)
			blinded_value = jbits256i(bv_info, player_id);
		else
			blinded_value = jbits256i(bv_info, 0);

		//dlg_info("blinded_value::%s", bits256_str(str, blinded_value));
		//dlg_info("blinded_card::%s", bits256_str(str, b_blinded_card));
		dealer_blind_info =
			get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_D_DECK_KEY, game_id_str));
		//dlg_info("dealer_blind_info::%s", cJSON_Print(dealer_blind_info));
		card_value = decode_card(b_blinded_card, blinded_value, dealer_blind_info);
		if (card_value == -1) {
			retval = ERR_CARD_DECODING_FAILED;
		}
	}
	return retval;
}

int32_t handle_game_state_player(char *table_id)
{
	int32_t game_state, retval = OK;

	game_state = get_game_state(table_id);
	dlg_info("%s", game_state_str(game_state));
	switch (game_state) {
	case G_REVEAL_CARD_P:
		retval = reveal_card(table_id);
		if (!retval)
			append_game_state(table_id, G_REVEAL_CARD_P_DONE, NULL);
		break;
	default:
		dlg_info("%s", game_state_str(game_state));
	}
	return retval;
}

int32_t handle_verus_player()
{
	int32_t retval = OK;

	if ((retval = check_poker_ready()) != OK) {
		return retval;
	}

	if (retval = bet_parse_verus_player() != OK) {
		return retval;
	}

	if ((retval = find_table()) != OK) {
		// TODO:: If retval is ERR_PA_EXISTS, i.e PA exists in the table and the player can rejoin.
		return retval;
	}
	dlg_info("Table found");
	print_struct_table(&player_t);

	if ((retval = join_table()) != OK) {
		return retval;
	}
	dlg_info("Table Joined");

	if ((retval = get_player_id(&p_deck_info.player_id)) != OK) {
		return retval;
	}
	dlg_info("Player ID ::%d", p_deck_info.player_id);

	if ((retval = player_init_deck()) != OK) {
		return retval;
	}
	dlg_info("Player deck shuffling info updated to table");

	while (1) {
		handle_game_state_player(player_config.table_id);
		sleep(2);
	}

	return retval;
}
