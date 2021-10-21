/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
#include "states.h"
#include "bet.h"
#include "client.h"
#include "common.h"
#include "host.h"
#include "network.h"
#include "payment.h"
#include "table.h"

char action_str[8][100] = { "", "small_blind", "big_blind", "check", "raise", "call", "allin", "fold" };

/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/

int32_t bet_initiate_statemachine(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *dealerInfo = NULL;
	int32_t retval = 1, bytes;
	char *rendered = NULL;
	cJSON *temp = NULL;
	vars->turni = 0;
	vars->round = 0;
	vars->pot = 0;
	vars->last_turn = 0;
	vars->last_raise = 0;
	for (int i = 0; i < bet->maxplayers; i++) {
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			vars->bet_actions[i][j] = 0;
			vars->betamount[i][j] = 0;
		}
	}
	dealerInfo = cJSON_CreateObject();
	cJSON_AddStringToObject(dealerInfo, "method", "dealer");
	cJSON_AddNumberToObject(dealerInfo, "playerid", vars->dealer);

	temp = cJSON_CreateObject();
	temp = cJSON_Parse(cJSON_Print(dealerInfo));

	rendered = cJSON_Print(dealerInfo);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed");
		goto end;
	}

	bet_push_dcv_to_gui(temp);

end:
	return retval;
}

/**
 *
 * Function: BET_DCV_next_turn
 *
 * Parameters:
 *
 *		structure:  cJSON *argjson contains the information about
 *current active turn of the player structure:  struct privatebet_info *bet
 *contains deck and player information structure:  privatebet_vars *vars
 *contains the game information
 *
 * Return Value :
 *
 *		0 --> bet->maxplayers : It indicates there a valid turn exists
 *in the game. -1 : No valid turn exists
 *
 * Description:
 *
 *	  	This function identifies if there is any valid turn possible
 *during the game, if any valid turn exists it returns the index of the player
 *if not it returns -1
 *
 */

int32_t bet_dcv_next_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t maxamount = 0, retval = -1, players_left = 0;

	for (int i = 0; i < bet->maxplayers; i++) {
		if (vars->bet_actions[i][vars->round] == fold)
			players_left++;
	}
	players_left = bet->maxplayers - players_left;
	if (players_left < 2)
		return retval;

	for (int i = 0; i < bet->maxplayers; i++) {
		if (maxamount < vars->betamount[i][vars->round])
			maxamount = vars->betamount[i][vars->round];
	}
	for (int i = ((vars->turni + 1) % bet->maxplayers); (i != vars->turni); i = ((i + 1) % bet->maxplayers)) {
		if ((vars->bet_actions[i][vars->round] != fold) && (vars->bet_actions[i][vars->round] != allin) &&
		    (vars->funds[i] != 0)) {
			if (vars->bet_actions[i][vars->round] == 0) {
				for (int j = 0; j < bet->maxplayers; j++) {
					if ((i != j) &&
					    (((vars->bet_actions[j][vars->round] == 0) && (vars->funds[j] != 0)) ||
					     (vars->bet_actions[j][vars->round] != 0))) {
						return i;
					}
				}
			} else if (/*(vars->bet_actions[i][vars->round] == 0) ||*/ (vars->bet_actions[i][vars->round] ==
										    small_blind) ||
				   (vars->bet_actions[i][vars->round] == big_blind) ||
				   (((vars->bet_actions[i][vars->round] == check) ||
				     (vars->bet_actions[i][vars->round] == call) ||
				     (vars->bet_actions[i][vars->round] == raise)) &&
				    (maxamount != vars->betamount[i][vars->round]))) {
				return i;
			}
		}
	}
	return retval;
}
int32_t bet_dcv_round_betting(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *roundBetting = NULL, *possibilities = NULL, *actions = NULL, *betAmounts = NULL;
	int maxamount = 0, bytes, retval = 1, players_left = 0, toCall = 0, toRaise = 0, totalBet = 0;
	char *rendered = NULL;

	if ((retval = bet_dcv_next_turn(argjson, bet, vars)) == -1) {
		for (int i = 0; i < bet->maxplayers; i++) {
			if (vars->bet_actions[i][vars->round] == fold) //|| (vars->bet_actions[i][vars->round]==allin))
				players_left++;
		}
		players_left = bet->maxplayers - players_left;

		vars->round += 1;
		vars->turni = vars->dealer;
		vars->last_raise = 0;

		if ((vars->round >= CARDS777_MAXROUNDS) || (players_left < 2)) {
			vars->round -= 1;
			retval = bet_evaluate_hand(bet, vars);
			goto end;
		}

		retval = bet_dcv_turn(argjson, bet, vars);
		goto end;
	}

	players_left = 0;
	for (int i = 0; i < bet->maxplayers; i++) {
		if (vars->bet_actions[i][vars->round] == fold) //|| (vars->bet_actions[i][vars->round]==allin))
			players_left++;
	}
	players_left = bet->maxplayers - players_left;
	if (players_left < 2) {
		retval = bet_evaluate_hand(bet, vars);
		goto end;
	}

	vars->last_turn = vars->turni;
	vars->turni = bet_dcv_next_turn(argjson, bet, vars);

	roundBetting = cJSON_CreateObject();
	cJSON_AddStringToObject(roundBetting, "method", "betting");
	cJSON_AddStringToObject(roundBetting, "action", "round_betting");
	cJSON_AddNumberToObject(roundBetting, "playerid", vars->turni);
	cJSON_AddNumberToObject(roundBetting, "round", vars->round);
	cJSON_AddNumberToObject(roundBetting, "pot", vars->pot);
	/* */
	cJSON_AddItemToObject(roundBetting, "betAmounts", betAmounts = cJSON_CreateArray());
	for (int j = 0; j < bet->maxplayers; j++) {
		totalBet = 0;
		for (int i = 0; i <= vars->round; i++) {
			totalBet += vars->betamount[j][i];
		}
		cJSON_AddItemToArray(betAmounts, cJSON_CreateNumber(totalBet));
	}

	cJSON_AddItemToObject(roundBetting, "actions", actions = cJSON_CreateArray());
	for (int i = 0; i <= vars->round; i++) {
		for (int j = 0; j < bet->maxplayers; j++) {
			if (vars->bet_actions[j][i] > 0)
				cJSON_AddItemToArray(actions, cJSON_CreateNumber(vars->bet_actions[j][i]));
		}
	}

	cJSON_AddItemToObject(roundBetting, "possibilities", possibilities = cJSON_CreateArray());

	if ((vars->betamount[vars->last_turn][vars->round] == vars->betamount[vars->turni][vars->round]) &&
	    (vars->bet_actions[vars->turni][vars->round] == big_blind)) {
		toCall = vars->betamount[vars->turni][vars->round];
	} else {
		// raise, call, allin, fold
		int32_t max = 0;
		for (int32_t i = 0; i < bet->maxplayers; i++) {
			if (max < vars->betamount[i][vars->round])
				max = vars->betamount[i][vars->round];
		}
		toCall = max; //vars->betamount[vars->last_turn][vars->round];
	}

	if (vars->last_raise < big_blind_amount)
		toRaise = big_blind_amount;
	else
		toRaise = vars->last_raise;

	toRaise += toCall;

	cJSON_AddNumberToObject(roundBetting, "toCall", toCall);
	cJSON_AddNumberToObject(roundBetting, "minRaiseTo", toRaise);

	for (int i = 0; i < bet->maxplayers; i++) {
		if (maxamount < vars->betamount[i][vars->round]) {
			maxamount = vars->betamount[i][vars->round];
		}
	}

	if (maxamount > vars->betamount[vars->turni][vars->round]) {
		if (maxamount >= vars->funds[vars->turni]) {
			for (int i = allin; i <= fold; i++) {
				if (i == allin)
					cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(call));
				else
					cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
			}

		} else {
			int allin_flag = 1;
			for (int i = raise; i <= fold; i++) {
				if (i == raise) {
					for (int j = 0; j < bet->maxplayers; j++) {
						if ((j != vars->turni) && (vars->funds[j] != 0)) {
							cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
							break;
						}
					}
				} else if (i == call) {
					int totatBet = 0;
					for (int j = 0; j <= vars->round; j++)
						totatBet += vars->betamount[vars->turni][j];
					if (toCall == (totalBet + vars->funds[vars->turni])) {
						allin_flag = 0;
					}
					cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
				} else if (i == allin) {
					if (allin_flag == 1)
						cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
				} else
					cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
			}
		}

		// raise or fold or call
	} else if (maxamount == vars->betamount[vars->turni][vars->round]) {
		if (maxamount >= vars->funds[vars->turni]) {
			cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(check));
			for (int i = allin; i <= fold; i++)
				cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));

		} else {
			for (int i = check; i <= fold; i++) {
				if (i != call)
					cJSON_AddItemToArray(possibilities, cJSON_CreateNumber(i));
			}
		}
		// raise or fold or call or check
	}

	cJSON_AddNumberToObject(roundBetting, "min_amount", (maxamount - vars->betamount[vars->turni][vars->round]));
	rendered = cJSON_Print(roundBetting);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		goto end;
	}

end:
	return retval;
}

int32_t bet_dcv_round_betting_response(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, playerid, round, bet_amount = 0, players_left = 0, min_amount = 0;
	char *action = NULL;
	cJSON *playerFunds = NULL;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");
	bet_amount = jint(argjson, "invoice_amount");
	min_amount = jint(argjson, "min_amount");

	vars->betamount[playerid][round] += bet_amount;
	vars->pot += bet_amount;
	vars->funds[playerid] -= bet_amount;

	cJSON_AddNumberToObject(argjson, "pot", vars->pot);
	cJSON_AddItemToObject(argjson, "player_funds", playerFunds = cJSON_CreateArray());
	for (int i = 0; i < bet->maxplayers; i++) {
		cJSON_AddItemToArray(playerFunds, cJSON_CreateNumber(vars->funds[i]));
	}

	if ((action = jstr(argjson, "action")) != NULL) {
		if (strcmp(action, "check") == 0) {
			vars->bet_actions[playerid][round] = check;
		} else if (strcmp(action, "call") == 0) {
			vars->bet_actions[playerid][round] = call;
		} else if (strcmp(action, "raise") == 0) {
			vars->bet_actions[playerid][round] = raise;
			vars->last_raise = bet_amount - min_amount;
		} else if (strcmp(action, "allin") == 0) {
			vars->bet_actions[playerid][round] = allin;
		} else if (strcmp(action, "fold") == 0) {
			vars->bet_actions[playerid][round] = fold;
			for (int32_t i = round; i < CARDS777_MAXROUNDS; i++) {
				vars->bet_actions[playerid][i] = fold;
			}
		}

		for (int i = 0; i < bet->maxplayers; i++) {
			if (vars->bet_actions[i][round] == fold)
				players_left++;
		}
		players_left = bet->maxplayers - players_left;
		if (players_left < 2) {
			for (int i = 0; i < bet->maxplayers; i++) {
				for (int j = vars->round; j < CARDS777_MAXROUNDS; j++) {
					vars->bet_actions[i][j] = vars->bet_actions[i][round]; // check
				}
			}
		}
	}
	retval = bet_dcv_round_betting(argjson, bet, vars);

	return retval;
}

int32_t bet_dcv_big_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, playerid, round;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");

	vars->big_blind = jint(argjson, "amount");
	vars->bet_actions[playerid][round] = big_blind;
	vars->betamount[playerid][round] = vars->big_blind;
	vars->pot += vars->big_blind;
	vars->funds[playerid] -= vars->big_blind;

	retval = bet_dcv_round_betting(argjson, bet, vars);

	return retval;
}

int32_t bet_dcv_big_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *rendered = NULL;
	int32_t retval = 1, bytes;
	cJSON *big_blind_info = NULL;

	vars->last_turn = vars->turni;
	vars->turni = (vars->turni + 1) % bet->maxplayers;
	big_blind_info = cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info, "method", "betting");
	cJSON_AddStringToObject(big_blind_info, "action", "big_blind");
	cJSON_AddNumberToObject(big_blind_info, "playerid", vars->turni);
	cJSON_AddNumberToObject(big_blind_info, "min_amount", (vars->small_blind * 2));
	cJSON_AddNumberToObject(big_blind_info, "pot", vars->pot);

	rendered = cJSON_Print(big_blind_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed");
		goto end;
	}

end:
	return retval;
}

int32_t bet_dcv_small_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, playerid, round;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");

	vars->small_blind = jint(argjson, "amount");
	vars->bet_actions[playerid][round] = small_blind;
	vars->betamount[playerid][round] = vars->small_blind;
	vars->pot += vars->small_blind;
	vars->funds[playerid] -= vars->small_blind;

	retval = bet_dcv_big_blind(argjson, bet, vars);

	return retval;
}

int32_t bet_dcv_small_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *smallBlindInfo = NULL;
	int32_t retval = 1, bytes;
	char *rendered = NULL;

	vars->last_turn = vars->dealer;
	vars->turni = (vars->dealer) % bet->maxplayers; // vars->dealer+1 is removed since
	// dealer is the one who does small_blind

	smallBlindInfo = cJSON_CreateObject();
	cJSON_AddStringToObject(smallBlindInfo, "method", "betting");
	cJSON_AddStringToObject(smallBlindInfo, "action", "small_blind");
	cJSON_AddNumberToObject(smallBlindInfo, "playerid", vars->turni);
	cJSON_AddNumberToObject(smallBlindInfo, "round", vars->round);
	cJSON_AddNumberToObject(smallBlindInfo, "pot", vars->pot);
	rendered = cJSON_Print(smallBlindInfo);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed");
		goto end;
	}

end:
	return retval;
}
/***************************************************************
Here contains the functions which are common across all the nodes
****************************************************************/

int32_t bet_player_betting_statemachine(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *action = NULL;
	char *rendered = NULL;
	int retval = 1, bytes;

	if ((action = jstr(argjson, "action")) != 0) {
		if (strcmp(action, "small_blind") == 0) {
			if (jint(argjson, "playerid") == bet->myplayerid) {
				display_cards();
				retval = bet_player_small_blind(argjson, bet, vars);
			}
		} else if (strcmp(action, "big_blind") == 0) {
			if (jint(argjson, "playerid") == bet->myplayerid) {
				display_cards();
				retval = bet_player_big_blind(argjson, bet, vars);
			}
		} else if (strcmp(action, "small_blind_bet") == 0) {
			if (bet->myplayerid == -2) {
				bet_relay(argjson, bet, vars);
				retval = bet_dcv_small_blind_bet(argjson, bet, vars);
			} else {
				bet_player_small_blind_bet(argjson, bet, vars);
			}
		} else if (strcmp(action, "big_blind_bet") == 0) {
			if (bet->myplayerid == -2) {
				bet_relay(argjson, bet, vars);
				retval = bet_dcv_big_blind_bet(argjson, bet, vars);
			} else {
				retval = bet_player_big_blind_bet(argjson, bet, vars);
			}

		} else if (strcmp(action, "round_betting") == 0) {
			display_cards();

			cJSON *player_funds = NULL;
			cJSON_AddItemToObject(argjson, "player_funds", player_funds = cJSON_CreateArray());

			for (int i = 0; i < bet->maxplayers; i++) {
				int totalBet = 0;
				for (int j = 0; j <= jint(argjson, "round"); j++) // for(int j=0;j<=vars->round;j++)
				{
					totalBet += vars->betamount[i][j];
				}
				cJSON_AddItemToArray(player_funds, cJSON_CreateNumber(vars->funds[i] - totalBet));
			}

			player_lws_write(argjson);
		} else if ((strcmp(action, "check") == 0) || (strcmp(action, "call") == 0) ||
			   (strcmp(action, "raise") == 0) || (strcmp(action, "fold") == 0) ||
			   (strcmp(action, "allin") == 0)) {
			if (bet->myplayerid == -2) {
				rendered = cJSON_Print(argjson);
				bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
				if (bytes < 0) {
					retval = -1;
					goto end;
				}
				retval = bet_dcv_round_betting_response(argjson, bet, vars);
			} else {
				retval = bet_player_round_betting_response(argjson, bet, vars);
			}
		}
	}
end:
	return retval;
}

int32_t bet_display_current_state(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1;

	dlg_info("small_blind:%d", vars->small_blind);
	dlg_info("big_blind:%d", vars->big_blind);

	dlg_info("Display Actions:");

	for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
		dlg_info("Round:%d\n", j);
		for (int i = 0; i < bet->maxplayers; i++) {
			if (vars->bet_actions[i][j] == small_blind) {
				dlg_info("small blind ");
			} else if (vars->bet_actions[i][j] == big_blind) {
				dlg_info("big blind ");
			} else if (vars->bet_actions[i][j] == check) {
				dlg_info("raise ");
			} else if (vars->bet_actions[i][j] == raise) {
				dlg_info("check ");
			} else if (vars->bet_actions[i][j] == call) {
				dlg_info("call ");
			} else if (vars->bet_actions[i][j] == fold) {
				dlg_info("fold ");
			}
			dlg_info("%d ", vars->betamount[i][j]);
		}
	}

	return retval;
}

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/
int32_t bet_player_small_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, playerid, round;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");

	vars->turni = (vars->turni + 1) % bet->maxplayers;
	vars->small_blind = jint(argjson, "amount");
	vars->bet_actions[playerid][round] = small_blind;
	vars->betamount[playerid][round] = vars->small_blind;
	vars->pot += vars->small_blind;

	return retval;
}

int32_t bet_player_big_blind_bet(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, playerid, round;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");

	vars->turni = (vars->turni + 1) % bet->maxplayers;
	vars->big_blind = jint(argjson, "amount");
	vars->bet_actions[playerid][round] = big_blind;
	vars->betamount[playerid][round] = vars->big_blind;
	vars->pot += vars->big_blind;

	return retval;
}

int32_t bet_player_dealer_info(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 0, bytes;
	cJSON *dealerReady = NULL;
	char *rendered = NULL;

	vars->dealer = jint(argjson, "playerid");
	vars->turni = vars->dealer;
	vars->pot = 0;

	for (int i = 0; i < bet->maxplayers; i++) {
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			vars->bet_actions[i][j] = 0;
		}
	}
	for (int i = 0; i < bet->maxplayers; i++) {
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			vars->betamount[i][j] = 0;
		}
	}

	if (vars->dealer == bet->myplayerid) {
		dlg_info("I AM NEXT TO THE DEALER: %d\n", bet->myplayerid);
		dealerReady = cJSON_CreateObject();
		cJSON_AddStringToObject(dealerReady, "method", "dealer_ready");
		rendered = cJSON_Print(dealerReady);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			retval = -1;
			dlg_error("nn_send failed");
			goto end;
		}
	}
	bet_push_client(argjson);
end:
	return retval;
}

int32_t bet_player_small_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *small_blind_info = NULL;
	int32_t amount, retval = 1, bytes;
	char *rendered = NULL;
	cJSON *temp = NULL;

	small_blind_info = cJSON_CreateObject();
	cJSON_AddStringToObject(small_blind_info, "method", "betting");
	amount = small_blind_amount;
	vars->player_funds -= amount;
	retval = bet_player_invoice_pay(argjson, bet, vars, amount);

	if (retval < 0)
		goto end;

	cJSON_AddStringToObject(small_blind_info, "action", "small_blind_bet");
	cJSON_AddNumberToObject(small_blind_info, "amount", amount);
	vars->betamount[bet->myplayerid][vars->round] = vars->betamount[bet->myplayerid][vars->round] + amount;
	cJSON_AddNumberToObject(small_blind_info, "playerid", jint(argjson, "playerid"));
	cJSON_AddNumberToObject(small_blind_info, "round", jint(argjson, "round"));

	temp = cJSON_CreateObject();
	temp = cJSON_Parse(cJSON_Print(small_blind_info));

	rendered = cJSON_Print(small_blind_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed");
		goto end;
	}
	player_lws_write(temp);

end:
	return retval;
}

int32_t bet_player_big_blind(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *big_blind_info = NULL, *temp = NULL;
	int32_t amount, retval = 1, bytes;
	char *rendered = NULL;

	big_blind_info = cJSON_CreateObject();
	cJSON_AddStringToObject(big_blind_info, "method", "betting");
	cJSON_AddStringToObject(big_blind_info, "action", "big_blind_bet");

	amount = big_blind_amount;
	vars->player_funds -= amount;

	retval = bet_player_invoice_pay(argjson, bet, vars, amount);
	if (retval < 0)
		goto end;

	cJSON_AddNumberToObject(big_blind_info, "amount", amount);
	vars->betamount[bet->myplayerid][vars->round] = vars->betamount[bet->myplayerid][vars->round] + amount;
	cJSON_AddNumberToObject(big_blind_info, "playerid", jint(argjson, "playerid"));
	cJSON_AddNumberToObject(big_blind_info, "round", jint(argjson, "round"));

	temp = cJSON_CreateObject();
	temp = cJSON_Parse(cJSON_Print(big_blind_info));
	rendered = cJSON_Print(big_blind_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed");
		goto end;
	}
	player_lws_write(temp);

end:
	return retval;
}

int32_t bet_player_round_betting(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *possibilities = NULL, *action_response = NULL;
	int bytes, retval = 1, playerid, round, min_amount, option, raise_amount = 0, invoice_amount = 0;
	char *rendered = NULL;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");
	min_amount = jint(argjson, "min_amount");

	action_response = cJSON_CreateObject();
	cJSON_AddStringToObject(action_response, "method", "betting");
	cJSON_AddNumberToObject(action_response, "playerid", jint(argjson, "playerid"));
	cJSON_AddNumberToObject(action_response, "round", jint(argjson, "round"));
	cJSON_AddNumberToObject(action_response, "min_amount", min_amount);
	possibilities = cJSON_GetObjectItem(argjson, "possibilities");

	option = 1;
	vars->bet_actions[playerid][round] = jinti(possibilities, (option - 1));

	cJSON_AddStringToObject(action_response, "action", action_str[jinti(possibilities, (option - 1))]);

	if (jinti(possibilities, (option - 1)) == raise) {
		raise_amount = jint(argjson, "bet_amount");
		invoice_amount = raise_amount - vars->betamount[playerid][round];

		vars->betamount[playerid][round] += invoice_amount;
		vars->player_funds -= invoice_amount;

		if (vars->player_funds == 0) {
			cJSON_DetachItemFromObject(action_response, "action");
			cJSON_AddStringToObject(action_response, "action", "allin");
		}
		cJSON_AddNumberToObject(action_response, "bet_amount", jint(argjson, "bet_amount"));

		cJSON_AddNumberToObject(action_response, "invoice_amount", invoice_amount);

		retval = bet_player_invoice_request(argjson, action_response, bet, invoice_amount);
		if (retval < 0)
			goto end;

	} else if (jinti(possibilities, (option - 1)) == call) {
		vars->betamount[playerid][round] += min_amount;
		vars->player_funds -= min_amount;

		if (vars->player_funds == 0) {
			cJSON_DetachItemFromObject(action_response, "action");
			cJSON_AddStringToObject(action_response, "action", "allin");
			dlg_info("action response :: %s\n", cJSON_Print(action_response));
		}

		cJSON_AddNumberToObject(action_response, "bet_amount", jint(argjson, "bet_amount"));

		cJSON_AddNumberToObject(action_response, "invoice_amount", min_amount);

		retval = bet_player_invoice_request(argjson, action_response, bet, min_amount);
		if (retval < 0) {
			goto end;
		}

	} else if (jinti(possibilities, (option - 1)) == allin) {
		vars->betamount[playerid][round] += vars->player_funds;
		cJSON_AddNumberToObject(action_response, "bet_amount", jint(argjson, "bet_amount"));

		cJSON_AddNumberToObject(action_response, "invoice_amount", vars->player_funds);
		retval = bet_player_invoice_request(argjson, action_response, bet, vars->player_funds);
		vars->player_funds = 0;

		if (retval < 0)
			goto end;

	} else {
		dlg_info("action response :: %s\n", cJSON_Print(action_response));
		rendered = cJSON_Print(action_response);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

		if (bytes < 0) {
			retval = -1;
			dlg_error("nn_send failed");
			goto end;
		}
	}
end:
	return retval;
}

int32_t bet_player_round_betting_response(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, playerid, round, invoice_amount = 0;
	char *action = NULL;
	cJSON *playerFunds = NULL;

	playerid = jint(argjson, "playerid");
	round = jint(argjson, "round");
	invoice_amount = jint(argjson, "invoice_amount");

	if (bet->myplayerid != playerid)
		vars->betamount[playerid][round] += invoice_amount;

	vars->pot += invoice_amount;

	cJSON_AddNumberToObject(argjson, "pot", vars->pot);
	cJSON_AddItemToObject(argjson, "player_funds", playerFunds = cJSON_CreateArray());
	for (int i = 0; i < bet->maxplayers; i++) {
		int totalBet = 0;
		for (int j = 0; j <= vars->round; j++) {
			totalBet += vars->betamount[i][j];
		}
		cJSON_AddItemToArray(playerFunds, cJSON_CreateNumber(vars->funds[i] - totalBet));
	}

	if ((action = jstr(argjson, "action")) != NULL) {
		if (strcmp(action, "check") == 0) {
			vars->bet_actions[playerid][round] = check;
		} else if (strcmp(action, "call") == 0) {
			vars->bet_actions[playerid][round] = call;
		} else if (strcmp(action, "raise") == 0) {
			vars->bet_actions[playerid][round] = raise;
		} else if (strcmp(action, "fold") == 0) {
			vars->bet_actions[playerid][round] = fold;
		} else if (strcmp(action, "allin") == 0) {
			vars->bet_actions[playerid][round] = allin;
		}
	}
	bet_push_client(argjson);

	return retval;
}
