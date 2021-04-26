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

#define LWS_PLUGIN_STATIC

struct lws *wsi_global_client = NULL, *wsi_global_client_write = NULL;

int ws_connection_status = 0, ws_connection_status_write = 0;

struct lws *wsi_global_bvv = NULL;

int32_t player_card_matrix[hand_size];
int32_t player_card_values[hand_size];
int32_t number_cards_drawn = 0;

int32_t sharesflag[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t data_exists = 0;
char player_gui_data[8196];

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
int32_t all_player_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
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
int32_t backend_status = 0;
int32_t sitout_value = 0;
int32_t reset_lock = 0;

struct lws_context_creation_info lws_player_info, lws_player_info_read, lws_player_info_write;
struct lws_context *player_context = NULL, *player_context_read = NULL, *player_context_write = NULL;

void write_to_GUI(void *ptr)
{
	cJSON *data = ptr;

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(data));
#if 1
	if (ws_connection_status == 1) {
		memset(player_gui_data, 0, sizeof(player_gui_data));
		strncpy(player_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
		data_exists = 1;
		lws_callback_on_writable(wsi_global_client);
	}
#endif
}

void sg777_test(cJSON *data)
{
	pthread_t gui_thrd;

	if (OS_thread_create(&gui_thrd, NULL, (void *)write_to_GUI, (void *)data) != 0) {
		printf("error launching bet_dcv_live_loop]n");
		exit(-1);
	}
	if (pthread_join(gui_thrd, NULL)) {
		printf("\nError in joining the main thread for live_thrd");
	}
}

void player_lws_write(cJSON *data)
{
	if (backend_status == 1) {
		if (ws_connection_status_write == 1) {
			if (data_exists == 1) {
				printf("%s::%d::There is more data\n", __FUNCTION__, __LINE__);
				while (data_exists == 1) {
					printf("%s::%d::Inside while\n", __FUNCTION__, __LINE__);
					sleep(1);
				}
			}
			//printf("%s::%d::Writing data::%s\n", __FUNCTION__, __LINE__, cJSON_Print(data));
			memset(player_gui_data, 0, sizeof(player_gui_data));
			strncpy(player_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
			data_exists = 1;
			lws_callback_on_writable(wsi_global_client_write);
		}
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
	int32_t bytes, retval = 2;
	char *rendered = NULL, str[65], enc_str[177];
	cJSON *cjson_dcv_blind_cards = NULL, *cjson_peer_pubkeys = NULL, *bvv_init_info = NULL,
	      *cjson_bvv_blind_cards = NULL, *cjson_shamir_shards = NULL;
	bits256 dcv_blind_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS], peer_pubkeys[CARDS777_MAXPLAYERS];
	bits256 bvv_blinding_values[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 bvv_blind_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];

	bvv_info.numplayers = bet->numplayers;
	bvv_info.maxplayers = bet->maxplayers;
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
	printf("%s::%d::bvv_init_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(bvv_init_info));
	rendered = cJSON_Print(bvv_init_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

	return retval;
}

static int32_t bet_bvv_join_init(struct privatebet_info *bet)
{
	cJSON *bvv_response_info = NULL;
	int bytes, retval = 0;
	char *rendered = NULL;

	bvv_response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_response_info, "method", "bvv_join");
	rendered = cJSON_Print(bvv_response_info);
	printf("%s::%d::sent::%s\n", __FUNCTION__, __LINE__, cJSON_Print(bvv_response_info));
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

	return retval;
}

static cJSON *bet_player_fundchannel(char *channel_id)
{
	char **argv = NULL;
	int argc = 4;
	cJSON *fund_channel_info = NULL;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
		argv[i] = (char *)malloc(100 * sizeof(char));

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "fundchannel");
	strcpy(argv[2], channel_id);
	sprintf(argv[3], "%d", channel_fund_satoshis);

	fund_channel_info = cJSON_CreateObject();
	make_command(argc, argv, &fund_channel_info);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return fund_channel_info;
}

int32_t bet_check_bvv_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *bvv_ready = NULL;
	int32_t retval = 0, bytes;
	char *rendered = NULL;

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "bvv_ready");

	//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(bvv_ready));
	rendered = cJSON_Print(bvv_ready);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

end:
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
	int32_t recvlen, retval = 0;
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
				if ((retval = bet_bvv_backend(argjson, bet, bvv_vars)) < 0) {
					printf("%s::%d::Failed to send data\n", __FUNCTION__, __LINE__);
				} else if (retval == 2) {
					bet_bvv_reset(bet, bvv_vars);
					printf("%s::%d::The role of bvv is done for this hand\n", __FUNCTION__,
					       __LINE__);
					bvv_state = 0;
					break;
				}
			}
		}
	}
}

int32_t bet_bvv_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *method;
	int32_t retval = 0;

	if ((method = jstr(argjson, "method")) != 0) {
		printf("%s::%d::received::%s\n", __FUNCTION__, __LINE__, method);
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
			table_stack_in_chips = jdouble(argjson, "table_stack_in_chips");
			bet->maxplayers = max_players;
		}
	}
	return retval;
}

bits256 bet_decode_card(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int32_t cardid)
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

	// printf("\nDCV blinded card:%s",bits256_str(str,refval));

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
		printf("\nDecoding Failed\n");

	return tmp;
}

int32_t ln_pay_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *invoice_info = NULL, *pay_response = NULL;
	char *invoice = NULL;
	int argc, retval = 1;
	char **argv = NULL;
	int32_t player_id;

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(sizeof(char) * 1000);
	}
	player_id = jint(argjson, "playerID");
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);
	if (player_id == bet->myplayerid) {
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "pay");
		sprintf(argv[2], "%s", jstr(invoice_info, "bolt11"));
		pay_response = cJSON_CreateObject();
		make_command(argc, argv, &pay_response);

		if (jint(pay_response, "code") != 0) {
			retval = -1;
			printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(pay_response, "message"));
			goto end;
		}

		if (strcmp(jstr(pay_response, "status"), "complete") == 0)
			printf("Payment Success\n");
		else
			retval = -1;
	}
end:
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return retval;
}

static int32_t bet_player_betting_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *invoice_info = NULL, *pay_response = NULL;
	char *invoice = NULL;
	int argc, retval = 1;
	char **argv = NULL;
	int32_t player_id, bytes;
	char *rendered = NULL;
	cJSON *action_response = NULL;

	action_response = cJSON_CreateObject();
	action_response = cJSON_GetObjectItem(argjson, "actionResponse");

	argc = 3;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(sizeof(char) * 1000);
	}
	player_id = jint(argjson, "playerID");
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);
	if (player_id == bet->myplayerid) {
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "pay");
		sprintf(argv[2], "%s", jstr(invoice_info, "bolt11"));
		pay_response = cJSON_CreateObject();

		make_command(argc, argv, &pay_response);

		if (jint(pay_response, "code") != 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(pay_response));
			retval = -1;
			goto end;
		}

		if (strcmp(jstr(pay_response, "status"), "complete") == 0)
			printf("Payment Success\n");
		else
			retval = -1;

		rendered = cJSON_Print(action_response);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			retval = -1;
			goto end;
		}
	}
end:
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return retval;
}

static int32_t bet_player_winner(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, bytes, retval = 1;
	char **argv = NULL, hexstr[65], *rendered = NULL;
	cJSON *invoice_info = NULL, *winner_invoice_info = NULL;

	if (jint(argjson, "playerid") == bet->myplayerid) {
		argc = 5;
		argv = (char **)malloc(argc * sizeof(char *));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(sizeof(char) * 1000);
		}

		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "invoice");
		sprintf(argv[2], "%d", jint(argjson, "winning_amount"));
		sprintf(argv[3], "%s_%d", bits256_str(hexstr, player_info.deckid), jint(argjson, "winning_amount"));
		sprintf(argv[4], "Winning claim");

		winner_invoice_info = cJSON_CreateObject();
		make_command(argc, argv, &winner_invoice_info);
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "claim");
		cJSON_AddNumberToObject(invoice_info, "playerid", bet->myplayerid);
		cJSON_AddStringToObject(invoice_info, "label", argv[3]);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(winner_invoice_info));

		rendered = cJSON_Print(invoice_info);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			retval = -1;
			printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
			goto end;
		}
	}
end:
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}

static int32_t bet_player_bet_round(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t bet_amount, round, bytes, retval = 1;
	cJSON *bet_info = NULL;
	char *rendered = NULL;
	round = jint(argjson, "round");
	printf("\nEnter Betting Amount in MilliSatoshis:");
	scanf("%d", &bet_amount);

	bet_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bet_info, "method", "invoiceRequest");
	cJSON_AddNumberToObject(bet_info, "round", round);
	cJSON_AddNumberToObject(bet_info, "playerID", bet->myplayerid);
	cJSON_AddNumberToObject(bet_info, "betAmount", bet_amount);

	rendered = cJSON_Print(bet_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
		goto end;
	}
end:
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
	int32_t retval = 1, bytes, cardid, playerid, errs = 0, unpermi, card_type;
	cJSON *player_card_info = NULL;
	char *rendered = NULL;
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
	if (no_of_shares == bet->maxplayers) {
		no_of_shares = 0;
		decoded256 = bet_decode_card(argjson, bet, vars, cardid);
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

			rendered = cJSON_Print(player_card_info);
			bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
			if (bytes < 0) {
				retval = -1;
				printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
				goto end;
			}
		}
	}
end:
	return retval;
}

static int32_t bet_player_ask_share(struct privatebet_info *bet, int32_t cardid, int32_t playerid, int32_t card_type)
{
	cJSON *request_info = NULL;
	char *rendered = NULL;
	int32_t bytes, retval = 1;

	request_info = cJSON_CreateObject();
	cJSON_AddStringToObject(request_info, "method", "requestShare");
	cJSON_AddNumberToObject(request_info, "playerid", playerid);
	cJSON_AddNumberToObject(request_info, "cardid", cardid);
	cJSON_AddNumberToObject(request_info, "card_type", card_type);

	rendered = cJSON_Print(request_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_client_give_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, bytes, playerid, cardid, recvlen, card_type;
	cJSON *share_info = NULL;
	char *rendered = NULL;
	struct enc_share temp;
	uint8_t decipher[sizeof(bits256) + 1024], *ptr;
	bits256 share;

	playerid = jint(argjson, "playerid");
	cardid = jint(argjson, "cardid");
	card_type = jint(argjson, "card_type");

	if (playerid == bet->myplayerid)
		goto end;

	temp = g_shares[playerid * bet->numplayers * bet->range + (cardid * bet->numplayers + bet->myplayerid)];

	recvlen = sizeof(temp);

	if ((ptr = bet_decrypt(decipher, sizeof(decipher), player_info.bvvpubkey, player_info.player_key.priv,
			       temp.bytes, &recvlen)) == 0) {
		retval = -1;
		printf("%s::%d::decrypt error \n", __FUNCTION__, __LINE__);
		goto end;
	} else {
		memcpy(share.bytes, ptr, recvlen);
		share_info = cJSON_CreateObject();
		cJSON_AddStringToObject(share_info, "method", "share_info");
		cJSON_AddNumberToObject(share_info, "playerid", bet->myplayerid);
		cJSON_AddNumberToObject(share_info, "cardid", cardid);
		cJSON_AddNumberToObject(share_info, "card_type", card_type);
		jaddbits256(share_info, "share", share);

		rendered = cJSON_Print(share_info);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

		if (bytes < 0) {
			retval = -1;
			printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
		}
	}
end:
	return retval;
}

int32_t bet_get_own_share(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	struct enc_share temp;
	int32_t cardid, retval = 1, playerid, recvlen;
	uint8_t decipher[sizeof(bits256) + 1024], *ptr;
	bits256 share;

	playerid = jint(argjson, "playerid");
	cardid = jint(argjson, "cardid");

	temp = g_shares[bet->myplayerid * bet->numplayers * bet->range + (cardid * bet->numplayers + playerid)];
	recvlen = sizeof(temp);

	printf("%s::%d::share position::%d\n", __FUNCTION__, __LINE__,
	       (bet->myplayerid * bet->numplayers * bet->range + (cardid * bet->numplayers + playerid)));
	printf("%s::%d::bet->myplayerid::%d  bet->numplayers::%d  bet->range ::%d cardid ::%d playerid::%d\n",
	       __FUNCTION__, __LINE__, bet->myplayerid, bet->numplayers, bet->range, cardid, playerid);

	if ((ptr = bet_decrypt(decipher, sizeof(decipher), player_info.bvvpubkey, player_info.player_key.priv,
			       temp.bytes, &recvlen)) == 0) {
		retval = -1;
		printf("%s::%d::decrypt error\n", __FUNCTION__, __LINE__);
		goto end;
	} else {
		memcpy(share.bytes, ptr, recvlen);
		playershares[cardid][bet->myplayerid] = share;
		sharesflag[cardid][bet->myplayerid] = 1;
	}
end:
	return retval;
}

int32_t bet_client_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, playerid;

	playerid = jint(argjson, "playerid");
	printf("%s::%d::playerid::%d::%s::\n", __FUNCTION__, __LINE__, playerid, cJSON_Print(argjson));

	if (playerid == bet->myplayerid) {
		no_of_shares = 1;
		retval = bet_get_own_share(argjson, bet, vars);
		if (retval == -1) {
			printf("Failing to get own share: Decryption Error");
			goto end;
		}

		for (int i = 0; i < bet->numplayers; i++) {
			if ((!sharesflag[jint(argjson, "cardid")][i]) && (i != bet->myplayerid)) {
				retval = bet_player_ask_share(bet, jint(argjson, "cardid"), jint(argjson, "playerid"),
							      jint(argjson, "card_type"));
			}
		}
	}
end:
	return retval;
}

int32_t bet_player_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *player_ready = NULL;
	char *rendered = NULL;
	int bytes, retval = 1;

	player_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(player_ready, "method", "player_ready");
	cJSON_AddNumberToObject(player_ready, "playerid", bet->myplayerid);
	rendered = cJSON_Print(player_ready);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_client_bvv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)

{
	int32_t retval = 1;
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
	int32_t retval = 1;
	cJSON *cjson_card_prods, *cjsong_hash;

	player_info.deckid = jbits256(dcv_info, "deckid");
	cjson_card_prods = cJSON_GetObjectItem(dcv_info, "cardprods");

	//printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(cjson_card_prods));

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
	int32_t bytes, retval = 1;
	cJSON *cjson_player_cards, *init_p = NULL;
	char *rendered = NULL, str[65];

	init_p = cJSON_CreateObject();

	cJSON_AddStringToObject(init_p, "method", "init_p");
	cJSON_AddNumberToObject(init_p, "peerid", bet->myplayerid);
	jaddbits256(init_p, "pubkey", player_info.player_key.prod);
	cJSON_AddItemToObject(init_p, "cardinfo", cjson_player_cards = cJSON_CreateArray());
	for (int i = 0; i < bet->range; i++) {
		cJSON_AddItemToArray(cjson_player_cards,
				     cJSON_CreateString(bits256_str(str, player_info.cardpubkeys[i])));
	}
	printf("%s::%d::init_p::%s\n", __FUNCTION__, __LINE__, cJSON_Print(init_p));
	rendered = cJSON_Print(init_p);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
	if (bytes < 0) {
		retval = -1;
		printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
		goto end;
	}
end:
	return retval;
}

static int32_t bet_find_channel_balance(char *uri)
{
	int argc = 2, buf_size = 100;
	char **argv = NULL;
	cJSON *listfundsInfo = NULL, *channelsInfo = NULL;
	int balance = 0;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(buf_size * sizeof(char));
	}
	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "listfunds");

	listfundsInfo = cJSON_CreateObject();
	make_command(argc, argv, &listfundsInfo);

	channelsInfo = cJSON_CreateObject();
	channelsInfo = cJSON_GetObjectItem(listfundsInfo, "channels");
	for (int i = 0; i < cJSON_GetArraySize(channelsInfo); i++) {
		cJSON *temp = cJSON_GetArrayItem(channelsInfo, i);
		if (strcmp(jstr(temp, "peer_id"), uri) == 0)
			balance = jint(temp, "channel_sat") / 100000;
	}

	return balance;
}

static int32_t bet_check_player_stack(char *uri)
{
	int balance = 0;
	balance = bet_find_channel_balance(uri);
	balance = balance * 100;
	if (balance >= table_stack) {
		balance = table_stack;
	} else {
		printf("%s::%d::Insufficient Funds, Minimum needed::%d mCHIPS but only %d "
		       "exists on the channel\n",
		       __FUNCTION__, __LINE__, table_stack, balance);
	}
	return balance;
}
int32_t bet_client_join_res(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char uri[100], *rendered = NULL;
	int argc, retval = 1, channel_state, /* balance,*/ bytes;
	char **argv = NULL, channel_id[100];
	cJSON *init_card_info = NULL, *hole_card_info = NULL, *init_info = NULL, *stack_info = NULL;

	if (0 == bits256_cmp(player_info.player_key.prod, jbits256(argjson, "pubkey"))) {
		bet_player->myplayerid = jint(argjson, "playerid");
		bet->myplayerid = jint(argjson, "playerid");

		strcpy(uri, jstr(argjson, "uri"));
		strcpy(channel_id, strtok(jstr(argjson, "uri"), "@"));
		channel_state = ln_get_channel_status(channel_id);
		if ((channel_state != CHANNELD_AWAITING_LOCKIN) && (channel_state != CHANNELD_NORMAL)) {
			retval = ln_establish_channel(uri);
			if (retval == 1)
				printf("Channel Established\n");
			else
				printf("Channel Didn't Established\n");
		} else {
			strcpy(uri, jstr(argjson, "uri"));
			ln_check_peer_and_connect(uri);
		}
		init_card_info = cJSON_CreateObject();
		cJSON_AddNumberToObject(init_card_info, "dealer", jint(argjson, "dealer"));

		/*
		balance = bet_check_player_stack(jstr(argjson, "uri"));
		if (vars->player_funds == 0) {
			// refill the player stack only if it
			// becomes zero and funds are available
			vars->player_funds = balance;
		}
		*/
		// Here if the balance is not table_stack it should wait for the refill
		cJSON_AddNumberToObject(init_card_info, "balance", vars->player_funds);

		hole_card_info = cJSON_CreateArray();
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToObject(init_card_info, "holecards", hole_card_info);

		init_info = cJSON_CreateObject();
		cJSON_AddStringToObject(init_info, "method", "deal");
		cJSON_AddItemToObject(init_info, "deal", init_card_info);

		printf("%s::%d::init_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(init_info));
		player_lws_write(init_info);

		stack_info = cJSON_CreateObject();
		cJSON_AddStringToObject(stack_info, "method", "stack");
		cJSON_AddNumberToObject(stack_info, "playerid", bet->myplayerid);
		cJSON_AddNumberToObject(stack_info, "stack_value", vars->player_funds);

		printf("%s::%d::stack_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(stack_info));
		rendered = cJSON_Print(stack_info);
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
			goto end;
		}
	}
end:
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}

int32_t bet_client_join(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t bytes, argc, retval = -1;
	cJSON *joininfo = NULL, *channel_info = NULL, *addresses = NULL, *address = NULL;
	struct pair256 key;
	char **argv = NULL, *rendered = NULL, *uri = NULL;

	if (bet->pushsock >= 0) {
		key = deckgen_player(player_info.cardprivkeys, player_info.cardpubkeys, player_info.permis, bet->range);
		player_info.player_key = key;
		joininfo = cJSON_CreateObject();
		cJSON_AddStringToObject(joininfo, "method", "join_req");
		jaddbits256(joininfo, "pubkey", key.prod);
		argc = 2;
		argv = (char **)malloc(argc * sizeof(char *));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(100 * sizeof(char));
		}

		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "getinfo");
		channel_info = cJSON_CreateObject();

		make_command(argc, argv, &channel_info);

		if (jint(channel_info, "code") != 0) {
			retval = -1;
			printf("\n%s:%d:Message:%s", __FUNCTION__, __LINE__, jstr(channel_info, "message"));
			goto end;
		}

		uri = (char *)malloc(sizeof(char) * 100);

		strcpy(uri, jstr(channel_info, "id"));
		strcat(uri, "@");

		addresses = cJSON_CreateObject();
		addresses = cJSON_GetObjectItem(channel_info, "address");

		address = cJSON_CreateObject();
		address = cJSON_GetArrayItem(addresses, 0);

		strcat(uri, jstr(address, "address"));
		cJSON_AddStringToObject(joininfo, "uri", uri);
		cJSON_AddNumberToObject(joininfo, "gui_playerID", (jint(argjson, "gui_playerID") - 1));
		cJSON_AddStringToObject(joininfo, "req_identifier", req_identifier);
		rendered = cJSON_Print(joininfo);
		printf("%s::%d::joininfo::%s\n", __FUNCTION__, __LINE__, cJSON_Print(joininfo));
		bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
			goto end;
		}
		retval = 1;
	}
end:
	if (uri)
		free(uri);
	if (argv) {
		for (int i = 0; i < argc; i++)
			free(argv[i]);

		free(argv);
	}

	if (retval == -1)
		printf("%s::%d::Error\n", __FUNCTION__, __LINE__);
	return retval;
}

static int32_t bet_player_process_player_join(cJSON *argjson)
{
	int32_t retval = 1;
	cJSON *warning_info = NULL;

	if (player_joined == 0) {
		if (backend_status == 1) {
			retval = bet_client_join(argjson, bet_player);
		} else {
			warning_info = cJSON_CreateObject();
			cJSON_AddStringToObject(warning_info, "method", "warning");
			//cJSON_AddStringToObject(info, "response_to", "player_join");
			cJSON_AddNumberToObject(warning_info, "warning_num", backend_not_ready);
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(warning_info));
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
	cJSON_AddNumberToObject(wallet_info, "table_stack_in_chips",
				(table_stack_in_chips * satoshis) / (satoshis_per_unit * normalization_factor));
	cJSON_AddStringToObject(wallet_info, "table_id", table_id);
	cJSON_AddNumberToObject(wallet_info, "tx_fee", chips_tx_fee);
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(wallet_info));
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

int32_t bet_player_frontend(struct lws *wsi, cJSON *argjson)
{
	int32_t retval = 1;
	char *method = NULL;

	if ((method = jstr(argjson, "method")) != 0) {
		//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
		if (strcmp(method, "player_join") == 0) {
			bet_player_process_player_join(argjson);
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_round_betting(argjson, bet_player, player_vars);
		} else if (strcmp(method, "reset") == 0) {
			retval = bet_player_reset(bet_player, player_vars);
		} else if (strcmp(method, "withdrawRequest") == 0) {
			bet_player_withdraw_request();
		} else if (strcmp(method, "withdraw") == 0) {
			bet_player_withdraw(argjson);
		} else if (strcmp(method, "get_bal_info") == 0) {
			player_lws_write(bet_get_chips_ln_bal_info());
		} else if (strcmp(method, "get_addr_info") == 0) {
			player_lws_write(bet_get_chips_ln_addr_info());
		} else if (strcmp(method, "walletInfo") == 0) {
			bet_player_wallet_info();
		} else if (strcmp(method, "backend_status") == 0) {
			bet_player_process_be_status();
		} else if (strcmp(method, "sitout") == 0) {
			sitout_value = jint(argjson, "value");
			/*
			if(jint(argjson,"value") == 0) {
				bet_player_stack_info_req(bet_player);
			}
			*/
		} else {
			bet_player_handle_invalid_method(method);
		}
	}
	return retval;
}

#if 0
static void bet_init_seat_info()
{
	cJSON *seats_info = NULL, *table_info = NULL;
	cJSON *seat[max_players];

	bet_init_player_seats_info();
	seats_info = cJSON_CreateArray();
	for (int i = 0; i < max_players; i++) {
		seat[i] = cJSON_CreateObject();
		initialize_seat(seat[i], player_seats_info[i].seat_name, player_seats_info[i].seat,
				player_seats_info[i].chips, player_seats_info[i].empty, player_seats_info[i].playing);
		cJSON_AddItemToArray(seats_info, seat[i]);
	}

	table_info = cJSON_CreateObject();
	cJSON_AddStringToObject(table_info, "method", "seats");
	cJSON_AddItemToObject(table_info, "seats", seats_info);
	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(table_info));
	player_lws_write(table_info);
}
#endif

static void bet_gui_init_message(struct privatebet_info *bet)
{
	cJSON *warning_info = NULL;
	cJSON *req_seats_info = NULL;

	if (backend_status == 0) {
		warning_info = cJSON_CreateObject();
		cJSON_AddStringToObject(warning_info, "method", "warning");
		cJSON_AddNumberToObject(warning_info, "warning_num", backend_not_ready);
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(warning_info));
		player_lws_write(warning_info);
	} else {
		req_seats_info = cJSON_CreateObject();
		cJSON_AddStringToObject(req_seats_info, "method", "req_seats_info");
		cJSON_AddStringToObject(req_seats_info, "req_identifier", req_identifier);
		printf("%s::%d::joininfo::%s\n", __FUNCTION__, __LINE__, cJSON_Print(req_seats_info));
		nn_send(bet->pushsock, cJSON_Print(req_seats_info), strlen(cJSON_Print(req_seats_info)), 0);
	}
}

int lws_callback_http_player_write(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;

	//printf("%s::%d::In write callback code::%d\n", __FUNCTION__, __LINE__, (int)reason);
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf_1);

		if (bet_player_frontend(wsi, argjson) != 1) {
			printf("\n%s:%d:Failed to process the host command", __FUNCTION__, __LINE__);
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client_write = wsi;
		printf("%s:%d::Write::LWS_CALLBACK_ESTABLISHED\n", __FUNCTION__, __LINE__);
		ws_connection_status_write = 1;
		bet_gui_init_message(bet_player);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		printf("%s::%d::LWS_CALLBACK_SERVER_WRITEABLE triggered\n", __FUNCTION__, __LINE__);
		if (data_exists) {
			if (player_gui_data) {
				lws_write(wsi, player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	}
	return 0;
}

int lws_callback_http_player_read(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;

	//printf("%s::%d::In read callback code::%d\n", __FUNCTION__, __LINE__, (int)reason);
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;

		argjson = cJSON_Parse(unstringify(lws_buf_1));
		if (bet_player_frontend(wsi, argjson) != 1) {
			printf("\n%s:%d:Failed to process the host command", __FUNCTION__, __LINE__);
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client = wsi;
		printf("%s:%d::Read::LWS_CALLBACK_ESTABLISHED\n", __FUNCTION__, __LINE__);
		ws_connection_status = 1;
		//bet_gui_init_message(bet_player);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		printf("%s::%d::LWS_CALLBACK_SERVER_WRITEABLE triggered\n", __FUNCTION__, __LINE__);
		if (data_exists) {
			if (player_gui_data) {
				lws_write(wsi, player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	}
	return 0;
}

int lws_callback_http_player(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;

	printf("%s::%d::callback code::%d\n", __FUNCTION__, __LINE__, (int)reason);
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_1 + lws_buf_length_1, in, len);
		lws_buf_length_1 += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf_1);

		if (bet_player_frontend(wsi, argjson) != 1) {
			printf("\n%s:%d:Failed to process the host command", __FUNCTION__, __LINE__);
		}
		memset(lws_buf_1, 0x00, sizeof(lws_buf_1));
		lws_buf_length_1 = 0;

		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_client = wsi;
		printf("%s:%d::LWS_CALLBACK_ESTABLISHED\n", __FUNCTION__, __LINE__);
		ws_connection_status = 1;
		bet_gui_init_message(bet_player);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		printf("%s::%d::LWS_CALLBACK_SERVER_WRITEABLE triggered\n", __FUNCTION__, __LINE__);
		if (data_exists) {
			if (player_gui_data) {
				lws_write(wsi, player_gui_data, strlen(player_gui_data), 0);
				data_exists = 0;
			}
		}
		break;
	}
	return 0;
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

	// signal(SIGINT,player_sigint_handler);
	lws_set_log_level(logs, NULL);

	memset(&lws_player_info_read, 0, sizeof lws_player_info_read); /* otherwise uninitialized garbage */
	lws_player_info_read.port = 9001;
	lws_player_info_read.mounts = &lws_http_mount_player;
	lws_player_info_read.protocols = player_http_protocol_read;
	lws_player_info_read.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	player_context_read = lws_create_context(&lws_player_info_read);
	if (!player_context_read) {
		printf("lws init failed\n");
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
	lws_player_info_write.port = 9000;
	lws_player_info_write.mounts = &lws_http_mount_player;
	lws_player_info_write.protocols = player_http_protocol_write;
	lws_player_info_write.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	player_context_write = lws_create_context(&lws_player_info_write);
	if (!player_context_write) {
		printf("lws init failed\n");
	}
	while (n >= 0 && !interrupted1) {
		n = lws_service(player_context_write, 1000);
	}
}

void bet_player_frontend_loop(void *_ptr)
{
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	// signal(SIGINT,player_sigint_handler);
	lws_set_log_level(logs, NULL);

	memset(&lws_player_info, 0, sizeof lws_player_info); /* otherwise uninitialized garbage */
	lws_player_info.port = 9000;
	lws_player_info.mounts = &lws_http_mount_player;
	lws_player_info.protocols = player_http_protocol;
	lws_player_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	player_context = lws_create_context(&lws_player_info);
	if (!player_context) {
		printf("lws init failed\n");
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
	cJSON_AddNumberToObject(join_info, "seat_taken", jint(argjson, "seat_taken"));
	printf("%s::%d::join_info::%s\n", __FUNCTION__, __LINE__, cJSON_Print(join_info));
	player_lws_write(join_info);
}

static int32_t bet_player_handle_stack_info_resp(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *tx_info = NULL, *txid = NULL;
	double funds_needed;
	int32_t retval = 1, bytes;
	char *hex_data = NULL, *sql_query = NULL;

	funds_needed = jdouble(argjson, "table_stack_in_chips");
	if (chips_get_balance() < (funds_needed + chips_tx_fee)) {
		printf("%s::%d::Insufficient funds\n", __FUNCTION__, __LINE__);
		retval = -1;
	} else {
		max_players = jint(argjson, "max_players");
		table_stack_in_chips = jdouble(argjson, "table_stack_in_chips");
		chips_tx_fee = jdouble(argjson, "chips_tx_fee");
		legacy_m_of_n_msig_addr = (char *)malloc(strlen(jstr(argjson, "legacy_m_of_n_msig_addr")) + 1);
		memset(legacy_m_of_n_msig_addr, 0x00, strlen(jstr(argjson, "legacy_m_of_n_msig_addr")) + 1);
		strncpy(legacy_m_of_n_msig_addr, jstr(argjson, "legacy_m_of_n_msig_addr"),
			strlen(jstr(argjson, "legacy_m_of_n_msig_addr")));
		threshold_value = jint(argjson, "threshold_value");
		memset(table_id, 0x00, sizeof(table_id));
		strncpy(table_id, jstr(argjson, "table_id"), strlen(jstr(argjson, "table_id")));

		bet->maxplayers = max_players;
		tx_info = cJSON_CreateObject();
		cJSON *data_info = NULL;
		data_info = cJSON_CreateObject();
		cJSON_AddStringToObject(data_info, "table_id", table_id);
		cJSON_AddStringToObject(data_info, "msig_addr_nodes",
					unstringify(cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes"))));
		cJSON_AddNumberToObject(data_info, "min_cashiers", threshold_value);
		cJSON_AddStringToObject(data_info, "player_id", req_identifier);
		cJSON_AddStringToObject(data_info, "dispute_addr", chips_get_new_address());
		cJSON_AddStringToObject(data_info, "msig_addr", legacy_m_of_n_msig_addr);

		hex_data = calloc(1, 2 * tx_data_size);
		str_to_hexstr(cJSON_Print(data_info), hex_data);
		txid = cJSON_CreateObject();
#if 0 //sg777 commenting for testing purposes
		txid = chips_transfer_funds_with_data(funds_needed, legacy_m_of_n_msig_addr, hex_data); 

		printf("%s::%d::txid::%s\n", __FUNCTION__, __LINE__, cJSON_Print(txid));
		if (txid) {
			sql_query = calloc(1, sql_query_size);
			sprintf(sql_query, "INSERT INTO player_tx_mapping values(%s,\'%s\',\'%s\',\'%s\',%d,%d, NULL);",
				cJSON_Print(txid), table_id, req_identifier,
				cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes")), tx_unspent,
				threshold_value);
			bet_run_query(sql_query);

			cJSON *msig_addr_nodes = cJSON_CreateArray();
			msig_addr_nodes = cJSON_GetObjectItem(argjson, "msig_addr_nodes");
			sprintf(sql_query, "INSERT INTO c_tx_addr_mapping values(%s,\'%s\',%d,\'%s\',\'%s\',1,NULL);",
				cJSON_Print(txid), legacy_m_of_n_msig_addr, threshold_value, table_id,
				unstringify(cJSON_Print(cJSON_GetObjectItem(argjson, "msig_addr_nodes"))));

			cJSON *temp = cJSON_CreateObject();
			cJSON_AddStringToObject(temp, "method", "lock_in_tx");
			cJSON_AddStringToObject(temp, "sql_query", sql_query);
			for (int32_t i = 0; i < cJSON_GetArraySize(msig_addr_nodes); i++) {
				bet_msg_cashier(temp, unstringify(cJSON_Print(cJSON_GetArrayItem(msig_addr_nodes, i))));
			}
		}
#endif
		cJSON_AddStringToObject(tx_info, "method", "tx");
		cJSON_AddStringToObject(tx_info, "id", req_identifier);
		cJSON_AddStringToObject(tx_info, "chips_addr", chips_get_new_address());
		cJSON_AddItemToObject(tx_info, "tx_info", txid);

#if 0 //sg777 this is commented for testing purposes
		if (txid) {
			while (chips_get_block_hash_from_txid(cJSON_Print(txid)) == NULL) {
				printf("%s::%d::Waiting for tx to confirm\r", __FUNCTION__, __LINE__);
				sleep(2);
			}
			printf("\n");
			cJSON_AddNumberToObject(tx_info, "block_height",
						chips_get_block_height_from_block_hash(
							chips_get_block_hash_from_txid(cJSON_Print(txid))));
		} else {
			cJSON_AddNumberToObject(tx_info, "block_height", -1);
		}
#endif

		bytes = nn_send(bet->pushsock, cJSON_Print(tx_info), strlen(cJSON_Print(tx_info)), 0);
		if (bytes < 0)
			retval = -1;
	}
	if (sql_query)
		free(sql_query);
	if (hex_data)
		free(hex_data);
	return retval;
}

int32_t bet_player_stack_info_req(struct privatebet_info *bet)
{
	int32_t bytes, retval = 1;
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
	bytes = nn_send(bet->pushsock, cJSON_Print(stack_info_req), strlen(cJSON_Print(stack_info_req)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

static int32_t bet_player_process_payout_tx(cJSON *argjson)
{
	char *sql_query = NULL;
	int32_t rc;

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
	sql_query = calloc(1, 400);
	sprintf(sql_query, "UPDATE player_tx_mapping set status = 0,payout_tx_id = \'%s\' where table_id = \'%s\'",
		jstr(argjson, "tx_info"), jstr(argjson, "table_id"));
	rc = bet_run_query(sql_query);
	if (sql_query)
		free(sql_query);
	return rc;
}

static int32_t bet_player_process_game_info(cJSON *argjson)
{
	int argc = 3, rc;
	char **argv = NULL;
	char *sql_query = calloc(1, arg_size);

	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "player_game_state");
	sprintf(argv[1], "\'%s\'", table_id);
	sprintf(argv[2], "\'%s\'", cJSON_Print(cJSON_GetObjectItem(argjson, "game_state")));

	bet_make_insert_query(argc, argv, &sql_query);
	rc = bet_run_query(sql_query);
	bet_dealloc_args(argc, &argv);
	if (sql_query)
		free(sql_query);
	return rc;
}

static void bet_update_seat_info(cJSON *argjson)
{
	cJSON *seats_info = NULL;

	seats_info = cJSON_CreateObject();
	cJSON_AddStringToObject(seats_info, "method", "seats");
	cJSON_AddItemToObject(seats_info, "seats", cJSON_GetObjectItem(argjson, "seats"));
	//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(seats_info));
	player_lws_write(seats_info);
}

int32_t bet_player_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, bytes;
	char *method = NULL;
	char hexstr[65], *rendered = NULL;

	if (strcmp(jstr(argjson, "method"), "reset") == 0) {
		reset_lock = 0;
		retval = bet_player_reset(bet, vars);
	}
	if (reset_lock == 1) {
		return retval;
	}
	if ((method = jstr(argjson, "method")) != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, method);

		if (strcmp(method, "join_res") == 0) {
			//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
			bet_update_seat_info(argjson);
			if (strcmp(jstr(argjson, "req_identifier"), req_identifier) == 0) {
				bet_push_join_info(argjson);
				if (jint(argjson, "seat_taken") == 0) {
					retval = bet_client_join_res(argjson, bet, vars);
				}
			}
		} else if (strcmp(method, "init") == 0) {
			if (jint(argjson, "peerid") == bet->myplayerid) {
				bet_player_blinds_info();
				//printf("%s::%d::myplayerid::%d::init::%s\n", __FUNCTION__, __LINE__, bet->myplayerid,
				       cJSON_Print(argjson));
				retval = bet_client_init(argjson, bet, vars);
			}
		} else if (strcmp(method, "init_d") == 0) {
			retval = bet_client_dcv_init(argjson, bet, vars);
		} else if (strcmp(method, "init_b") == 0) {
			retval = bet_client_bvv_init(argjson, bet, vars);
		} else if (strcmp(method, "turn") == 0) {
			retval = bet_client_turn(argjson, bet, vars);
		} else if (strcmp(method, "ask_share") == 0) {
			retval = bet_client_give_share(argjson, bet, vars);
		} else if (strcmp(method, "requestShare") == 0) {
			retval = bet_client_give_share(argjson, bet, vars);
		} else if (strcmp(method, "share_info") == 0) {
			retval = bet_client_receive_share(argjson, bet, vars);
		} else if (strcmp(method, "bet") == 0) {
			retval = bet_player_bet_round(argjson, bet, vars);
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
		} else if (strcmp(method, "stack") == 0) {
			vars->funds[jint(argjson, "playerid")] = jint(argjson, "stack_value");
		} else if (strcmp(method, "signrawtransaction") == 0) {
			if (jint(argjson, "playerid") == bet->myplayerid) {
				cJSON *temp = cJSON_CreateObject();
				temp = chips_sign_raw_tx_with_wallet(jstr(argjson, "tx"));
				cJSON *signedTxInfo = cJSON_CreateObject();

				cJSON_AddStringToObject(signedTxInfo, "method", "signedrawtransaction");
				cJSON_AddStringToObject(signedTxInfo, "tx", jstr(temp, "hex"));
				cJSON_AddNumberToObject(signedTxInfo, "playerid", bet->myplayerid);

				rendered = cJSON_Print(signedTxInfo);
				bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
				if (bytes < 0)
					retval = -1;
			}
		} else if (strcmp(method, "stack_info_resp") == 0) {
			//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
			max_players = jint(argjson, "max_players");
			bet_player->maxplayers = max_players;
			bet_player->numplayers = max_players;
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				retval = bet_player_handle_stack_info_resp(argjson, bet);
			}
		} else if (strcmp(method, "tx_status") == 0) {
			//printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
			if (strcmp(req_identifier, jstr(argjson, "id")) == 0) {
				vars->player_funds = jint(argjson, "player_funds");
				if (jint(argjson, "tx_validity") == 1) {
					if (backend_status ==
					    1) { /* This snippet is added to handle the reset scenario after the initial hand got played*/
						cJSON *reset_info = cJSON_CreateObject();
						cJSON_AddStringToObject(reset_info, "method", "reset");
						player_lws_write(reset_info);
					}
					backend_status = 1;
					cJSON *info = cJSON_CreateObject();
					cJSON_AddStringToObject(info, "method", "backend_status");
					cJSON_AddNumberToObject(info, "backend_status", backend_status);
					player_lws_write(info);
					cJSON *req_seats_info = NULL;
					req_seats_info = cJSON_CreateObject();
					cJSON_AddStringToObject(req_seats_info, "method", "req_seats_info");
					cJSON_AddStringToObject(req_seats_info, "req_identifier", req_identifier);
					printf("%s::%d::joininfo::%s\n", __FUNCTION__, __LINE__,
					       cJSON_Print(req_seats_info));
					nn_send(bet->pushsock, cJSON_Print(req_seats_info),
						strlen(cJSON_Print(req_seats_info)), 0);

				} else {
					printf("%s::%d::%s\n", __FUNCTION__, __LINE__,
					       "unable make lock_in transaction");
					retval = 0;
				}
			}
		} else if (strcmp(method, "payout_tx") == 0) {
			retval = bet_player_process_payout_tx(argjson);
		} else if (strcmp(method, "game_info") == 0) {
			retval = bet_player_process_game_info(argjson);
		} else if (strcmp(method, "dcv_state") == 0) {
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				if (jint(argjson, "dcv_state") == 1) {
					printf("DCV which you trying to connect is full\n");
					printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
					bet_player_reset(bet, vars);
					retval = -1;
				}
			}
		} else if (strcmp(method, "tx_reverse") == 0) {
			if (strncmp(req_identifier, jstr(argjson, "id"), sizeof(req_identifier)) == 0) {
				printf("%s::%d::The dealers table is already full, claim your tx back using dispute resolution protocol::%s\n",
				       __FUNCTION__, __LINE__, cJSON_Print(argjson));
				retval = -1;
			}
		} else if (strcmp(method, "seats_info_resp") == 0) {
			if (strcmp(req_identifier, jstr(argjson, "req_identifier")) == 0) {
				cJSON *seats_info = cJSON_CreateObject();
				cJSON_AddStringToObject(seats_info, "method", "seats");
				cJSON_AddItemToObject(seats_info, "seats", cJSON_GetObjectItem(argjson, "seats"));
				player_lws_write(seats_info);
			}
		} else if (strcmp(method, "config_data") == 0) {
			printf("%s::%d::config_data::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
		} else if (strcmp(method, "is_player_active") == 0) {
			cJSON *active_info = NULL;
			active_info = cJSON_CreateObject();
			cJSON_AddStringToObject(active_info, "method", "player_active");
			cJSON_AddNumberToObject(active_info, "playerid", bet->myplayerid);
			cJSON_AddStringToObject(active_info, "req_identifier", req_identifier);
			bytes = nn_send(bet->pushsock, cJSON_Print(active_info), strlen(cJSON_Print(active_info)), 0);
			if (bytes < 0) {
				printf("%s::%d::There is a problem in sending the data", __FUNCTION__, __LINE__);
			}
		} else if (strcmp(method, "active_player_info") == 0) {
			player_lws_write(argjson);
		} else if (strcmp(method, "check_bvv_ready") == 0) {
			// Do nothing, this broadcast is for BVV nodes
		} else {
			printf("%s::%d:: %s method is not handled in the backend\n", __FUNCTION__, __LINE__, method);
		}
	}
	return retval;
}

void bet_player_backend_loop(void *_ptr)
{
	int32_t recvlen = 0;
	void *ptr = NULL;
	cJSON *msgjson = NULL;
	struct privatebet_info *bet = _ptr;
	uint8_t flag = 1;

	flag = bet_player_stack_info_req(bet);

	while (flag) {
		if (bet->subsock >= 0 && bet->pushsock >= 0) {
			ptr = 0;
			char *tmp = NULL;
			recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0);
			if (recvlen > 0)
				tmp = clonestr(ptr);
			if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
				if (bet_player_backend(msgjson, bet, player_vars) < 0) {
					// do something here, possibly this could be because unknown
					// commnad or because of encountering a special case which
					// state machine fails to handle
					//break;
					exit(-1);
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
	int32_t retval = 1;
	//cJSON *reset_info = NULL;

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
		bet_player_stack_info_req(bet); //sg777 commenting this to remove the auto start of the next hand
	} else {
		reset_lock = 1;
		printf("The player is choosen sitout option, so has to wait until the ongoing hand to be finished\n");
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
	lws_write(wsi, cJSON_Print(init_info), strlen(cJSON_Print(init_info)), 0);
}

void rest_display_cards(cJSON *argjson, int32_t this_playerID)
{
	char *suit[NSUITS] = { "clubs", "diamonds", "hearts", "spades" };
	char *face[NFACES] = { "two",  "three", "four", "five",	 "six",	 "seven", "eight",
			       "nine", "ten",	"jack", "queen", "king", "ace" };

	char action_str[8][100] = { "", "small_blind", "big_blind", "check", "raise", "call", "allin", "fold" };
	cJSON *actions = NULL;
	int flag;

	printf("\n******************** Player Cards ********************");
	printf("\nHole Cards:\n");
	for (int32_t i = 0; ((i < no_of_hole_cards) && (i < all_number_cards_drawn[this_playerID])); i++) {
		printf("%s-->%s \t", suit[all_player_card_values[this_playerID][i] / 13],
		       face[all_player_card_values[this_playerID][i] % 13]);
	}

	flag = 1;
	for (int32_t i = no_of_hole_cards; ((i < hand_size) && (i < all_number_cards_drawn[this_playerID])); i++) {
		if (flag) {
			printf("\nCommunity Cards:\n");
			flag = 0;
		}
		printf("%s-->%s \t", suit[all_player_card_values[this_playerID][i] / 13],
		       face[all_player_card_values[this_playerID][i] % 13]);
	}

	printf("\n******************** Betting done so far ********************");
	printf("\nsmall_blind:%d, big_blind:%d", small_blind_amount, big_blind_amount);
	printf("\npot size:%d", jint(argjson, "pot"));
	actions = cJSON_GetObjectItem(argjson, "actions");
	int count = 0;
	flag = 1;
	for (int i = 0; ((i <= jint(argjson, "round")) && (flag)); i++) {
		printf("\nRound:%d", i);
		for (int j = 0; ((j < BET_player[this_playerID]->maxplayers) && (flag)); j++) {
			if (jinti(actions, ((i * BET_player[this_playerID]->maxplayers) + j)) > 0)
				printf("\nplayed id:%d, action: %s", j,
				       action_str[jinti(actions, ((i * BET_player[this_playerID]->maxplayers) + j))]);
			count++;
			if (count == cJSON_GetArraySize(actions))
				flag = 0;
		}
		printf("\n");
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
				continue;
			}
			dealers_ip_info = cJSON_CreateArray();
			dealers_ip_info = cJSON_GetObjectItem(cashier_response_info, "dealer_ips");
			for (int32_t j = 0; j < cJSON_GetArraySize(dealers_ip_info); j++) {
				int flag = 1;
				for (int32_t k = 0; k < cJSON_GetArraySize(all_dealers_info); k++) {
					if (strcmp(cJSON_Print(cJSON_GetArrayItem(all_dealers_info, k)),
						   cJSON_Print(cJSON_GetArrayItem(dealers_ip_info, j))) == 0) {
						flag = 0;
						break;
					}
				}
				if (flag)
					cJSON_AddItemToArray(all_dealers_info, cJSON_GetArrayItem(dealers_ip_info, j));
			}
		}
	}
	return all_dealers_info;
}
