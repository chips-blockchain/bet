#include "bet.h"
#include "dealer.h"
#include "vdxf.h"
#include "deck.h"
#include "cards777.h"
#include "game.h"
#include "err.h"
#include "misc.h"
#include "commands.h"
#include "dealer_registration.h"

struct d_deck_info_struct d_deck_info;
struct game_meta_info_struct game_meta_info;

char all_t_d_p_keys[all_t_d_p_keys_no][128] = { T_D_DECK_KEY,    T_D_P1_DECK_KEY, T_D_P2_DECK_KEY, T_D_P3_DECK_KEY,
						T_D_P4_DECK_KEY, T_D_P5_DECK_KEY, T_D_P6_DECK_KEY, T_D_P7_DECK_KEY,
						T_D_P8_DECK_KEY, T_D_P9_DECK_KEY };

char all_t_d_p_key_names[all_t_d_p_keys_no][128] = { "t_d_deck",    "t_d_p1_deck", "t_d_p2_deck", "t_d_p3_deck",
						     "t_d_p4_deck", "t_d_p5_deck", "t_d_p6_deck", "t_d_p7_deck",
						     "t_d_p8_deck", "t_d_p9_deck" };

char all_game_keys[all_game_keys_no][128] = { T_GAME_INFO_KEY };

char all_game_key_names[all_game_keys_no][128] = { "t_game_info" };

int32_t num_of_players;
char player_ids[CARDS777_MAXPLAYERS][MAX_ID_LEN];

int32_t add_dealer(char *dealer_id)
{
	int32_t retval = OK;
	cJSON *dealers_info = NULL, *dealers = NULL, *out = NULL;

	if (!dealer_id) {
		return ERR_NULL_ID;
	}
	if (!is_id_exists(dealer_id, 0)) {
		return ERR_ID_NOT_FOUND;
	}

	if (!id_cansignfor(DEALERS_ID, 0, &retval)) {
		return retval;
	}

	dealers_info = cJSON_CreateObject();
	dealers = list_dealers();
	if (!dealers) {
		dealers = cJSON_CreateArray();
	}
	jaddistr(dealers, dealer_id);
	cJSON_AddItemToObject(dealers_info, "dealers", dealers);
	out = update_cmm_from_id_key_data_cJSON(DEALERS_ID, DEALERS_KEY, dealers_info, false);

	if (!out) {
		return ERR_UPDATEIDENTITY;
	}
	dlg_info("%s", cJSON_Print(out));

	return retval;
}

int32_t dealer_sb_deck(char *id, bits256 *player_r, int32_t player_id)
{
	int32_t retval = OK;
	char str[65], *game_id_str = NULL;
	cJSON *d_blinded_deck = NULL;

	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);

	dlg_info("Player::%d deck...", player_id);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}
	shuffle_deck_db(player_r, CARDS777_MAXCARDS, d_deck_info.d_permi);
	blind_deck_d(player_r, CARDS777_MAXCARDS, d_deck_info.dealer_r);

	dlg_info("Player::%d deck blinded with dealer secret key...", player_id);
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		dlg_info("%s", bits256_str(str, player_r[i]));
	}

	d_blinded_deck = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(d_blinded_deck, bits256_str(str, player_r[i]));
	}
	dlg_info("Updating Player ::%d blinded deck at the key ::%s by dealer..", player_id, all_t_d_p_keys[player_id]);
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

	switch (game_state) {
	case G_ZEROIZED_STATE:
		game_id = rand256(0);
		dlg_info("Updating %s key...", T_GAME_ID_KEY);
		out = append_cmm_from_id_key_data_hex(t.table_id, T_GAME_ID_KEY, bits256_str(hexstr, game_id), false);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		dlg_info("Updating Game state to %s...", game_state_str(G_TABLE_ACTIVE));
		out = append_game_state(t.table_id, G_TABLE_ACTIVE, NULL);
		if (!out)
			return ERR_GAME_STATE_UPDATE;
		dlg_info("%s", cJSON_Print(out));
		// No break is intentional
	case G_TABLE_ACTIVE:
		dlg_info("Updating %s key...", T_TABLE_INFO_KEY);
		out = append_cmm_from_id_key_data_cJSON(
			t.table_id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, bits256_str(hexstr, game_id)),
			struct_table_to_cJSON(&t), true);
		if (!out)
			return ERR_TABLE_LAUNCH;
		dlg_info("%s", cJSON_Print(out));

		dlg_info("Updating Game state to %s...", game_state_str(G_TABLE_STARTED));
		out = append_game_state(t.table_id, G_TABLE_STARTED, NULL);
		if (!out)
			return ERR_GAME_STATE_UPDATE;
		dlg_info("%s", cJSON_Print(out));
		break;
	default:
		dlg_info("Table is in game, at state ::%s", game_state_str(game_state));
	}
	return retval;
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
			if (G_DECK_SHUFFLING_P == get_game_state(player_ids[i]))
				count++;
		}
		if (count == num_players)
			return true;
	}
	return false;
}

int32_t dealer_shuffle_deck(char *id)
{
	int32_t retval = OK;
	char *game_id_str = NULL, str[65];
	cJSON *t_d_deck_info = NULL;
	bits256 t_p_r[CARDS777_MAXCARDS];

	dealer_init_deck();
	game_id_str = get_str_from_id_key(id, T_GAME_ID_KEY);

	for (int32_t i = 0; i < num_of_players; i++) {
		cJSON *player_deck =
			get_cJSON_from_id_key_vdxfid(player_ids[i], get_key_data_vdxf_id(PLAYER_DECK_KEY, game_id_str));
		cJSON *cardinfo = cJSON_GetObjectItem(player_deck, "cardinfo");
		for (int32_t j = 0; j < cJSON_GetArraySize(cardinfo); j++) {
			t_p_r[j] = jbits256i(cardinfo, j);
		}
		retval = dealer_sb_deck(id, t_p_r, (i + 1));
		if (retval)
			return retval;
	}

	t_d_deck_info = cJSON_CreateArray();
	for (int32_t i = 0; i < CARDS777_MAXCARDS; i++) {
		jaddistr(t_d_deck_info, bits256_str(str, d_deck_info.dealer_r[i].prod));
	}
	dlg_info("Updating the key :: %s, which contains public points of dealer blinded values..", T_D_DECK_KEY);
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
	cJSON *game_state_info = NULL;

	game_state = get_game_state(table_id);
	dlg_info("%s", game_state_str(game_state));
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
		if (!retval)
			append_game_state(table_id, G_DECK_SHUFFLING_D, NULL);
		break;
	case G_DECK_SHUFFLING_B:
		dlg_info("Its time for game");
		retval = init_game_state(table_id);
		break;
	case G_REVEAL_CARD:
		if (is_card_drawn(table_id) == OK) {
			dlg_info("Card is drawn");
			retval = verus_receive_card(table_id, dcv_vars);
		}
		break;
	}
	return retval;
}

int32_t register_table(struct table t)
{
	int32_t retval = OK;
	cJSON *d_table_info = NULL, *out = NULL;

	d_table_info = get_cJSON_from_id_key(t.dealer_id, T_TABLE_INFO_KEY, 0);
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
	double balance = 0;

	balance = chips_get_balance();
	if (balance < RESERVE_AMOUNT) {
		dlg_info("Wallet balance ::%f, Minimum funds needed to host a table :: %f", balance, RESERVE_AMOUNT);
		return ERR_CHIPS_INSUFFICIENT_FUNDS;
	}
	if ((!id_cansignfor(t.dealer_id, 0, &retval)) || (!id_cansignfor(t.table_id, 0, &retval))) {
		return retval;
	}

	if (!is_dealer_registered(t.dealer_id)) {
		// TODO:: An automated mechanism to register the dealer with dealers.poker.chips10sec need to be worked out
		return ERR_DEALER_UNREGISTERED;
	}

	if (is_table_registered(t.table_id, t.dealer_id)) {
		dlg_info("Table::%s is already registered with the dealer ::%s", t.table_id, t.dealer_id);
	} else {
		// TODO:: At the moment only one table we are registering with the dealer, if any other table exists it will be replaced with new table info
		retval = register_table(t);
		if (retval) {
			dlg_error("Table::%s, registration at dealer::%s is failed", t.table_id, t.dealer_id);
			return retval;
		}
	}

	retval = dealer_table_init(t);
	if (retval != OK) {
		dlg_info("Table Init is failed");
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
