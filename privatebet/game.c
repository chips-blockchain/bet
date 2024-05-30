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
	case G_REVEAL_CARD_P_DONE:
		return "Player(s) got the card";
	case G_REVEAL_CARD:
		return "Drawing the card from deck";
	case G_ROUND_BETTING:
		return "Round betting is happening...";
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
	no_of_cards = 0;
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
	int32_t retval = OK;
	cJSON *game_state_info = NULL, *player_game_state_info = NULL;

	// TODO:: Need to add block wait time and based on whcih dealer can take action on player
	game_state_info = get_game_state_info(table_id);
	dlg_info("Players verus ID::%s", player_ids[jint(game_state_info, "player_id")]);
	while (1) {
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
	return retval;
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

int32_t verus_receive_card(char *table_id, struct privatebet_vars *vars)
{
	int retval = OK, playerid, cardid, card_type, flag = 1;
	cJSON *game_state_info = NULL;

	game_state_info = get_game_state_info(table_id);

	playerid = jint(game_state_info, "player_id");
	cardid = jint(game_state_info, "card_id");
	card_type = jint(game_state_info, "card_type");

	no_of_cards++;

	dlg_info("no_of_cards drawn :: %d", no_of_cards);

	if (card_type == hole_card) {
		card_matrix[(cardid % num_of_players)][(cardid / num_of_players)] = 1;
		//card_values[(cardid % bet->maxplayers)][(cardid / bet->maxplayers)] = jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_1) {
		card_matrix[playerid][no_of_hole_cards] = 1;
		//card_values[playerid][no_of_hole_cards] = jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_2) {
		card_matrix[playerid][no_of_hole_cards + 1] = 1;
		//card_values[playerid][no_of_hole_cards + 1] = jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_3) {
		card_matrix[playerid][no_of_hole_cards + 2] = 1;
		//card_values[playerid][no_of_hole_cards + 2] = jint(player_card_info, "decoded_card");
	} else if (card_type == turn_card) {
		card_matrix[playerid][no_of_hole_cards + no_of_flop_cards] = 1;
		//card_values[playerid][no_of_hole_cards + no_of_flop_cards] = jint(player_card_info, "decoded_card");
	} else if (card_type == river_card) {
		card_matrix[playerid][no_of_hole_cards + no_of_flop_cards + no_of_turn_card] = 1;
		//card_values[playerid][no_of_hole_cards + no_of_flop_cards + no_of_turn_card] = jint(player_card_info, "decoded_card");
	}

	if (hole_cards_drawn == 0) {
		flag = 1;
		for (int i = 0; ((i < no_of_hole_cards) && (flag)); i++) {
			for (int j = 0; ((j < num_of_players) && (flag)); j++) {
				if (card_matrix[j][i] == 0) {
					flag = 0;
				}
			}
		}
		if (flag)
			hole_cards_drawn = 1;

	} else if (flop_cards_drawn == 0) {
		flag = 1;
		for (int i = no_of_hole_cards; ((i < no_of_hole_cards + no_of_flop_cards) && (flag)); i++) {
			for (int j = 0; ((j < num_of_players) && (flag)); j++) {
				if (card_matrix[j][i] == 0) {
					flag = 0;
				}
			}
		}
		if (flag)
			flop_cards_drawn = 1;

	} else if (turn_card_drawn == 0) {
		flag = 1;
		for (int i = no_of_hole_cards + no_of_flop_cards;
		     ((i < no_of_hole_cards + no_of_flop_cards + no_of_turn_card) && (flag)); i++) {
			for (int j = 0; ((j < num_of_players) && (flag)); j++) {
				if (card_matrix[j][i] == 0) {
					flag = 0;
				}
			}
		}
		if (flag)
			turn_card_drawn = 1;

	} else if (river_card_drawn == 0) {
		flag = 1;
		for (int i = no_of_hole_cards + no_of_flop_cards + no_of_turn_card;
		     ((i < no_of_hole_cards + no_of_flop_cards + no_of_turn_card + no_of_river_card) && (flag)); i++) {
			for (int j = 0; ((j < num_of_players) && (flag)); j++) {
				if (card_matrix[j][i] == 0) {
					flag = 0;
				}
			}
		}
		if (flag)
			river_card_drawn = 1;
	}

	if (flag) {
		if (vars->round == 0) {
			dlg_info("Initiate betting with small blind");
			//retval = bet_dcv_small_blind(NULL, vars);
			retval = verus_small_blind(table_id, vars);
		} else {
			//retval = bet_dcv_round_betting(NULL, vars);
		}
	} else {
		retval = deal_next_card(table_id);
		//retval = bet_dcv_turn(player_card_info, vars);
	}

	return retval;
}

int32_t verus_small_blind(char *table_id, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *smallBlindInfo = NULL, *out = NULL;

	vars->last_turn = vars->dealer;
	vars->turni = (vars->dealer) % num_of_players; // vars->dealer+1 is removed since
	// dealer is the one who does small_blind

	smallBlindInfo = cJSON_CreateObject();
	cJSON_AddStringToObject(smallBlindInfo, "method", "betting");
	cJSON_AddStringToObject(smallBlindInfo, "action", "small_blind");
	cJSON_AddNumberToObject(smallBlindInfo, "playerid", vars->turni);
	cJSON_AddNumberToObject(smallBlindInfo, "round", vars->round);
	cJSON_AddNumberToObject(smallBlindInfo, "pot", vars->pot);

	out = append_game_state(table_id, G_ROUND_BETTING, smallBlindInfo);
	dlg_info("%s", cJSON_Print(out));

	return retval;
}
