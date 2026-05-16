#include "bet.h"
#include "misc.h"
#include "print.h"
#include "game.h"
#include "poker_vdxf.h"

void print_struct_table(struct table *t)
{
	if (t) {
		dlg_info("Table Info\n");
		dlg_info("max_players :: %d", t->max_players);
		dlg_info("big_blind :: %f", uint32_s_to_float(t->big_blind));
		dlg_info("min_stake :: %f", uint32_s_to_float(t->min_stake));
		dlg_info("max_stake :: %f", uint32_s_to_float(t->max_stake));
		dlg_info("table_id :: %s", t->table_id);
		dlg_info("dealer_id :: %s", t->dealer_id);
	}
}

void print_cashier_id(char *id)
{
	char *game_id = NULL;

	game_id = get_str_from_id_key_from_height(id, T_GAME_ID_KEY, 0);
	if (game_id) {
		dlg_info("game_id::%s", game_id);

		cJSON *temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(C_DISPUTE_RESULT_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", C_DISPUTE_RESULT_KEY, cJSON_Print(temp));

		/* Cashier-owned blinded decks (docs/TODO.md item 1.1).
		 * Cashier is the sole writer; players read directly from cashier id. */
		for (int32_t i = 0; i < all_t_b_p_keys_no; i++) {
			temp = get_cJSON_from_id_key_vdxfid_from_height(
				id, get_key_data_vdxf_id(all_c_b_p_keys[i], game_id), 0);
			if (temp)
				dlg_info("%s :: %s", all_c_b_p_key_names[i], cJSON_Print(temp));
		}

		/* Cashier-owned BV (docs/TODO.md item 1.2).
		 * Cashier publishes the latest (player_id, card_id, bv) here;
		 * players poll directly. */
		temp = get_cJSON_from_id_key_vdxfid_from_height(
			id, get_key_data_vdxf_id(C_CARD_BV_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", C_CARD_BV_KEY, cJSON_Print(temp));
	}
}

void print_dealers_id(char *id)
{
	cJSON *temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_vdxf_id(DEALERS_KEY), 0);
	if (temp)
		dlg_info("%s :: %s", DEALERS_KEY, cJSON_Print(temp));
}

void print_dealer_id(char *id)
{
	cJSON *temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_vdxf_id(T_TABLE_INFO_KEY), 0);
	if (temp)
		dlg_info("%s :: %s", T_TABLE_INFO_KEY, cJSON_Print(temp));

	temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_vdxf_id("registration_info"), 0);
	if (temp)
		dlg_info("registration_info :: %s", cJSON_Print(temp));
}

void print_table_id(char *id)
{
	char *game_id = NULL;

	game_id = get_str_from_id_key_from_height(id, T_GAME_ID_KEY, 0);
	if (game_id) {
		dlg_info("game_id::%s", game_id);

		cJSON *temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_TABLE_INFO_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", T_TABLE_INFO_KEY, cJSON_Print(temp));

		temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", T_PLAYER_INFO_KEY, cJSON_Print(temp));

		temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_SETTLEMENT_INFO_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", T_SETTLEMENT_INFO_KEY, cJSON_Print(temp));

		temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_BOARD_CARDS_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", T_BOARD_CARDS_KEY, cJSON_Print(temp));

		temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_BETTING_STATE_KEY, game_id), 0);
		if (temp)
			dlg_info("%s :: %s", T_BETTING_STATE_KEY, cJSON_Print(temp));

		for (int32_t i = 0; i < all_t_d_p_keys_no; i++) {
			cJSON *temp =
				get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(all_t_d_p_keys[i], game_id), 0);
			if (temp)
				dlg_info("%s :: %s", all_t_d_p_key_names[i], cJSON_Print(temp));
		}
		/* T_B_P*_DECK_KEY are no longer written to the table (docs/TODO.md
		 * item 1.1: cashier owns blinded-deck keys on its own id under
		 * C_B_P*_DECK_KEY). Inspect those via `print_id <cashier_id> cashiers`. */
		temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id), 0);
		if (temp) {
			int game_state = jint(temp, "game_state");
			dlg_info("Game State: %s", game_state_str(game_state));
		}

		/* T_CARD_BV_KEY is no longer written to the table (docs/TODO.md
		 * item 1.2: cashier owns BV on its own id under C_CARD_BV_KEY).
		 * Inspect it via `print_id <cashier_id> cashiers`. */
	}
}

void print_player_id(char *id)
{
	char *game_id = NULL;

	cJSON *join_req = get_cJSON_from_id_key_vdxfid_from_height(id, get_vdxf_id(P_JOIN_REQUEST_KEY), 0);
	if (join_req) {
		dlg_info("%s :: %s", P_JOIN_REQUEST_KEY, cJSON_Print(join_req));
	}

	game_id = get_str_from_id_key_from_height(id, T_GAME_ID_KEY, 0);
	if (game_id) {
		dlg_info("game_id::%s", game_id);

		const char *player_keys[] = {
			P_GAME_HISTORY_KEY,
			PLAYER_DECK_KEY,
			P_BETTING_ACTION_KEY,
			P_DECODED_CARD_KEY,
			P_DISPUTE_REQUEST_KEY
		};
		int num_keys = sizeof(player_keys) / sizeof(player_keys[0]);

		for (int i = 0; i < num_keys; i++) {
			cJSON *temp = get_cJSON_from_id_key_vdxfid_from_height(id, get_key_data_vdxf_id((char *)player_keys[i], game_id), 0);
			if (temp) {
				dlg_info("%s :: %s", player_keys[i], cJSON_Print(temp));
			}
		}
	}
}

void print_table_key_info(int argc, char **argv)
{
	char *game_id_str = NULL;
	cJSON *key_info = NULL;

	if (argc == 4) {
		game_id_str = poker_get_key_str(argv[2], get_vdxf_id(T_GAME_ID_KEY));
		if (argv[3]) {
			dlg_info("%s::%s", game_id_str, get_key_data_vdxf_id(argv[3], game_id_str));
			key_info = get_cJSON_from_id_key_vdxfid(argv[2], get_key_data_vdxf_id(argv[3], game_id_str));
			dlg_info("%s", cJSON_Print(key_info));
		}
	}
}

void print_id_info(int argc, char **argv)
{
	if (!is_id_exists(argv[2])) {
		dlg_info("ID doesn't exists\n");
	} else {
		if ((strcmp(argv[3], "t") == 0) || (strcmp(argv[3], "table") == 0)) {
			print_table_id(argv[2]);
		} else if ((strcmp(argv[3], "d") == 0) || (strcmp(argv[3], "dealer") == 0)) {
			print_dealer_id(argv[2]);
		} else if ((strcmp(argv[3], "p") == 0) || (strcmp(argv[3], "player") == 0)) {
			print_player_id(argv[2]);
		} else if (strcmp(argv[3], "dealers") == 0) {
			print_dealers_id(argv[2]);
		} else if ((strcmp(argv[3], "c") == 0) || (strcmp(argv[3], "cashier") == 0)) {
			print_cashier_id(argv[2]);
		} else {
			dlg_info("Print is not supported for this ID::%s of type::%s", argv[2], argv[3]);
		}
	}
}

void print_vdxf_info(int argc, char **argv)
{
	if (argc < 4 || !argv[2] || !argv[3]) {
		dlg_error("Invalid arguments for print_vdxf_info");
		return;
	}

	char *id = argv[2];
	char *key = argv[3];

	if (!is_id_exists(id)) {
		dlg_info("ID doesn't exist");
		return;
	}

	char *vdxf_id = get_key_vdxf_id(key);
	cJSON *json_data = NULL;
	char *str_data = NULL;

	if (strcmp(vdxf_id, get_vdxf_id(T_TABLE_INFO_KEY)) == 0 ||
	    strcmp(vdxf_id, get_vdxf_id(T_PLAYER_INFO_KEY)) == 0 || strcmp(vdxf_id, get_vdxf_id(DEALERS_KEY)) == 0) {
		json_data = get_cJSON_from_id_key_vdxfid(id, vdxf_id);
		if (json_data) {
			dlg_info("%s", cJSON_Print(json_data));
			cJSON_Delete(json_data);
		}
	} else if (strcmp(vdxf_id, get_vdxf_id(T_GAME_ID_KEY)) == 0) {
		str_data = poker_get_key_str(id, get_full_key(key));
		if (str_data) {
			dlg_info("%s", str_data);
			free(str_data);
		}
	} else if (strcmp(vdxf_id, get_vdxf_id(T_GAME_INFO_KEY)) == 0) {
		char *game_id_str = poker_get_key_str(id, T_GAME_ID_KEY);
		if (game_id_str) {
			dlg_info("Game ID: %s", game_id_str);
			json_data =
				get_cJSON_from_id_key_vdxfid(id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
			if (json_data) {
				int game_state = jint(json_data, "game_state");
				dlg_info("Game State: %s", game_state_str(game_state));
				cJSON_Delete(json_data);
			}
			free(game_id_str);
		}
	} else {
		dlg_info("Print operation is not supported for the given ID: %s and key: %s", id, key);
	}
}
