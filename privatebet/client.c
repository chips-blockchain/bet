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
#define _POSIX_C_SOURCE 200809L /* For pclose, popen, strdup */

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "../includes/cJSON.h"
#include "../includes/ppapi/c/pp_stdint.h"
#include "../log/macrologger.h"
#include "bet.h"
#include "cards777.h"
#include "client.h"
#include "commands.h"
#include "common.h"
#include "gfshare.h"
#include "network.h"
#include "payment.h"
#include "states.h"
#include "table.h"
#include "storage.h"
#include "cashier.h"
#include "misc.h"
#include "host.h"
#include "config.h"
#include "err.h"
#include "switchs.h"

#define LWS_PLUGIN_STATIC

struct lws *wsi_global_client = NULL, *wsi_global_client_write = NULL;

int ws_connection_status = 0, ws_connection_status_write = 0;

struct lws *wsi_global_bvv = NULL;

double max_allowed_dcv_commission = 2;

int32_t player_card_matrix[hand_size];
int32_t player_card_values[hand_size];
int32_t number_cards_drawn = 0;

int32_t sharesflag[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t data_exists = 0;
char player_gui_data[8196];

char player_payin_txid[100];
struct deck_player_info player_info;
struct deck_bvv_info bvv_info;
int32_t no_of_shares = 0;
int32_t player_cards[CARDS777_MAXCARDS];
int32_t no_of_player_cards = 0;

int32_t player_id = 0;
int32_t player_joined = 0;

bits256 all_v_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXCARDS];
bits256 all_g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
int32_t all_sharesflag[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t all_player_card_values[CARDS777_MAXPLAYERS][hand_size];
int32_t all_number_cards_drawn[CARDS777_MAXPLAYERS];
int32_t all_no_of_player_cards[CARDS777_MAXPLAYERS];
bits256 all_playershares[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

struct enc_share *all_g_shares[CARDS777_MAXPLAYERS];

struct privatebet_info *bet_bvv = NULL;
struct privatebet_vars *bvv_vars = NULL;

struct privatebet_info *BET_player[CARDS777_MAXPLAYERS] = { NULL };
struct deck_player_info all_players_info[CARDS777_MAXPLAYERS];

char lws_buf_1[65536];
int32_t lws_buf_length_1 = 0;
char lws_buf_bvv[2000];
int32_t lws_buf_length_bvv = 0;

char req_identifier[65];
int32_t backend_status = backend_not_ready;
int32_t sitout_value = 0;
int32_t reset_lock = 0;

struct lws_context_creation_info lws_player_info, lws_player_info_read, lws_player_info_write;
struct lws_context *player_context = NULL, *player_context_read = NULL, *player_context_write = NULL;

void write_to_GUI(void *ptr)
{
	cJSON *data = ptr;

	dlg_info("%s\n", cJSON_Print(data));
	if (ws_connection_status == 1) {
		memset(player_gui_data, 0, sizeof(player_gui_data));
		strncpy(player_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
		data_exists = 1;
		lws_callback_on_writable(wsi_global_client);
	}
}

void player_lws_write(cJSON *data)
{
	if (backend_status == backend_ready) {
		if (ws_connection_status_write == 1) {
			if (data_exists == 1) {
				while (data_exists == 1) {
					sleep(1);
				}
			}
			memset(player_gui_data, 0, sizeof(player_gui_data));
			strncpy(player_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
			data_exists = 1;
			lws_callback_on_writable(wsi_global_client_write);
		} else {
			dlg_warn("Backend is ready, but GUI is not started yet...");
		}
	} else {
		dlg_warn("Backend is not ready to write data to the GUI");
	}
}

char *enc_share_str(char hexstr[177], struct enc_share x)
{
	init_hexbytes_noT(hexstr, x.bytes, sizeof(x));
	return (hexstr);
}

struct enc_share get_API_enc_share(cJSON *obj)
{
	struct enc_share hash;
	char *str = NULL;
	memset(hash.bytes, 0, sizeof(hash));
	if (obj != 0) {
		if (is_cJSON_String(obj) != 0 && (str = obj->valuestring) != 0 && strlen(str) == 176) {
			decode_hex(hash.bytes, sizeof(hash), str);
		}
	}

	return (hash);
}

int32_t bet_bvv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	char str[65], enc_str[177];
	cJSON *cjson_dcv_blind_cards = NULL, *cjson_peer_pubkeys = NULL, *bvv_init_info = NULL,
	      *cjson_bvv_blind_cards = NULL, *cjson_shamir_shards = NULL;
	bits256 dcv_blind_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS], peer_pubkeys[CARDS777_MAXPLAYERS];
	bits256 bvv_blinding_values[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 bvv_blind_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];

	bvv_info.deckid = jbits256(argjson, "deckid");
	bvv_info.bvv_key.priv = curve25519_keypair(&bvv_info.bvv_key.prod);
	cjson_peer_pubkeys = cJSON_GetObjectItem(argjson, "peerpubkeys");
	cjson_dcv_blind_cards = cJSON_GetObjectItem(argjson, "dcvblindcards");

	for (uint32_t i = 0; i < bvv_info.maxplayers; i++) {
		peer_pubkeys[i] = jbits256i(cjson_peer_pubkeys, i);
		for (int j = 0; j < bet->range; j++) {
			dcv_blind_cards[i][j] = jbits256i(cjson_dcv_blind_cards, i * bet->range + j);
		}
	}
	g_shares = (struct enc_share *)malloc(CARDS777_MAXPLAYERS * CARDS777_MAXPLAYERS * CARDS777_MAXCARDS *
					      sizeof(struct enc_share));

	for (uint32_t i = 0; i < bvv_info.maxplayers; i++) {
		p2p_bvv_init(peer_pubkeys, bvv_info.bvv_key, bvv_blinding_values[i], bvv_blind_cards[i],
			     dcv_blind_cards[i], bet->range, bvv_info.numplayers, i, bvv_info.deckid);
	}

	bvv_init_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_init_info, "method", "init_b");
	jaddbits256(bvv_init_info, "bvvpubkey", bvv_info.bvv_key.prod);
	cJSON_AddItemToObject(bvv_init_info, "bvvblindcards", cjson_bvv_blind_cards = cJSON_CreateArray());
	for (uint32_t i = 0; i < bvv_info.numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			cJSON_AddItemToArray(cjson_bvv_blind_cards,
					     cJSON_CreateString(bits256_str(str, bvv_blind_cards[i][j])));
		}
	}
	cJSON_AddItemToObject(bvv_init_info, "shamirshards", cjson_shamir_shards = cJSON_CreateArray());
	int k = 0;
	for (uint32_t playerid = 0; playerid < bvv_info.numplayers; playerid++) {
		for (int i = 0; i < bet->range; i++) {
			for (uint32_t j = 0; j < bvv_info.numplayers; j++) {
				cJSON_AddItemToArray(cjson_shamir_shards,
						     cJSON_CreateString(enc_share_str(enc_str, g_shares[k++])));
			}
		}
	}
	retval = (nn_send(bet->pushsock, cJSON_Print(bvv_init_info), strlen(cJSON_Print(bvv_init_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

static int32_t bet_bvv_join_init(struct privatebet_info *bet)
{
	int32_t retval = OK;
	cJSON *bvv_response_info = NULL;

	bvv_response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_response_info, "method", "bvv_join");
	dlg_info("BVV Response Info::%s\n", cJSON_Print(bvv_response_info));
	retval = (nn_send(bet->pushsock, cJSON_Print(bvv_response_info), strlen(cJSON_Print(bvv_response_info)), 0) <
		  0) ?
			 ERR_NNG_SEND :
			 OK;

	return retval;
}

int32_t bet_check_bvv_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *bvv_ready = NULL;

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "bvv_ready");

	dlg_info("BVV ready info::%s\n", cJSON_Print(bvv_ready));
	retval = (nn_send(bet->pushsock, cJSON_Print(bvv_ready), strlen(cJSON_Print(bvv_ready)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

void bet_bvv_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	bet_permutation(bvv_info.permis, bet->range);
	for (int i = 0; i < bet->range; i++) {
		permis_b[i] = bvv_info.permis[i];
	}
	if (g_shares)
		free(g_shares);
}

void bet_bvv_backend_loop(void *_ptr)
{
	int32_t recvlen, retval = OK;
	cJSON *argjson = NULL;
	void *ptr = NULL;
	struct privatebet_info *bet = _ptr;
	struct privatebet_vars *VARS = NULL;

	VARS = calloc(1, sizeof(*VARS));
	bet_permutation(bvv_info.permis, bet->range);
	for (int i = 0; i < bet->range; i++) {
		permis_b[i] = bvv_info.permis[i];
	}
	bet_bvv_join_init(bet);
	while ((bet->pushsock >= 0 && bet->subsock >= 0) && (bvv_state == 1)) {
		ptr = 0;
		if ((recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0)) > 0) {
			char *tmp = clonestr(ptr);
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				if ((retval = bet_bvv_backend(argjson, bet, bvv_vars)) != OK) {
					dlg_error("%s", bet_err_str(retval));
				} else {
					if (strcmp(jstr(argjson, "method"), "init_d") == 0) {
						bet_bvv_reset(bet, bvv_vars);
						dlg_info("The role of bvv is done for this hand\n");
						bvv_state = 0;
						break;
					}
				}
			}
		}
	}
}

int32_t bet_bvv_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *method;
	int32_t retval = OK;

	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("received message::%s\n", method);
		if (strcmp(method, "init_d") == 0) {
			retval = bet_bvv_init(argjson, bet, vars);
		} else if (strcmp(method, "bvv_join") == 0) {
			retval = bet_bvv_join_init(bet);
		} else if (strcmp(method, "check_bvv_ready") == 0) {
			retval = bet_check_bvv_ready(argjson, bet, vars);
		} else if (strcmp(method, "reset") == 0) {
			bet_bvv_reset(bet, vars);
			retval = bet_bvv_join_init(bet);
		} else if (strcmp(method, "config_data") == 0) {
			max_players = jint(argjson, "max_players");
			chips_tx_fee = jdouble(argjson, "chips_tx_fee");
			bet->maxplayers = max_players;
			bet->numplayers = max_players;
			bvv_info.maxplayers = bet->maxplayers;
			bvv_info.numplayers = bet->numplayers;
		}
	}
	return retval;
}

bits256 bet_decode_card(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int32_t cardid,
			int32_t *retval)
{
	int32_t numplayers, M, flag = 0;
	bits256 recover, decoded, refval, tmp, xoverz, hash, fe, basepoint;
	uint8_t **shares;
	char str[65];

	numplayers = bet->maxplayers;
	shares = calloc(numplayers, sizeof(uint8_t *));
	for (int i = 0; i < numplayers; i++)
		shares[i] = calloc(sizeof(bits256), sizeof(uint8_t));

	M = (numplayers / 2) + 1;
	for (int i = 0; i < M; i++) {
		memcpy(shares[i], playershares[cardid][i].bytes, sizeof(bits256));
	}
	gfshare_calc_sharenrs(sharenrs, numplayers, player_info.deckid.bytes,
			      sizeof(player_info.deckid)); // same for all players for this round

	gfshare_recoverdata(shares, sharenrs, M, recover.bytes, sizeof(bits256), M);

	gfshare_recoverdata(shares, sharenrs, M, recover.bytes, sizeof(bits256), M);
	refval = fmul_donna(player_info.bvvblindcards[bet->myplayerid][cardid], crecip_donna(recover));

	// dlg_info("DCV blinded card:%s",bits256_str(str,refval));

	for (int i = 0; i < bet->range; i++) {
		for (int j = 0; j < bet->range; j++) {
			bits256 temp = xoverz_donna(curve25519(player_info.player_key.priv,
							       curve25519(player_info.cardprivkeys[i],
									  player_info.cardprods[bet->myplayerid][j])));
			vcalc_sha256(0, v_hash[i][j].bytes, temp.bytes, sizeof(temp));
		}
	}

	basepoint = curve25519_basepoint9();
	for (int i = 0; i < bet->range; i++) {
		for (int j = 0; j < bet->range; j++) {
			if (bits256_cmp(v_hash[i][j], g_hash[bet->myplayerid][cardid]) == 0) {
				for (int m = 0; m < bet->range; m++) {
					for (int n = 0; n < bet->range; n++) {
						tmp = curve25519(player_info.player_key.priv,
								 curve25519(player_info.cardprivkeys[m],
									    player_info.cardprods[bet->myplayerid][n]));
						xoverz = xoverz_donna(tmp);
						vcalc_sha256(0, hash.bytes, xoverz.bytes, sizeof(xoverz));

						fe = crecip_donna(curve25519_fieldelement(hash));

						decoded = curve25519(fmul_donna(refval, fe), basepoint);
						for (int k = 0; k < bet->range; k++) {
							if (bits256_cmp(decoded,
									player_info.cardprods[bet->myplayerid][k]) ==
							    0) {
								player_cards[no_of_player_cards] =
									atoi(bits256_str(str, decoded));
								no_of_player_cards++;
								tmp = player_info.cardprivkeys[m];
								flag = 1;
								goto end;
							}
						}
					}
				}
			}
		}
	}

end:
	if (!flag)
		*retval = ERR_CARD_RETRIEVING_USING_SS;

	return tmp;
}

int32_t ln_pay_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, player_id, retval = OK;
	char **argv = NULL, *invoice = NULL;
	cJSON *invoice_info = NULL, *pay_response = NULL;

	player_id = jint(argjson, "playerID");
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);

	if (player_id == bet->myplayerid) {
		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "lightning-cli", "pay", jstr(invoice_info, "bolt11"));
		pay_response = cJSON_CreateObject();
		retval = make_command(argc, argv, &pay_response);
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

static int32_t bet_player_betting_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, player_id, retval = OK;
	char **argv = NULL, *invoice = NULL;
	cJSON *invoice_info = NULL, *pay_response = NULL;
	cJSON *action_response = NULL;

	player_id = jint(argjson, "playerID");
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);
	if (player_id == bet->myplayerid) {
		argc = 3;
		bet_alloc_args(argc, &argv);
		argv = bet_copy_args(argc, "lightning-cli", "pay", jstr(invoice_info, "bolt11"));
		pay_response = cJSON_CreateObject();
		retval = make_command(argc, argv, &pay_response);
		if (retval != OK) {
			dlg_error("%s", cJSON_Print(pay_response));
		}

		action_response = cJSON_CreateObject();
		action_response = cJSON_GetObjectItem(argjson, "actionResponse");
		retval = (nn_send(bet->pushsock, cJSON_Print(action_response), strlen(cJSON_Print(action_response)),
				  0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

static int32_t bet_player_winner(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, retval = OK;
	char **argv = NULL, hexstr[65], params[arg_size];
	cJSON *invoice_info = NULL, *winner_invoice_info = NULL;

	if (jint(argjson, "playerid") == bet->myplayerid) {
		argc = 5;
		bet_alloc_args(argc, &argv);

		sprintf(params, "%s_%d", bits256_str(hexstr, player_info.deckid), jint(argjson, "winning_amount"));
		argv = bet_copy_args(argc, "lightning-cli", "invoice", jint(argjson, "winning_amount"), params,
				     "Winning claim");

		winner_invoice_info = cJSON_CreateObject();
		retval = make_command(argc, argv, &winner_invoice_info);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "claim");
		cJSON_AddNumberToObject(invoice_info, "playerid", bet->myplayerid);
		cJSON_AddStringToObject(invoice_info, "label", params);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(winner_invoice_info));

		retval = (nn_send(bet->pushsock, cJSON_Print(invoice_info), strlen(cJSON_Print(invoice_info)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

void display_cards()
{
	char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };

	cJSON *init_card_info = NULL, *hole_card_info = NULL, *init_info = NULL, *board_card_info = NULL;

	init_info = cJSON_CreateObject();
	cJSON_AddStringToObject(init_info, "method", "deal");

	init_card_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(init_card_info, "dealer", 0);

	hole_card_info = cJSON_CreateArray();
	for (int32_t i = 0; ((i < no_of_hole_cards) && (i < number_cards_drawn)); i++) {
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateString(cards[player_card_values[i]]));
	}

	cJSON_AddItemToObject(init_card_info, "holecards", hole_card_info);

	board_card_info = cJSON_CreateArray();
	for (int32_t i = no_of_hole_cards; ((i < hand_size) && (i < number_cards_drawn)); i++) {
		cJSON_AddItemToArray(board_card_info, cJSON_CreateString(cards[player_card_values[i]]));
	}

	cJSON_AddItemToObject(init_card_info, "board", board_card_info);
	cJSON_AddItemToObject(init_info, "deal", init_card_info);
	player_lws_write(init_info);
}

int32_t bet_client_receive_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK, cardid, playerid, errs = 0, unpermi, card_type;
	cJSON *player_card_info = NULL;
	bits256 share, decoded256;

	share = jbits256(argjson, "share");
	cardid = jint(argjson, "cardid");
	playerid = jint(argjson, "playerid");
	card_type = jint(argjson, "card_type");

	if (sharesflag[cardid][playerid] == 0) {
		playershares[cardid][playerid] = share;
		sharesflag[cardid][playerid] = 1;
		no_of_shares++;
	}
	if ((no_of_shares < bet->maxplayers) && (jint(argjson, "to_playerid") == bet->myplayerid)) {
		for (int i = 0; i < bet->maxplayers; i++) {
			if ((!sharesflag[jint(argjson, "cardid")][i]) && (i != bet->myplayerid)) {
				retval = bet_player_ask_share(bet, jint(argjson, "cardid"), bet->myplayerid,
							      jint(argjson, "card_type"), i);
				break;
			}
		}
	} else if (no_of_shares == bet->maxplayers) {
		no_of_shares = 0;
		decoded256 = bet_decode_card(argjson, bet, vars, cardid, &retval);
		if (retval != OK)
			return retval;

		if (bits256_nonz(decoded256) == 0)
			errs++;
		else {
			unpermi = -1;
			for (int k = 0; k < bet->range; k++) {
				if (player_info.permis[k] == decoded256.bytes[30]) {
					unpermi = k;
					break;
				}
			}
		}
		if (unpermi != -1) {
			player_card_values[number_cards_drawn++] = decoded256.bytes[30];
			player_card_info = cJSON_CreateObject();
			cJSON_AddStringToObject(player_card_info, "method", "playerCardInfo");
			cJSON_AddNumberToObject(player_card_info, "playerid", bet->myplayerid);
			cJSON_AddNumberToObject(player_card_info, "cardid", cardid);
			cJSON_AddNumberToObject(player_card_info, "card_type", card_type);
			cJSON_AddNumberToObject(player_card_info, "decoded_card", decoded256.bytes[30]);
			dlg_info("%s", cJSON_Print(player_card_info));
			retval = (nn_send(bet->pushsock, cJSON_Print(player_card_info),
					  strlen(cJSON_Print(player_card_info)), 0) < 0) ?
					 ERR_NNG_SEND :
					 OK;
		}
	}
	return retval;
}

int32_t bet_player_ask_share(struct privatebet_info *bet, int32_t cardid, int32_t playerid, int32_t card_type,
			     int32_t other_player)
{
	cJSON *request_info = NULL;
	int32_t retval = OK;

	request_info = cJSON_CreateObject();
	cJSON_AddStringToObject(request_info, "method", "requestShare");
	cJSON_AddNumberToObject(request_info, "playerid", playerid);
	cJSON_AddNumberToObject(request_info, "cardid", cardid);
	cJSON_AddNumberToObject(request_info, "card_type", card_type);
	cJSON_AddNumberToObject(request_info, "other_player", other_player);

	retval = (nn_send(bet->pushsock, cJSON_Print(request_info), strlen(cJSON_Print(request_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

int32_t bet_client_give_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK, playerid, cardid, recvlen, card_type;
	cJSON *share_info = NULL;
	struct enc_share temp;
	uint8_t decipher[sizeof(bits256) + 1024], *ptr;
	bits256 share;

	playerid = jint(argjson, "playerid");
	cardid = jint(argjson, "cardid");
	card_type = jint(argjson, "card_type");

	if (playerid == bet->myplayerid) {
		return retval;
	}

	temp = g_shares[playerid * bet->numplayers * bet->range + (cardid * bet->numplayers + bet->myplayerid)];

	recvlen = sizeof(temp);

	if ((ptr = bet_decrypt(decipher, sizeof(decipher), player_info.bvvpubkey, player_info.player_key.priv,
			       temp.bytes, &recvlen)) == 0) {
		retval = ERR_DECRYPTING_OTHER_SHARE;
	}

	share_info = cJSON_CreateObject();
	cJSON_AddStringToObject(share_info, "method", "share_info");
	cJSON_AddNumberToObject(share_info, "playerid", bet->myplayerid);
	cJSON_AddNumberToObject(share_info, "cardid", cardid);
	cJSON_AddNumberToObject(share_info, "card_type", card_type);
	cJSON_AddNumberToObject(share_info, "to_playerid", playerid);
	cJSON_AddNumberToObject(share_info, "error", retval);

	if (retval != ERR_DECRYPTING_OTHER_SHARE) {
		memcpy(share.bytes, ptr, recvlen);
		jaddbits256(share_info, "share", share);
	}
	retval = (nn_send(bet->pushsock, cJSON_Print(share_info), strlen(cJSON_Print(share_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;

	return retval;
}

int32_t bet_get_own_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	struct enc_share temp;
	int32_t cardid, retval = OK, playerid, recvlen;
	uint8_t decipher[sizeof(bits256) + 1024], *ptr;
	bits256 share;

	playerid = jint(argjson, "playerid");
	cardid = jint(argjson, "cardid");

	temp = g_shares[bet->myplayerid * bet->numplayers * bet->range + (cardid * bet->numplayers + playerid)];
	recvlen = sizeof(temp);

	if ((ptr = bet_decrypt(decipher, sizeof(decipher), player_info.bvvpubkey, player_info.player_key.priv,
			       temp.bytes, &recvlen)) == 0) {
		retval = ERR_DECRYPTING_OWN_SHARE;
	} else {
		memcpy(share.bytes, ptr, recvlen);
		playershares[cardid][bet->myplayerid] = share;
		sharesflag[cardid][bet->myplayerid] = 1;
	}
	return retval;
}

int32_t bet_client_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK, playerid;

	playerid = jint(argjson, "playerid");
	dlg_info("playerid::%d::%s::\n", playerid, cJSON_Print(argjson));

	if (playerid == bet->myplayerid) {
		no_of_shares = 1;
		if ((retval = bet_get_own_share(argjson, bet, vars)) != OK)
			return retval;

		for (int i = 0; i < bet->numplayers; i++) {
			if ((!sharesflag[jint(argjson, "cardid")][i]) && (i != bet->myplayerid)) {
				retval = bet_player_ask_share(bet, jint(argjson, "cardid"), jint(argjson, "playerid"),
							      jint(argjson, "card_type"), i);
				break;
			}
		}
	}
	return retval;
}

int32_t bet_player_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *player_ready = NULL;
	int retval = OK;

	player_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(player_ready, "method", "player_ready");
	cJSON_AddNumberToObject(player_ready, "playerid", bet->myplayerid);

	retval = (nn_send(bet->pushsock, cJSON_Print(player_ready), strlen(cJSON_Print(player_ready)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

int32_t bet_client_bvv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)

{
	int32_t retval = OK;
	cJSON *cjson_bvv_blind_cards, *cjson_shamir_shards;
	bits256 temp, player_privs[CARDS777_MAXCARDS];

	player_info.bvvpubkey = jbits256(argjson, "bvvpubkey");
	g_shares = (struct enc_share *)malloc(CARDS777_MAXPLAYERS * CARDS777_MAXPLAYERS * CARDS777_MAXCARDS *
					      sizeof(struct enc_share));
	cjson_bvv_blind_cards = cJSON_GetObjectItem(argjson, "bvvblindcards");

	for (int i = 0; i < bet->numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			player_info.bvvblindcards[i][j] = jbits256i(cjson_bvv_blind_cards, i * bet->range + j);
		}
	}

	cjson_shamir_shards = cJSON_GetObjectItem(argjson, "shamirshards");
	int k = 0;
	for (int playerid = 0; playerid < bet->numplayers; playerid++) {
		for (int i = 0; i < bet->range; i++) {
			for (int j = 0; j < bet->numplayers; j++) {
				g_shares[k] = get_API_enc_share(cJSON_GetArrayItem(cjson_shamir_shards, k));
				k++;
			}
		}
	}

	for (int i = 0; i < bet->range; i++) {
		for (int j = 0; j < bet->range; j++) {
			temp = xoverz_donna(
				curve25519(player_info.player_key.priv,
					   curve25519(player_privs[i], player_info.cardprods[bet->myplayerid][j])));
			vcalc_sha256(0, v_hash[i][j].bytes, temp.bytes, sizeof(temp));
		}
	}
	retval = bet_player_ready(argjson, bet, vars);
	return retval;
}

int32_t bet_client_dcv_init(cJSON *dcv_info, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *cjson_card_prods, *cjsong_hash;

	player_info.deckid = jbits256(dcv_info, "deckid");
	cjson_card_prods = cJSON_GetObjectItem(dcv_info, "cardprods");

	for (int i = 0; i < bet->numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			player_info.cardprods[i][j] = jbits256i(cjson_card_prods, i * bet->range + j);
		}
	}

	cjsong_hash = cJSON_GetObjectItem(dcv_info, "g_hash");
	for (int i = 0; i < bet->numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			g_hash[i][j] = jbits256i(cjsong_hash, i * bet->range + j);
		}
	}

	return retval;
}

int32_t bet_client_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *cjson_player_cards, *init_p = NULL;
	char str[65];

	init_p = cJSON_CreateObject();

	cJSON_AddStringToObject(init_p, "method", "init_p");
	cJSON_AddNumberToObject(init_p, "peerid", bet->myplayerid);
	jaddbits256(init_p, "pubkey", player_info.player_key.prod);
	cJSON_AddItemToObject(init_p, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int i = 0; i < bet->range; i++) {
		cJSON_AddItemToArray(cjson_player_cards,
				     cJSON_CreateString(bits256_str(str, player_info.cardpubkeys[i])));
	}
	retval = (nn_send(bet->pushsock, cJSON_Print(init_p), strlen(cJSON_Print(init_p)), 0) < 0) ? ERR_NNG_SEND : OK;

	return retval;
}

static int32_t bet_establish_ln_channel_with_dealer(cJSON *argjson)
{
	int32_t channel_state, retval = OK;
	char channel_id[ln_uri_length], uri[ln_uri_length];

	if ((retval = ln_check_if_address_isof_type(jstr(argjson, "type"))) != OK)
		return retval;

	strcpy(uri, jstr(argjson, "uri"));
	strcpy(channel_id, strtok(jstr(argjson, "uri"), "@"));
	channel_state = ln_get_channel_status(channel_id);
	if ((channel_state != CHANNELD_AWAITING_LOCKIN) && (channel_state != CHANNELD_NORMAL)) {
		if ((retval = ln_establish_channel(uri)) != OK)
			return retval;
	} else if (0 == channel_state) {
		dlg_info("There isn't any pre-established channel with the dealer, so creating one now");
		strcpy(uri, jstr(argjson, "uri"));
		retval = ln_check_peer_and_connect(uri);
	}
	return retval;
}

int32_t bet_client_join_res(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *init_card_info = NULL, *hole_card_info = NULL, *init_info = NULL;

	if (0 == bits256_cmp(player_info.player_key.prod, jbits256(argjson, "pubkey"))) {
		bet_player->myplayerid = jint(argjson, "playerid");
		bet->myplayerid = jint(argjson, "playerid");
		dlg_info("%s", cJSON_Print(argjson));

		if (bet_ln_config == BET_WITH_LN) {
			retval = bet_establish_ln_channel_with_dealer(argjson);
			if (retval != OK) {
				dlg_error("%s", bet_err_str(retval));
				return retval;
			}
		}

		init_card_info = cJSON_CreateObject();
		cJSON_AddNumberToObject(init_card_info, "dealer", jint(argjson, "dealer"));

		cJSON_AddNumberToObject(init_card_info, "balance", vars->player_funds);

		hole_card_info = cJSON_CreateArray();
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToObject(init_card_info, "holecards", hole_card_info);

		init_info = cJSON_CreateObject();
		cJSON_AddStringToObject(init_info, "method", "deal");
		cJSON_AddItemToObject(init_info, "deal", init_card_info);

		dlg_info("init_info::%s", cJSON_Print(init_info));
		player_lws_write(init_info);
	}
	return retval;
}

int32_t bet_client_join(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK;
	char *uri = NULL;
	cJSON *joininfo = NULL;
	struct pair256 key;

	if ((jint(argjson, "gui_playerID") < 1) || (jint(argjson, "gui_playerID") > bet->maxplayers)) {
		retval = ERR_INVALID_POS;
		return retval;
	}
	key = deckgen_player(player_info.cardprivkeys, player_info.cardpubkeys, player_info.permis, bet->range);
	player_info.player_key = key;
	joininfo = cJSON_CreateObject();
	cJSON_AddStringToObject(joininfo, "method", "join_req");
	jaddbits256(joininfo, "pubkey", key.prod);

	if (bet_ln_config == BET_WITH_LN) {
		uri = (char *)malloc(ln_uri_length * sizeof(char));
		ln_get_uri(&uri);
		cJSON_AddStringToObject(joininfo, "uri", uri);
	}

	cJSON_AddNumberToObject(joininfo, "gui_playerID", (jint(argjson, "gui_playerID") - 1));
	cJSON_AddStringToObject(joininfo, "req_identifier", req_identifier);
	cJSON_AddStringToObject(joininfo, "player_name", player_name);

	dlg_info("join info::%s\n", cJSON_Print(joininfo));
	retval = (nn_send(bet->pushsock, cJSON_Print(joininfo), strlen(cJSON_Print(joininfo)), 0) < 0) ? ERR_NNG_SEND :
													 OK;

end:
	if (uri)
		free(uri);
	return retval;
}

static int32_t bet_player_process_player_join(cJSON *argjson)
{
	int32_t retval = OK;
	cJSON *warning_info = NULL;

	if (player_joined == 0) {
		if (backend_status == backend_ready) {
			retval = bet_client_join(argjson, bet_player);
		} else {
			warning_info = cJSON_CreateObject();
			cJSON_AddStringToObject(warning_info, "method", "warning");
			cJSON_AddNumberToObject(warning_info, "warning_num", backend_not_ready);
			dlg_warn("%s\n", cJSON_Print(warning_info));
			player_lws_write(warning_info);
		}
	}
	return retval;
}

static void bet_player_handle_invalid_method(char *method)
{
	cJSON *error_info = NULL;
	char error_msg[2048];

	error_info = cJSON_CreateObject();
	cJSON_AddStringToObject(error_info, "method", "error");
	snprintf(error_msg, sizeof(error_msg), "Method::%s is not handled", method);
	cJSON_AddStringToObject(error_info, "msg", error_msg);
	player_lws_write(error_info);
}

static void bet_player_withdraw_request()
{
	cJSON *withdraw_response_info = NULL;

	withdraw_response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(withdraw_response_info, "method", "withdrawResponse");
	cJSON_AddNumberToObject(withdraw_response_info, "balance", chips_get_balance());
	cJSON_AddNumberToObject(withdraw_response_info, "tx_fee", chips_tx_fee);
	cJSON_AddItemToObject(withdraw_response_info, "addrs", chips_list_address_groupings());
	player_lws_write(withdraw_response_info);
}

static void bet_player_withdraw(cJSON *argjson)
{
	cJSON *withdraw_info = NULL;
	double amount = 0.0;
	char *addr = NULL;

	amount = jdouble(argjson, "amount");
	addr = jstr(argjson, "addr");
	withdraw_info = cJSON_CreateObject();
	cJSON_AddStringToObject(withdraw_info, "method", "withdrawInfo");
	cJSON_AddItemToObject(withdraw_info, "tx", chips_transfer_funds(amount, addr));
	player_lws_write(withdraw_info);
}

static void bet_player_wallet_info()
{
	cJSON *wallet_info = NULL;

	wallet_info = cJSON_CreateObject();
	cJSON_AddStringToObject(wallet_info, "method", "walletInfo");
	cJSON_AddStringToObject(wallet_info, "addr", chips_get_wallet_address());
	cJSON_AddNumberToObject(wallet_info, "balance", chips_get_balance());
	cJSON_AddNumberToObject(wallet_info, "backend_status", backend_status);
	cJSON_AddNumberToObject(wallet_info, "max_players", max_players);
	cJSON_AddNumberToObject(wallet_info, "table_stack_in_chips", (table_stake_in_chips / SB_in_chips));
	cJSON_AddStringToObject(wallet_info, "table_id", table_id);
	cJSON_AddNumberToObject(wallet_info, "tx_fee", chips_tx_fee);
	dlg_info("%s\n", cJSON_Print(wallet_info));
	player_lws_write(wallet_info);
}

static void bet_player_process_be_status()
{
	cJSON *be_info = NULL;

	be_info = cJSON_CreateObject();
	cJSON_AddStringToObject(be_info, "method", "backend_status");
	cJSON_AddNumberToObject(be_info, "backend_status", backend_status);
	player_lws_write(be_info);
}

// clang-format off
int32_t bet_player_frontend(struct lws *wsi, cJSON *argjson)
{
	int32_t retval = OK;
	char *method = NULL;

	method = jstr(argjson,"method");
	switchs(method) {
		dlg_info("Recv from GUI :: %s", cJSON_Print(argjson));
		cases("backend_status")
			bet_player_process_be_status();
			break;
		cases("betting")
			retval = bet_player_round_betting(argjson, bet_player, player_vars);
			break;
		cases("get_bal_info")
			player_lws_write(bet_get_chips_ln_bal_info());
			break;
		cases("player_join")
			retval = bet_player_process_player_join(argjson);
			break;
		cases("reset")
			retval = bet_player_reset(bet_player, player_vars);
			break;
		cases("sitout")
			sitout_value = jint(argjson, "value");
			break;
		cases("walletInfo") 
			bet_player_wallet_info();
			break;
		cases("withdraw")
			bet_player_withdraw(argjson);
			break;
		cases("withdrawRequest")
			bet_player_withdraw_request();
			break;
		defaults
			bet_player_handle_invalid_method(method);
	}switchs_end;
	
	if (retval != OK)
		bet_handle_player_error(bet_player, retval);
	return retval;
}
// clang-format on

static void bet_gui_init_message(struct privatebet_info *bet)
{
	cJSON *warning_info = NULL;
	cJSON *req_seats_info = NULL;

	if (backend_status == backend_not_ready) {
		warning_info = cJSON_CreateObject();
		cJSON_AddStringToObject(warning_info, "method", "warning");
		cJSON_AddNumberToObject(warning_info, "warning_num", backend_not_ready);
		dlg_warn("Backend is not yet ready, it takes a while please wait...");
		player_lws_write(warning_info);
	} else {
		req_seats_info = cJSON_CreateObject();
		cJSON_AddStringToObject(req_seats_info, "method", "req_seats_info");
		cJSON_AddStringToObject(req_seats_info, "req_identifier", req_identifier);
		dlg_info("Requesting seats into with the dealer \n %s", cJSON_Print(req_seats_info));
		if (nn_send(bet->pushsock, cJSON_Print(req_seats_info), strlen(cJSON_Print(req_seats_info)), 0) < 0) {
			dlg_error("Error in sending the data");
		}
	}
}

int32_t lws_callback_http_player_write(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in,
				       size_t len)
{
	int32_t retval = OK;
	cJSON *argjson = NULL;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf_1);

		if ((retval = bet_player_frontend(wsi, argjson)) != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client_write = wsi;
		ws_connection_status_write = 1;
		bet_gui_init_message(bet_player);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (data_exists) {
			if (strlen(player_gui_data) != 0) {
				lws_write(wsi, (unsigned char *)player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	default:
		break;
	}
	return retval;
}

int32_t lws_callback_http_player_read(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in,
				      size_t len)
{
	int32_t retval = OK;
	cJSON *argjson = NULL;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;

		argjson = cJSON_Parse(unstringify(lws_buf_1));
		if ((retval = bet_player_frontend(wsi, argjson)) != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client = wsi;
		ws_connection_status = 1;
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (data_exists) {
			if (strlen(player_gui_data) != 0) {
				lws_write(wsi, (unsigned char *)player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	default:
		break;
	}
	return retval;
}

int32_t lws_callback_http_player(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	int32_t retval = OK;
	cJSON *argjson = NULL;

	//dlg_info("callback code::%d\n", (int)reason);
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf_1);

		if ((retval = bet_player_frontend(wsi, argjson)) != OK) {
			dlg_error("%s", bet_err_str(retval));
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client = wsi;
		dlg_info("LWS_CALLBACK_ESTABLISHED\n");
		ws_connection_status = 1;
		bet_gui_init_message(bet_player);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		dlg_info("LWS_CALLBACK_SERVER_WRITEABLE triggered\n");
		if (data_exists) {
			if (strlen(player_gui_data) != 0) {
				lws_write(wsi, (unsigned char *)player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	default:
		break;
	}
	return retval;
}

static struct lws_protocols player_http_protocol[] = {
	{ "http", lws_callback_http_player, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static struct lws_protocols player_http_protocol_read[] = {
	{ "http", lws_callback_http_player_read, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static struct lws_protocols player_http_protocol_write[] = {
	{ "http", lws_callback_http_player_write, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted1, interrupted_read, interrupted_write;

static const struct lws_http_mount lws_http_mount_player = {
	/* .mount_next */ NULL, /* linked-list "next" */
	/* .mountpoint */ "/", /* mountpoint URL */
	/* .origin */ "./mount-origin", /* serve from dir */
	/* .def */ "index.html", /* default filename */
	/* .protocol */ NULL,
	/* .cgienv */ NULL,
	/* .extra_mimetypes */ NULL,
	/* .interpret */ NULL,
	/* .cgi_timeout */ 0,
	/* .cache_max_age */ 0,
	/* .auth_mask */ 0,
	/* .cache_reusable */ 0,
	/* .cache_revalidate */ 0,
	/* .cache_intermediaries */ 0,
	/* .origin_protocol */ LWSMPRO_FILE, /* files in a dir */
	/* .mountpoint_len */ 1, /* char count */
	/* .basic_auth_login_file */ NULL,
};

void player_sigint_handler(int sig)
{
	interrupted1 = 1;
}

void player_sigint_handler_read(int sig)
{
	interrupted_read = 1;
}

void player_sigint_handler_write(int sig)
{
	interrupted_write = 1;
}

void bet_player_frontend_read_loop(void *_ptr)
{
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	lws_set_log_level(logs, NULL);

	memset(&lws_player_info_read, 0, sizeof lws_player_info_read); /* otherwise uninitialized garbage */
	lws_player_info_read.port = 9001;
	lws_player_info_read.mounts = &lws_http_mount_player;
	lws_player_info_read.protocols = player_http_protocol_read;
	lws_player_info_read.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	player_context_read = lws_create_context(&lws_player_info_read);
	if (!player_context_read) {
		dlg_error("lws init failed\n");
	}
	while (n >= 0 && !interrupted1) {
		n = lws_service(player_context_read, 1000);
	}
}

void bet_player_frontend_write_loop(void *_ptr)
{
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	// signal(SIGINT,player_sigint_handler);
	lws_set_log_level(logs, NULL);

	memset(&lws_player_info_write, 0, sizeof lws_player_info_write); /* otherwise uninitialized garbage */
	lws_player_info_write.port = gui_ws_port;
	lws_player_info_write.mounts = &lws_http_mount_player;
	lws_player_info_write.protocols = player_http_protocol_write;
	lws_player_info_write.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	player_context_write = lws_create_context(&lws_player_info_write);
	if (!player_context_write) {
		dlg_error("lws init failed\n");
	}
	while (n >= 0 && !interrupted1) {
		n = lws_service(player_context_write, 1000);
	}
}

void bet_player_frontend_loop(void *_ptr)
{
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	lws_set_log_level(logs, NULL);

	memset(&lws_player_info, 0, sizeof lws_player_info); /* otherwise uninitialized garbage */
	lws_player_info.port = gui_ws_port;
	lws_player_info.mounts = &lws_http_mount_player;
	lws_player_info.protocols = player_http_protocol;
	lws_player_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	if (lws_player_info.vhost_name == NULL) {
		dlg_info("vhost is NULL");
		lws_player_info.vhost_name = (char *)malloc(100 * sizeof(char));
	} else {
		dlg_info("vhost::%s\n", lws_player_info.vhost_name);
	}

	player_context = lws_create_context(&lws_player_info);
	if (!player_context) {
		dlg_error("lws init failed\n");
	}
	while (n >= 0 && !interrupted1) {
		n = lws_service(player_context, 1000);
	}
}

void bet_push_client(cJSON *argjson)
{
	player_lws_write(argjson);
}

static void bet_player_blinds_info()
{
	cJSON *blinds_info = NULL;

	blinds_info = cJSON_CreateObject();
	cJSON_AddStringToObject(blinds_info, "method", "blindsInfo");
	cJSON_AddNumberToObject(blinds_info, "small_blind", small_blind_amount);
	cJSON_AddNumberToObject(blinds_info, "big_blind", big_blind_amount);
	player_lws_write(blinds_info);
}

static void bet_push_join_info(cJSON *argjson)
{
	cJSON *join_info = NULL;

	join_info = cJSON_CreateObject();
	cJSON_AddStringToObject(join_info, "method", "info"); //changed to join_info to info
	cJSON_AddNumberToObject(join_info, "playerid", jint(argjson, "playerid"));
	if (jint(argjson, "pos_status") == pos_on_table_full)
		cJSON_AddNumberToObject(join_info, "seat_taken", 1);
	else
		cJSON_AddNumberToObject(join_info, "seat_taken", 0);
	dlg_info("Writing the availability of the seat info to the GUI \n %s", cJSON_Print(join_info));
	player_lws_write(join_info);
}

static int32_t bet_update_payin_tx_across_cashiers(cJSON *argjson, cJSON *txid)
{
	int32_t retval = OK;
	char *sql_query = NULL;
	cJSON *msig_addr_nodes = NULL, *payin_tx_insert_query = NULL;

	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "INSERT INTO player_tx_mapping values(%s,\'%s\',\'%s\',\'%s\',%d,%d, NULL);",
		cJSON_Print(txid), table_id, req_identifier,
		cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes")), tx_unspent, threshold_value);
	retval = bet_run_query(sql_query); // This is to update payin_tx in the players DB

	memset(sql_query, 0x00, sql_query_size);
	msig_addr_nodes = cJSON_CreateArray();
	msig_addr_nodes = cJSON_GetObjectItem(argjson, "msig_addr_nodes");
	sprintf(sql_query, "INSERT INTO c_tx_addr_mapping values(%s,\'%s\',%d,\'%s\',\'%s\',1,NULL);",
		cJSON_Print(txid), legacy_m_of_n_msig_addr, threshold_value, table_id,
		unstringify(cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes"))));

	payin_tx_insert_query = cJSON_CreateObject();
	cJSON_AddStringToObject(payin_tx_insert_query, "method", "lock_in_tx");
	cJSON_AddStringToObject(payin_tx_insert_query, "sql_query", sql_query);
	for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
		bet_msg_cashier(payin_tx_insert_query,
				unstringify(cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i))));
	}
	if (sql_query)
		free(sql_query);

	return retval;
}

static int32_t check_funds_for_poker(double table_stake)
{
	int32_t no_of_possible_moves = 20, retval = 0;
	double game_fee = 0, funds_needed = 0, balance = 0;

	game_fee = no_of_possible_moves * chips_tx_fee;

	funds_needed = table_stake + game_fee + chips_tx_fee;

	balance = chips_get_balance();

	dlg_info("table_stake ::%f\n, reserved_game_fee ::%f\n, funds_available::%f", table_stake, game_fee, balance);
	if (balance >= funds_needed) {
		retval = 1;
	}

	return retval;
}

static struct cJSON *add_tx_split_vouts(double amount, char *address)
{
	cJSON *vout_addresses = NULL;
	int no_of_split_tx = 20;

	vout_addresses = cJSON_CreateArray();
	if (address) {
		cJSON *payin_vout = cJSON_CreateObject();
		cJSON_AddStringToObject(payin_vout, "addr", address);
		cJSON_AddNumberToObject(payin_vout, "amount", amount);
		cJSON_AddItemToArray(vout_addresses, payin_vout);
	}
	for (int32_t i = 0; i < no_of_split_tx; i++) {
		cJSON *fee_vout = cJSON_CreateObject();
		cJSON_AddStringToObject(fee_vout, "addr", chips_get_new_address());
		cJSON_AddNumberToObject(fee_vout, "amount", chips_tx_fee);
		cJSON_AddItemToArray(vout_addresses, fee_vout);
	}
	return vout_addresses;
}

static void bet_player_check_dealer_gui_url(cJSON *argjson)
{
	if (0 == check_url(jstr(argjson, "gui_url"))) {
		if (0 == strlen(jstr(argjson, "gui_url"))) {
			dlg_warn("Dealer is not hosting the GUI");
		} else {
			dlg_warn("Dealer hosted GUI :: %s is not reachable", jstr(argjson, "gui_url"));
		}
		dlg_info("Player can use any of the GUI's hosted by cashiers to connect to backend");
		bet_display_cashier_hosted_gui();

	} else {
		dlg_warn("Dealer hosted GUI :: %s, using this you can connect to player backend and interact",
			 jstr(argjson, "gui_url"));
	}
}

static int32_t bet_do_player_checks(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK;
	if (jdouble(argjson, "dcv_commission") > max_allowed_dcv_commission) {
		dlg_warn(
			"Dealer set the commission %f%% which is more than max commission i.e %f%% set by player, so exiting",
			jdouble(argjson, "dcv_commission"), max_allowed_dcv_commission);
		retval = ERR_DCV_COMMISSION_MISMATCH;
		return retval;
	}

	if (0 == check_funds_for_poker(jdouble(argjson, "table_min_stake"))) {
		retval = ERR_CHIPS_INSUFFICIENT_FUNDS;
		return retval;
	}
	return retval;
}

static int32_t bet_player_initialize_table_params(cJSON *argjson, struct privatebet_info *bet)
{
	BB_in_chips = jdouble(argjson, "bb_in_chips");
	SB_in_chips = BB_in_chips / 2;
	table_stake_in_chips = table_stack_in_bb * BB_in_chips;
	if (table_stake_in_chips < jdouble(argjson, "table_min_stake")) {
		table_stake_in_chips = jdouble(argjson, "table_min_stake");
	}
	if (table_stake_in_chips > jdouble(argjson, "table_max_stake")) {
		table_stake_in_chips = jdouble(argjson, "table_max_stake");
	}
	max_players = jint(argjson, "max_players");
	chips_tx_fee = jdouble(argjson, "chips_tx_fee");
	legacy_m_of_n_msig_addr = (char *)malloc(strlen(jstr(argjson, "legacy_m_of_n_msig_addr")) + 1);
	memset(legacy_m_of_n_msig_addr, 0x00, strlen(jstr(argjson, "legacy_m_of_n_msig_addr")) + 1);
	strncpy(legacy_m_of_n_msig_addr, jstr(argjson, "legacy_m_of_n_msig_addr"),
		strlen(jstr(argjson, "legacy_m_of_n_msig_addr")));
	threshold_value = jint(argjson, "threshold_value");
	memset(table_id, 0x00, sizeof(table_id));
	strncpy(table_id, jstr(argjson, "table_id"), strlen(jstr(argjson, "table_id")));

	bet->maxplayers = max_players;
	bet->numplayers = max_players;
}

static struct cJSON *bet_player_make_payin_tx_data(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *payin_tx_data = NULL;

	payin_tx_data = cJSON_CreateObject();
	cJSON_AddStringToObject(payin_tx_data, "table_id", table_id);
	cJSON_AddStringToObject(payin_tx_data, "msig_addr_nodes",
				unstringify(cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes"))));
	cJSON_AddNumberToObject(payin_tx_data, "min_cashiers", threshold_value);
	cJSON_AddStringToObject(payin_tx_data, "player_id", req_identifier);
	cJSON_AddStringToObject(payin_tx_data, "dispute_addr", chips_get_new_address());
	cJSON_AddStringToObject(payin_tx_data, "msig_addr", legacy_m_of_n_msig_addr);
	cJSON_AddStringToObject(payin_tx_data, "tx_type", "payin");

	return payin_tx_data;
}

static int32_t bet_player_handle_stack_info_resp(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK, hex_data_len = 0;
	//double funds_available;
	char *hex_data = NULL;
	cJSON *tx_info = NULL, *txid = NULL, *payin_tx_data = NULL, *vout_addresses = NULL;

	bet_player_check_dealer_gui_url(argjson);

	retval = bet_do_player_checks(argjson, bet);
	if (retval != OK)
		return retval;

	bet_player_initialize_table_params(argjson, bet);

	/*
	funds_available = chips_get_balance() - chips_tx_fee;
	if (funds_available < jdouble(argjson, "table_min_stake")) {
		retval = ERR_CHIPS_INSUFFICIENT_FUNDS;
		return retval;
	}
	*/

	payin_tx_data = bet_player_make_payin_tx_data(argjson, bet);

	hex_data_len = 2 * strlen(cJSON_Print(payin_tx_data)) + 1;
	hex_data = calloc(hex_data_len, sizeof(char));
	str_to_hexstr(cJSON_Print(payin_tx_data), hex_data);

	/*
	dlg_info("funds_needed::%f", table_stake_in_chips);
	
	dlg_info("Will wait for a while till the tx's in mempool gets cleared");
	while (!chips_is_mempool_empty()) {
		sleep(2);
	}
	*/
	vout_addresses = add_tx_split_vouts(table_stake_in_chips, legacy_m_of_n_msig_addr);
	txid = chips_transfer_funds_with_data1(vout_addresses, hex_data);
	//txid = chips_transfer_funds_with_data(table_stake_in_chips, legacy_m_of_n_msig_addr, hex_data);
	if (txid == NULL) {
		retval = ERR_CHIPS_INVALID_TX;
		return retval;
	} else {
		retval = bet_store_game_info_details(cJSON_Print(txid), table_id);
	}
	dlg_info("tx id::%s", cJSON_Print(txid));
	memset(player_payin_txid, 0x00, sizeof(player_payin_txid));
	strcpy(player_payin_txid, cJSON_Print(txid));

	retval = bet_update_payin_tx_across_cashiers(argjson, txid);
	if (retval != OK) {
		dlg_error("Updating payin_tx across the cashier nodes or in the player DB got failed");
		retval = OK;
	}

	tx_info = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_info, "method", "tx");
	cJSON_AddStringToObject(tx_info, "id", req_identifier);
	cJSON_AddStringToObject(tx_info, "chips_addr", chips_get_new_address());
	cJSON_AddItemToObject(tx_info, "tx_info", txid);
	dlg_info("Waiting for tx to confirm");
	while (chips_get_block_hash_from_txid(cJSON_Print(txid)) == NULL) {
		sleep(2);
	}
	dlg_info("TX ::%s got confirmed", cJSON_Print(txid));
	cJSON_AddNumberToObject(
		tx_info, "block_height",
		chips_get_block_height_from_block_hash(chips_get_block_hash_from_txid(cJSON_Print(txid))));
	retval =
		(nn_send(bet->pushsock, cJSON_Print(tx_info), strlen(cJSON_Print(tx_info)), 0) < 0) ? ERR_NNG_SEND : OK;

	if (hex_data)
		free(hex_data);
	return retval;
}

int32_t bet_player_stack_info_req(struct privatebet_info *bet)
{
	int32_t bytes, retval = OK;
	cJSON *stack_info_req = NULL;
	char rand_str[65] = { 0 };
	bits256 randval;

	stack_info_req = cJSON_CreateObject();
	cJSON_AddStringToObject(stack_info_req, "method", "stack_info_req");
	OS_randombytes(randval.bytes, sizeof(randval));
	bits256_str(rand_str, randval);
	strncpy(req_identifier, rand_str, sizeof(req_identifier));
	cJSON_AddStringToObject(stack_info_req, "id", rand_str);
	cJSON_AddStringToObject(stack_info_req, "chips_addr", chips_get_new_address());
	cJSON_AddNumberToObject(stack_info_req, "is_table_private", is_table_private);
	if (is_table_private) {
		cJSON_AddStringToObject(stack_info_req, "table_password", table_password);
	}
	bytes = nn_send(bet->pushsock, cJSON_Print(stack_info_req), strlen(cJSON_Print(stack_info_req)), 0);
	if (bytes < 0)
		retval = ERR_NNG_SEND;

	return retval;
}

static int32_t bet_player_process_payout_tx(cJSON *argjson)
{
	char *sql_query = NULL;
	int32_t rc = OK;

	dlg_info("%s\n", cJSON_Print(argjson));
	sql_query = calloc(sql_query_size, sizeof(char));
	sprintf(sql_query, "UPDATE player_tx_mapping set status = 0,payout_tx_id = \'%s\' where table_id = \'%s\'",
		jstr(argjson, "tx_info"), jstr(argjson, "table_id"));
	rc = bet_run_query(sql_query);
	if (rc != SQLITE_OK)
		rc = ERR_SQL;
	if (sql_query)
		free(sql_query);
	return rc;
}

static int32_t bet_player_process_game_info(cJSON *argjson)
{
	int argc = 3, retval = OK;
	char **argv = NULL;
	char *sql_query = calloc(sql_query_size, sizeof(char));

	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "player_game_state");
	sprintf(argv[1], "\'%s\'", table_id);
	sprintf(argv[2], "\'%s\'", cJSON_Print(cJSON_GetObjectItem(argjson, "game_state")));

	bet_make_insert_query(argc, argv, &sql_query);
	retval = bet_run_query(sql_query);
	if (retval != SQLITE_OK)
		retval = ERR_SQL;

	bet_dealloc_args(argc, &argv);
	if (sql_query)
		free(sql_query);
	return retval;
}

static void bet_update_seat_info(cJSON *argjson)
{
	cJSON *seats_info = NULL;

	seats_info = cJSON_CreateObject();
	cJSON_AddStringToObject(seats_info, "method", "seats");
	cJSON_AddItemToObject(seats_info, "seats", cJSON_GetObjectItem(argjson, "seats"));
	dlg_info("%s", cJSON_Print(seats_info));
	player_lws_write(seats_info);
}

void bet_handle_player_error(struct privatebet_info *bet, int32_t err_no)
{
	cJSON *publish_error = NULL;

	dlg_error("%s", bet_err_str(err_no));
	publish_error = cJSON_CreateObject();
	cJSON_AddStringToObject(publish_error, "method", "player_error");
	cJSON_AddNumberToObject(publish_error, "playerid", bet->myplayerid);
	cJSON_AddNumberToObject(publish_error, "err_no", err_no);
	if (nn_send(bet->pushsock, cJSON_Print(publish_error), strlen(cJSON_Print(publish_error)), 0) < 0)
		exit(-1);

	switch (err_no) {
	case ERR_DECRYPTING_OWN_SHARE:
	case ERR_DECRYPTING_OTHER_SHARE:
	case ERR_CARD_RETRIEVING_USING_SS:
		dlg_info("This error can impact whole game...");
		break;
	case ERR_DEALER_TABLE_FULL:
		bet_raise_dispute(player_payin_txid);
	case ERR_LN_ADDRESS_TYPE_MISMATCH:
	case ERR_INVALID_POS:
	case ERR_CHIPS_INSUFFICIENT_FUNDS:
	case ERR_CHIPS_INVALID_TX:
	case ERR_PT_PLAYER_UNAUTHORIZED:
	case ERR_DCV_COMMISSION_MISMATCH:
	case ERR_INI_PARSING:
	case ERR_JSON_PARSING:
	case ERR_NNG_SEND:
	case ERR_NNG_BINDING:
	case ERR_PTHREAD_LAUNCHING:
	case ERR_PTHREAD_JOINING:
		exit(-1);
	default:
		dlg_error("The err_no :: %d is not handled by the backend player yet", err_no);
	}
}

int32_t bet_player_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	char *method = NULL;
	char hexstr[65];

	if (strcmp(jstr(argjson, "method"), "reset") == 0) {
		reset_lock = 0;
		retval = bet_player_reset(bet, vars);
	}
	if (reset_lock == 1) {
		return retval;
	}
	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("recv :: %s", method);
		if (strcmp(method, "join_res") == 0) {
			bet_update_seat_info(argjson);
			if (strcmp(jstr(argjson, "req_identifier"), req_identifier) == 0) {
				bet_push_join_info(argjson);
				if (jint(argjson, "pos_status") == pos_on_table_empty) {
					retval = bet_client_join_res(argjson, bet, vars);
				} else {
					dlg_warn(
						"Player selected pos on the talbe is already taken, player need to select another pos to sit in");
				}
			}
		} else if (strcmp(method, "init") == 0) {
			if (jint(argjson, "peerid") == bet->myplayerid) {
				bet_player_blinds_info();
				dlg_info("myplayerid::%d::init::%s\n", bet->myplayerid, cJSON_Print(argjson));
				retval = bet_client_init(argjson, bet, vars);
			}
		} else if (strcmp(method, "init_d") == 0) {
			retval = bet_client_dcv_init(argjson, bet, vars);
		} else if (strcmp(method, "init_b") == 0) {
			retval = bet_client_bvv_init(argjson, bet, vars);
		} else if (strcmp(method, "turn") == 0) {
			if (jint(argjson, "playerid") == bet->myplayerid) {
				retval = bet_client_turn(argjson, bet, vars);
			}
		} else if (strcmp(method, "ask_share") == 0) {
			retval = bet_client_give_share(argjson, bet, vars);
		} else if (strcmp(method, "requestShare") == 0) {
			if (jint(argjson, "other_player") == bet->myplayerid) {
				retval = bet_client_give_share(argjson, bet, vars);
			}
		} else if (strcmp(method, "share_info") == 0) {
			if (jint(argjson, "to_playerid") == bet->myplayerid) {
				retval = bet_client_receive_share(argjson, bet, vars);
			}
		} else if (strcmp(method, "invoice") == 0) {
			retval = ln_pay_invoice(argjson, bet, vars);
		} else if (strcmp(method, "bettingInvoice") == 0) {
			retval = bet_player_betting_invoice(argjson, bet, vars);
		} else if (strcmp(method, "winner") == 0) {
			retval = bet_player_winner(argjson, bet, vars);
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_betting_statemachine(argjson, bet, vars);
		} else if (strcmp(method, "display_current_state") == 0) {
			retval = bet_display_current_state(argjson, bet, vars);
		} else if (strcmp(method, "dealer") == 0) {
			retval = bet_player_dealer_info(argjson, bet, vars);
		} else if (strcmp(method, "invoiceRequest_player") == 0) {
			retval = bet_player_create_invoice(argjson, bet, vars, bits256_str(hexstr, player_info.deckid));
		} else if (strcmp(method, "reset") == 0) {
			retval = bet_player_reset(bet, vars);
		} else if (strcmp(method, "seats") == 0) {
			cJSON_AddNumberToObject(argjson, "playerFunds", ln_listfunds());
			player_lws_write(argjson);
		} else if (strcmp(method, "finalInfo") == 0) {
			player_lws_write(argjson);
		} else if (strcmp(method, "player_stakes_info") == 0) {
			cJSON *stakes = cJSON_GetObjectItem(argjson, "stakes");
			dlg_info("Player_stakes");
			for (int32_t i = 0; i < bet->maxplayers; i++) {
				vars->funds[i] = jinti(stakes, i);
				dlg_info("player::%d, stake::%d", i, vars->funds[i]);
			}
		} else if (strcmp(method, "signrawtransaction") == 0) {
			if (jint(argjson, "playerid") == bet->myplayerid) {
				cJSON *temp = cJSON_CreateObject();
				temp = chips_sign_raw_tx_with_wallet(jstr(argjson, "tx"));
				cJSON *signedTxInfo = cJSON_CreateObject();

				cJSON_AddStringToObject(signedTxInfo, "method", "signedrawtransaction");
				cJSON_AddStringToObject(signedTxInfo, "tx", jstr(temp, "hex"));
				cJSON_AddNumberToObject(signedTxInfo, "playerid", bet->myplayerid);
				retval = (nn_send(bet->pushsock, cJSON_Print(signedTxInfo),
						  strlen(cJSON_Print(signedTxInfo)), 0) < 0) ?
						 ERR_NNG_SEND :
						 OK;
			}
		} else if (strcmp(method, "stack_info_resp") == 0) {
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				retval = bet_player_handle_stack_info_resp(argjson, bet);
			}
		} else if (strcmp(method, "tx_status") == 0) {
			if (strcmp(req_identifier, jstr(argjson, "id")) == 0) {
				bet_player_wallet_info();
				vars->player_funds = jint(argjson, "player_funds");
				if (jint(argjson, "tx_validity") == OK) {
					dlg_info("Dealer verified the TX made by the player");
					if (backend_status == backend_ready) {
						/* 
						This snippet is added to handle the reset scenario after the initial hand got played. How this works is the backend status 
						for the player is set when the dealer verifies the tx as valid.
						It means when the backend is ready, for the next hand the GUI is expecting to push the reset info from the backend for this reason
						this snippet is added.
						*/
						cJSON *reset_info = cJSON_CreateObject();
						cJSON_AddStringToObject(reset_info, "method", "reset");
						player_lws_write(reset_info);
					}
					backend_status = backend_ready;
					cJSON *info = cJSON_CreateObject();
					cJSON_AddStringToObject(info, "method", "backend_status");
					cJSON_AddNumberToObject(info, "backend_status", backend_status);
					player_lws_write(info);
					cJSON *req_seats_info = NULL;
					req_seats_info = cJSON_CreateObject();
					cJSON_AddStringToObject(req_seats_info, "method", "req_seats_info");
					cJSON_AddStringToObject(req_seats_info, "req_identifier", req_identifier);
					dlg_info("Player requesting seats info from the dealer to join");
					retval = (nn_send(bet->pushsock, cJSON_Print(req_seats_info),
							  strlen(cJSON_Print(req_seats_info)), 0) < 0) ?
							 ERR_NNG_SEND :
							 OK;

				} else {
					retval = ERR_CHIPS_INVALID_TX;
				}
			}
		} else if (strcmp(method, "payout_tx") == 0) {
			if (jstr(argjson, "tx_info")) {
				retval = bet_store_game_info_details(jstr(argjson, "tx_info"),
								     jstr(argjson, "table_id"));
				retval = bet_player_process_payout_tx(argjson);
			} else {
				dlg_warn("Error occured in payout_tx, so raising the dispute for payin_tx");
				bet_raise_dispute(player_payin_txid);
			}
		} else if (strcmp(method, "game_info") == 0) {
			retval = bet_player_process_game_info(argjson);
		} else if (strcmp(method, "dcv_state") == 0) {
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				if (jint(argjson, "dcv_state") == 1) {
					dlg_warn("DCV which you trying to connect is full");
					dlg_info("%s\n", cJSON_Print(argjson));
					bet_player_reset(bet, vars);
					retval = ERR_DEALER_TABLE_FULL;
				}
			}
		} else if (strcmp(method, "tx_reverse") == 0) {
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				dlg_warn(
					"The dealers table is already full, the payin_tx will be reversed using dispute resolution protocol::%s\n",
					cJSON_Print(argjson));
				bet_raise_dispute(player_payin_txid);
				retval = ERR_DEALER_TABLE_FULL;
			}
		} else if (strcmp(method, "seats_info_resp") == 0) {
			if (strcmp(req_identifier, jstr(argjson, "req_identifier")) == 0) {
				cJSON *seats_info = cJSON_CreateObject();
				cJSON_AddStringToObject(seats_info, "method", "seats");
				cJSON_AddItemToObject(seats_info, "seats", cJSON_GetObjectItem(argjson, "seats"));
				player_lws_write(seats_info);
				if ((backend_status == backend_ready) && (ws_connection_status_write == 0)) {
					dlg_info("Backend is ready, from GUI you can connect to backend and play...");
				}
			}
		} else if (strcmp(method, "is_player_active") == 0) {
			cJSON *active_info = NULL;
			active_info = cJSON_CreateObject();
			cJSON_AddStringToObject(active_info, "method", "player_active");
			cJSON_AddNumberToObject(active_info, "playerid", bet->myplayerid);
			cJSON_AddStringToObject(active_info, "req_identifier", req_identifier);
			retval = (nn_send(bet->pushsock, cJSON_Print(active_info), strlen(cJSON_Print(active_info)),
					  0) < 0) ?
					 ERR_NNG_SEND :
					 OK;
		} else if (strcmp(method, "active_player_info") == 0) {
			player_lws_write(argjson);
		} else if (strcmp(method, "game_abort") == 0) {
			dlg_warn("Player :: %d encounters the error ::%s, it has impact on game so exiting...",
				 jint(argjson, "playerid"), bet_err_str(jint(argjson, "err_no")));
			bet_raise_dispute(player_payin_txid);
			exit(-1);
		} else if (strcmp(method, "game_abort_player") == 0) {
			if (strcmp(req_identifier, jstr(argjson, "id")) == 0) {
				bet_handle_player_error(bet, jint(argjson, "err_no"));
				exit(-1);
			}
		} else {
			dlg_info("%s method is not handled in the backend\n", method);
		}
	}
	return retval;
}

void bet_player_backend_loop(void *_ptr)
{
	int32_t recvlen = 0, retval = OK;
	void *ptr = NULL;
	cJSON *msgjson = NULL;
	struct privatebet_info *bet = _ptr;

	retval = bet_player_stack_info_req(bet);
	if (retval != OK) {
		bet_handle_player_error(bet, retval);
	}
	while (retval == OK) {
		if (bet->subsock >= 0 && bet->pushsock >= 0) {
			ptr = 0;
			char *tmp = NULL;
			recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0);
			if (recvlen > 0)
				tmp = clonestr(ptr);
			if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
				if ((retval = bet_player_backend(msgjson, bet, player_vars)) != OK) {
					bet_handle_player_error(bet, retval);
					retval = OK;
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
}

int32_t bet_player_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;

	player_joined = 0;
	no_of_shares = 0;
	no_of_player_cards = 0;
	for (int i = 0; i < bet->range; i++) {
		for (int j = 0; j < bet->numplayers; j++) {
			sharesflag[i][j] = 0;
		}
	}
	number_cards_drawn = 0;
	for (int i = 0; i < hand_size; i++) {
		player_card_matrix[i] = 0;
		player_card_values[i] = -1;
	}

	vars->pot = 0;
	for (int i = 0; i < bet->maxplayers; i++) {
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			vars->bet_actions[i][j] = 0;
			vars->betamount[i][j] = 0;
		}
	}

	memset(req_identifier, 0x00, sizeof(req_identifier));
	if (sitout_value == 0) {
		retval = bet_player_stack_info_req(
			bet); //sg777 commenting this to remove the auto start of the next hand
	} else {
		reset_lock = 1;
		dlg_info("The player is choosen sitout option, so has to wait until the ongoing hand to be finished");
	}
	return retval;
}

bits256 bet_get_deckid(int32_t player_id)
{
	return all_players_info[player_id].deckid;
}

void rest_push_cards(struct lws *wsi, cJSON *argjson, int32_t this_playerID)
{
	char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };

	cJSON *init_card_info = NULL, *hole_card_info = NULL, *init_info = NULL, *board_card_info = NULL;

	init_info = cJSON_CreateObject();
	cJSON_AddStringToObject(init_info, "method", "deal");

	init_card_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(init_card_info, "dealer", 0);

	hole_card_info = cJSON_CreateArray();
	for (int32_t i = 0; ((i < no_of_hole_cards) && (i < all_number_cards_drawn[this_playerID])); i++) {
		cJSON_AddItemToArray(hole_card_info,
				     cJSON_CreateString(cards[all_player_card_values[this_playerID][i]]));
	}

	cJSON_AddItemToObject(init_card_info, "holecards", hole_card_info);

	board_card_info = cJSON_CreateArray();
	for (int32_t i = no_of_hole_cards; ((i < hand_size) && (i < all_number_cards_drawn[this_playerID])); i++) {
		cJSON_AddItemToArray(board_card_info,
				     cJSON_CreateString(cards[all_player_card_values[this_playerID][i]]));
	}

	cJSON_AddItemToObject(init_card_info, "board", board_card_info);

	cJSON_AddItemToObject(init_info, "deal", init_card_info);
	lws_write(wsi, (unsigned char *)cJSON_Print(init_info), strlen(cJSON_Print(init_info)), 0);
}

void rest_display_cards(cJSON *argjson, int32_t this_playerID)
{
	char *suit[NSUITS] = { "clubs", "diamonds", "hearts", "spades" };
	char *face[NFACES] = { "two",  "three", "four", "five",  "six",  "seven", "eight",
			       "nine", "ten",   "jack", "queen", "king", "ace" };

	char action_str[8][100] = { "", "small_blind", "big_blind", "check", "raise", "call", "allin", "fold" };
	cJSON *actions = NULL;
	int flag;

	dlg_info("******************** Player Cards ********************");
	dlg_info("Hole Cards:");
	for (int32_t i = 0; ((i < no_of_hole_cards) && (i < all_number_cards_drawn[this_playerID])); i++) {
		dlg_info("%s-->%s \t", suit[all_player_card_values[this_playerID][i] / 13],
			 face[all_player_card_values[this_playerID][i] % 13]);
	}

	flag = 1;
	for (int32_t i = no_of_hole_cards; ((i < hand_size) && (i < all_number_cards_drawn[this_playerID])); i++) {
		if (flag) {
			dlg_info("Community Cards:");
			flag = 0;
		}
		dlg_info("%s-->%s \t", suit[all_player_card_values[this_playerID][i] / 13],
			 face[all_player_card_values[this_playerID][i] % 13]);
	}

	dlg_info("******************** Betting done so far ********************");
	dlg_info("small_blind:%d, big_blind:%d", small_blind_amount, big_blind_amount);
	dlg_info("pot size:%d", jint(argjson, "pot"));
	actions = cJSON_GetObjectItem(argjson, "actions");
	int count = 0;
	flag = 1;
	for (int i = 0; ((i <= jint(argjson, "round")) && (flag)); i++) {
		dlg_info("Round:%d", i);
		for (int j = 0; ((j < BET_player[this_playerID]->maxplayers) && (flag)); j++) {
			if (jinti(actions, ((i * BET_player[this_playerID]->maxplayers) + j)) > 0)
				dlg_info("played id:%d, action: %s", j,
					 action_str[jinti(actions, ((i * BET_player[this_playerID]->maxplayers) + j))]);
			count++;
			if (count == cJSON_GetArraySize(actions))
				flag = 0;
		}
	}
}

cJSON *bet_get_available_dealers()
{
	cJSON *rqst_dealer_info = NULL, *cashier_response_info = NULL;
	cJSON *dealers_ip_info = NULL, *all_dealers_info = NULL;

	rqst_dealer_info = cJSON_CreateObject();
	cJSON_AddStringToObject(rqst_dealer_info, "method", "rqst_dealer_info");
	cJSON_AddStringToObject(rqst_dealer_info, "id", unique_id);
	all_dealers_info = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			cashier_response_info = bet_msg_cashier_with_response_id(rqst_dealer_info, notary_node_ips[i],
										 "rqst_dealer_info_response");
			if (cashier_response_info == NULL) {
				dlg_warn("No response from cashier :: %s", notary_node_ips[i]);
				continue;
			}
			dealers_ip_info = cJSON_CreateArray();
			dealers_ip_info = cJSON_GetObjectItem(cashier_response_info, "dealers_info");
			for (int32_t j = 0; j < cJSON_GetArraySize(dealers_ip_info); j++) {
				cJSON *temp = cJSON_GetArrayItem(dealers_ip_info, j);
				int flag = 1;
				for (int32_t k = 0; k < cJSON_GetArraySize(all_dealers_info); k++) {
					if (strcmp(jstr(cJSON_GetArrayItem(all_dealers_info, k), "ip"),
						   jstr(temp, "ip")) == 0) {
						flag = 0;
						break;
					}
				}
				if (flag)
					cJSON_AddItemToArray(all_dealers_info, temp);
			}
		}
	}
	return all_dealers_info;
}
