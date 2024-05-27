#include "bet.h"
#include "game.h"
#include "vdxf.h"
#include "common.h"
#include "err.h"

struct t_game_info_struct t_game_info;

const char *game_state_str(int32_t game_state)
{
	switch (game_state) {
	case G_ZEROIZED_STATE:
		return "Zeroized state, Table is not initialized yet...";
	case G_TABLE_ACTIVE:
		return "Table is active";
	case G_TABLE_STARTED:
		return "Table is started";
	case G_PLAYERS_JOINED:
		return "Players joined the table";
	case G_DECK_SHUFFLING_P:
		return "Deck shuffling by players are done";
	case G_DECK_SHUFFLING_D:
		return "Deck shuffling by dealer is done";
	case G_DECK_SHUFFLING_B:
		return "Deck shuffling by cashier is done";
	case G_REVEAL_CARD_B:
		return "Waiting for cashier to reveal blinding value";
	case G_REVEAL_CARD_P:
		return "Waiting for player(s) to reveal the card";
	case G_REVEAL_CARD_P_DONE:
		return "Player(s) got the card";
	case G_REVEAL_CARD:
		return "Drawing the card from deck";
	default:
		return "Invalid game state...";
	}
}

cJSON *append_game_state(char *table_id, int32_t game_state, cJSON *game_state_info)
{
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL, *out = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		dlg_warn("Porbabaly the table is not initialized");
		return NULL;
	}
	t_game_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(t_game_info, "game_state", game_state);
	if (game_state_info)
		cJSON_AddItemToObject(t_game_info, "game_state_info", game_state_info);

	out = append_cmm_from_id_key_data_cJSON(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str),
						t_game_info, true);
	return out;
}

int32_t get_game_state(char *table_id)
{
	int32_t game_state = G_ZEROIZED_STATE;
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str) {
		/*
			Game ID is NULL, it probably mean the table hasn't been started yet, so game state is in zeroized state.
		*/
		return game_state;
	}

	t_game_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
	if (!t_game_info)
		return game_state;

	game_state = jint(t_game_info, "game_state");
	return game_state;
}

cJSON *get_game_state_info(char *table_id)
{
	char *game_id_str = NULL;
	cJSON *t_game_info = NULL, *game_state_info = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	if (!game_id_str)
		return NULL;

	t_game_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_GAME_INFO_KEY, game_id_str));
	if (!t_game_info)
		return NULL;

	game_state_info = jobj(t_game_info, "game_state_info");
	return game_state_info;
}

void init_struct_vars()
{
	dcv_vars = calloc(1, sizeof(struct privatebet_vars));

	dcv_vars->turni = 0;
	dcv_vars->round = 0;
	dcv_vars->pot = 0;
	dcv_vars->last_turn = 0;
	dcv_vars->last_raise = 0;
	for (int i = 0; i < CARDS777_MAXPLAYERS; i++) {
		dcv_vars->funds[i] = 0;
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			dcv_vars->bet_actions[i][j] = 0;
			dcv_vars->betamount[i][j] = 0;
		}
	}
}

int32_t init_game_meta_info(char *table_id)
{
	int32_t retval = OK;
	char *game_id_str = NULL;
	cJSON *t_player_info = NULL;

	game_id_str = get_str_from_id_key(table_id, T_GAME_ID_KEY);
	t_player_info = get_cJSON_from_id_key_vdxfid(table_id, get_key_data_vdxf_id(T_PLAYER_INFO_KEY, game_id_str));

	game_meta_info.num_players = jint(t_player_info, "num_players");
	game_meta_info.dealer_pos = 0;
	game_meta_info.turn = (game_meta_info.dealer_pos + 1) % game_meta_info.num_players;
	game_meta_info.card_id = 0;

	for (int32_t i = 0; i < game_meta_info.num_players; i++) {
		for (int32_t j = 0; j < hand_size; j++) {
			game_meta_info.card_state[i][j] = no_card_drawn;
		}
	}
	return retval;
}

int32_t is_card_drawn(char *table_id)
{
	int32_t retval = OK, timeout = 0;
	cJSON *game_state_info = NULL, *player_game_state_info = NULL;

	game_state_info = get_game_state_info(table_id);

	// wait for 60 seconds for the player reaction
	while (timeout < 60) {
		timeout++;
		player_game_state_info = get_game_state_info(player_ids[jint(game_state_info, "player_id")]);
		if (!player_game_state_info) {
			sleep(2);
			continue;
		}
		if ((jint(game_state_info, "player_id") == jint(player_game_state_info, "player_id")) &&
		    (jint(game_state_info, "card_id") == jint(player_game_state_info, "card_id"))) {
			dlg_info("Drawn card details::%s", cJSON_Print(player_game_state_info));
			break;
		}
	}
}

static int32_t update_next_card(char *table_id, int32_t player_id, int32_t card_id, int32_t card_type)
{
	cJSON *game_state_info = NULL, *out = NULL;
	int32_t retval = OK;

	game_state_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(game_state_info, "player_id", player_id);
	cJSON_AddNumberToObject(game_state_info, "card_id", card_id);
	cJSON_AddNumberToObject(game_state_info, "card_type", card_type);

	dlg_info("%s", cJSON_Print(game_state_info));

	out = append_game_state(table_id, G_REVEAL_CARD, game_state_info);
	if (!out)
		retval = ERR_UPDATEIDENTITY;

	return retval;
}

int32_t deal_next_card(char *table_id)
{
	int32_t retval = OK;

	if (hole_cards_drawn == 0) {
		for (int i = 0; i < no_of_hole_cards; i++) {
			for (int j = 0; j < num_of_players; j++) {
				if (card_matrix[j][i] == 0) {
					retval = update_next_card(table_id, j, (i * num_of_players) + j, hole_card);
					goto end;
				}
			}
		}
	} else if (flop_cards_drawn == 0) {
		for (int i = no_of_hole_cards; i < no_of_hole_cards + no_of_flop_cards; i++) {
			for (int j = 0; j < num_of_players; j++) {
				if (card_matrix[j][i] == 0) {
					if ((i - (no_of_hole_cards)) == 0) {
						retval = update_next_card(table_id, j,
									  (no_of_hole_cards * num_of_players) +
										  (i - no_of_hole_cards) + 1,
									  flop_card_1);
					} else if ((i - (no_of_hole_cards)) == 1) {
						retval = update_next_card(table_id, j,
									  (no_of_hole_cards * num_of_players) +
										  (i - no_of_hole_cards) + 1,
									  flop_card_2);
					} else if ((i - (no_of_hole_cards)) == 2) {
						retval = update_next_card(table_id, j,
									  (no_of_hole_cards * num_of_players) +
										  (i - no_of_hole_cards) + 1,
									  flop_card_3);
					}
					goto end;
				}
			}
		}
	} else if (turn_card_drawn == 0) {
		for (int i = no_of_hole_cards + no_of_flop_cards;
		     i < no_of_hole_cards + no_of_flop_cards + no_of_turn_card; i++) {
			for (int j = 0; j < num_of_players; j++) {
				if (card_matrix[j][i] == 0) {
					retval = update_next_card(table_id, j,
								  (no_of_hole_cards * num_of_players) +
									  (i - no_of_hole_cards) + 2,
								  turn_card);
					goto end;
				}
			}
		}
	} else if (river_card_drawn == 0) {
		for (int i = no_of_hole_cards + no_of_flop_cards + no_of_turn_card;
		     i < no_of_hole_cards + no_of_flop_cards + no_of_turn_card + no_of_river_card; i++) {
			for (int j = 0; j < num_of_players; j++) {
				if (card_matrix[j][i] == 0) {
					retval = update_next_card(table_id, j,
								  (no_of_hole_cards * num_of_players) +
									  (i - no_of_hole_cards) + 3,
								  river_card);
					goto end;
				}
			}
		}
	} else
		retval = ERR_ALL_CARDS_DRAWN;
end:
	return retval;
}

int32_t init_game_state(char *table_id)
{
	int32_t retval = OK;

	init_struct_vars();
	retval = deal_next_card(table_id);
	return retval;
}
