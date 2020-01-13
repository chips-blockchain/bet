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

#define LWS_PLUGIN_STATIC

struct lws *wsi_global_client = NULL;

int ws_connection_status = 0;

struct lws *wsi_global_bvv = NULL;

int32_t player_card_matrix[hand_size];
int32_t player_card_values[hand_size];
int32_t number_cards_drawn = 0;

int32_t sharesflag[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t data_exists = 0;
char player_gui_data[1024];

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

void player_lws_write(cJSON *data)
{
	if (ws_connection_status == 1) {
		if (data_exists == 1) {
			printf("%s::%d::There is more data\n", __FUNCTION__, __LINE__);
			while (data_exists == 1)
				sleep(1);
		}
		memset(player_gui_data, 0, sizeof(player_gui_data));
		strncpy(player_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
		data_exists = 1;
		lws_callback_on_writable(wsi_global_client);
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
	int32_t bytes, retval = 0;
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
	rendered = cJSON_Print(bvv_init_info);
	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

	return retval;
}

static int32_t bet_bvv_join_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *channel_info = NULL, *addresses = NULL, *address = NULL, *bvv_response_info = NULL;
	int argc, bytes, retval = 0;
	char **argv = NULL, *uri = NULL, *rendered = NULL;

	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
		argv[i] = (char *)malloc(100 * sizeof(char));

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "getinfo");

	channel_info = cJSON_CreateObject();
	make_command(argc, argv, &channel_info);

	cJSON_Print(channel_info);
	if (jint(channel_info, "code") != 0) {
		retval = -1;
		printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(channel_info, "message"));
		goto end;
	}

	uri = (char *)malloc(sizeof(char) * 100);
	if (!uri) {
		retval = -1;
		printf("%s::%d::malloc failed\n", __FUNCTION__, __LINE__);
		goto end;
	}
	strcpy(uri, jstr(channel_info, "id"));
	strcat(uri, "@");
	addresses = cJSON_GetObjectItem(channel_info, "address");
	address = cJSON_GetArrayItem(addresses, 0);
	strcat(uri, jstr(address, "address"));

	bvv_response_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_response_info, "method", "bvv_join");
	cJSON_AddStringToObject(bvv_response_info, "uri", uri);
	rendered = cJSON_Print(bvv_response_info);

	bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

end:
	if (uri)
		free(uri);
	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}
int32_t bet_bvv_connect(char *uri)
{
	char **argv = NULL;
	int argc = 3;
	cJSON *connect_info = NULL;

	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
		argv[i] = (char *)malloc(100 * sizeof(char));

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "connect");
	strcpy(argv[2], uri);
	connect_info = cJSON_CreateObject();
	make_command(argc, argv, &connect_info);

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return 1;
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
	int retval = 0, channel_state, bytes;
	cJSON *uri_info = NULL, *fund_channel_info = NULL, *bvv_ready = NULL;
	char uri[100], channel_id[100], *rendered = NULL;

	uri_info = cJSON_GetObjectItem(argjson, "uri_info");
	for (int i = 0; i < cJSON_GetArraySize(uri_info); i++) {
		strcpy(uri, jstri(uri_info, i));
		strcpy(channel_id, strtok(uri, "@"));
		channel_state = ln_get_channel_status(channel_id);
		if ((channel_state != 2) && (channel_state != 3)) {
			bet_bvv_connect(jstri(uri_info, i));
			fund_channel_info = cJSON_CreateObject();
			fund_channel_info = bet_player_fundchannel(channel_id);

			if (jint(fund_channel_info, "code") == -1) {
				retval = -1;
				printf("%s::%d::%s", __FUNCTION__, __LINE__, cJSON_Print(fund_channel_info));
				goto end;
			}
		}
	}
	for (int i = 0; i < cJSON_GetArraySize(uri_info); i++) {
		strcpy(uri, jstri(uri_info, i));
		strcpy(channel_id, strtok(uri, "@"));
		while ((channel_state = ln_get_channel_status(channel_id)) != 3) {
			if (channel_state == 2) {
				printf("CHANNELD AWAITING LOCKIN\r");
				fflush(stdout);
				sleep(2);
			} else {
				retval = -1;
				printf("\n%s:%d: BVV is failed to establish the channel with "
				       "Player: %d",
				       __FUNCTION__, __LINE__, i);
				break;
			}
		}

		printf("BVV  --> Player %d channel ready\n", i);
	}

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "bvv_ready");

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

int32_t bet_bvv_frontend(struct lws *wsi, cJSON *argjson)
{
	char *method = NULL;
	int32_t retval = 0;
	struct privatebet_info *bet = NULL;
	struct privatebet_vars *vars = NULL;

	if ((method = jstr(argjson, "method")) != 0) {
		if (strcmp(method, "TableInfo") == 0) {
			bet_table_info(argjson, bet_bvv, vars);
		} else if (strcmp(method, "init_d") == 0) {
			bet_bvv_init(argjson, bet_bvv, vars);
		} else if (strcmp(method, "bvv_join") == 0) {
			bet_bvv_join_init(argjson, bet_bvv, vars);
		} else if (strcmp(method, "check_bvv_ready") == 0) {
			bet_check_bvv_ready(argjson, bet_bvv, vars);
		} else if (strcmp(method, "dealer") == 0) {
			retval = bet_player_dealer_info(argjson, bet_bvv, vars);
		} else if (strcmp(method, "reset") == 0) {
			bet_bvv_reset(bet, vars);
		}
	}
	return retval;
}

void bet_bvv_backend_loop(void *_ptr)
{
	int32_t recvlen;
	cJSON *argjson = NULL;
	void *ptr = NULL;
	struct privatebet_info *bet = _ptr;
	struct privatebet_vars *VARS = NULL;
	cJSON *bvv_join_info = NULL;

	VARS = calloc(1, sizeof(*VARS));
	bet_permutation(bvv_info.permis, bet->range);
	for (int i = 0; i < bet->range; i++) {
		permis_b[i] = bvv_info.permis[i];
	}

	bvv_join_info = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_join_info, "method", "bvv_join");
	if (bet_bvv_backend(bvv_join_info, bet, VARS) != 0) {
		printf("\n%s:%d:BVV joining the table failed", __FUNCTION__, __LINE__);
	}
	while (bet->pushsock >= 0 && bet->subsock >= 0) {
		ptr = 0;
		if ((recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0)) > 0) {
			char *tmp = clonestr(ptr);
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				if (bet_bvv_backend(argjson, bet, bvv_vars) < 0) // usually just relay to players
				{
					printf("%s::%d::Failed to send data\n", __FUNCTION__, __LINE__);
				}
				free_json(argjson);
			}
			if (tmp)
				free(tmp);
			if (ptr)
				nn_freemsg(ptr);
		}
	}
}

static int32_t bet_live_response(struct privatebet_info *bet, char *node_type, int32_t playerid)
{
	cJSON *live_info = NULL;
	int retval = 1;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	cJSON_AddStringToObject(live_info, "node_type", node_type);

	if (strcmp(node_type, "player") == 0)
		cJSON_AddNumberToObject(live_info, "playerid", playerid);

	int bytes = nn_send(bet->pushsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_bvv_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *method;
	int32_t retval = 0;

	if ((method = jstr(argjson, "method")) != 0) {
		if (strcmp(method, "TableInfo") == 0) {
			bet_table_info(argjson, bet, vars);

		} else if (strcmp(method, "init_d") == 0) {
			retval = bet_bvv_init(argjson, bet, vars);
		} else if (strcmp(method, "bvv_join") == 0) {
			printf("%s::%d::bvv_join\n", __FUNCTION__, __LINE__);
			retval = bet_bvv_join_init(argjson, bet, vars);
		} else if (strcmp(method, "check_bvv_ready") == 0) {
			retval = bet_check_bvv_ready(argjson, bet, vars);
		} else if (strcmp(method, "reset") == 0) {
			bet_bvv_reset(bet, vars);
			retval = bet_bvv_join_init(argjson, bet_bvv, vars);
		} else if (strcmp(method, "seats") == 0) {
			retval = bet_bvv_join_init(argjson, bet, vars);
		} else if (strcmp(method, "live") == 0) {
			retval = bet_live_response(bet, "bvv", -1);
		} else if (strcmp(method, "status_info") == 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
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

void display_cards(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
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
		printf("decrypt error \n");
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

	if ((ptr = bet_decrypt(decipher, sizeof(decipher), player_info.bvvpubkey, player_info.player_key.priv,
			       temp.bytes, &recvlen)) == 0) {
		retval = -1;
		printf("decrypt error ");
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
	int argc, retval = 1, channel_state, balance, bytes;
	char **argv = NULL, channel_id[100];
	cJSON *init_card_info = NULL, *hole_card_info = NULL, *init_info = NULL, *stack_info = NULL;

	if (0 == bits256_cmp(player_info.player_key.prod, jbits256(argjson, "pubkey"))) {
		bet_player->myplayerid = jint(argjson, "peerid");
		bet->myplayerid = jint(argjson, "peerid");

		strcpy(uri, jstr(argjson, "uri"));
		strcpy(channel_id, strtok(jstr(argjson, "uri"), "@"));
		channel_state = ln_get_channel_status(channel_id);
		if ((channel_state != CHANNELD_AWAITING_LOCKIN) &&
		    (channel_state != CHANNELD_NORMAL)) 
		{
			retval = ln_establish_channel(uri);
			if(retval == 1)
				printf("Channel Established\n");
			else
				printf("Channel Didn't Established\n");			
		} else {
			strcpy(uri, jstr(argjson, "uri"));
			ln_check_peer_and_connect(uri);
		}
		init_card_info = cJSON_CreateObject();
		cJSON_AddNumberToObject(init_card_info, "dealer", jint(argjson, "dealer"));
		balance = bet_check_player_stack(jstr(argjson, "uri"));

		if (vars->player_funds == 0) // refill the player stack only if it
			// becomes zero and funds are available
			vars->player_funds = balance;

		// Here if the balance is not table_stack it should wait for the refill
		cJSON_AddNumberToObject(init_card_info, "balance", vars->player_funds);

		hole_card_info = cJSON_CreateArray();
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		cJSON_AddItemToObject(init_card_info, "holecards", hole_card_info);

		init_info = cJSON_CreateObject();
		cJSON_AddStringToObject(init_info, "method", "deal");
		cJSON_AddItemToObject(init_info, "deal", init_card_info);

		player_lws_write(init_info);

		stack_info = cJSON_CreateObject();
		cJSON_AddStringToObject(stack_info, "method", "stack");
		cJSON_AddNumberToObject(stack_info, "playerid", bet->myplayerid);
		cJSON_AddNumberToObject(stack_info, "stack_value", vars->player_funds);

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

int32_t bet_client_join(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
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
		cJSON_AddNumberToObject(joininfo, "gui_playerID", jint(argjson, "gui_playerID"));

		rendered = cJSON_Print(joininfo);
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

void bet_table_info(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	printf("\nTable Info:%s", cJSON_Print(argjson));
}

int32_t bet_player_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1;
	cJSON *reset_info = NULL;

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

	reset_info = cJSON_CreateObject();
	cJSON_AddStringToObject(reset_info, "method", "reset");
	player_lws_write(reset_info);
	return retval;
}

int32_t bet_player_frontend(struct lws *wsi, cJSON *argjson)
{
	int32_t retval = 1;
	char *method = NULL;
	struct privatebet_vars *vars = NULL;
	struct privatebet_info *bet = NULL;
	if ((method = jstr(argjson, "method")) != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
		if (strcmp(method, "player_join") == 0) {
			if (player_joined == 0)
				retval = bet_client_join(argjson, bet_player, vars);
			else
				printf("%s::%d::Player is already joined\n", __FUNCTION__, __LINE__);
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_round_betting(argjson, bet_player, player_vars);
		} else if (strcmp(method, "reset") == 0) {
			retval = bet_player_reset(bet, vars);
		} else {
			printf("%s::%d::Method::%s is not handled\n", __FUNCTION__, __LINE__, method);
		}
	}
	return retval;
}

int lws_callback_http_player(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;
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
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
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

static int interrupted1;

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

void bet_player_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	// signal(SIGINT,player_sigint_handler);
	lws_set_log_level(logs, NULL);

	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
	dcv_info.port = 9000;
	dcv_info.mounts = &lws_http_mount_player;
	dcv_info.protocols = player_http_protocol;
	dcv_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	dcv_context = lws_create_context(&dcv_info);
	if (!dcv_context) {
		printf("lws init failed\n");
	}
	while (n >= 0 && !interrupted1) {
		n = lws_service(dcv_context, 1000);
	}
	if (dcv_context)
		lws_context_destroy(dcv_context);
}

int lws_callback_http_bvv(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf_bvv + lws_buf_length_bvv, in, len);
		lws_buf_length_bvv += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf_bvv);
		if (bet_bvv_frontend(wsi, argjson) != 0) {
			printf("\n%s:%d:Failed to process the host command", __FUNCTION__, __LINE__);
		}
		memset(lws_buf_bvv, 0x00, sizeof(lws_buf_bvv));
		lws_buf_length_bvv = 0;
		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_bvv = wsi;
		break;
	}
	return 0;
}

static struct lws_protocols protocols_bvv[] = {
	{ "http", lws_callback_http_bvv, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted_bvv;

static const struct lws_http_mount mount_bvv = {
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

void bet_bvv_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:7681\n");
	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
	dcv_info.port = 9000;
	dcv_info.mounts = &mount_bvv;
	dcv_info.protocols = protocols_bvv;
	dcv_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	dcv_context = lws_create_context(&dcv_info);
	if (!dcv_context) {
		printf("lws init failed\n");
	}
	while (n >= 0 && !interrupted_bvv) {
		n = lws_service(dcv_context, 1000);
	}
	lws_context_destroy(dcv_context);
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

static void bet_push_join_info(int32_t playerid)
{
	cJSON *join_info = NULL;

	join_info = cJSON_CreateObject();
	cJSON_AddStringToObject(join_info, "method", "join_info");
	cJSON_AddNumberToObject(join_info, "playerid", playerid);
	player_lws_write(join_info);
}

int32_t bet_player_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, bytes;
	char *method = NULL;
	char hexstr[65], *rendered = NULL;

	if ((method = jstr(argjson, "method")) != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, method);

		if (strcmp(method, "join") == 0) {
			retval = bet_client_join(argjson, bet, vars);

		} else if (strcmp(method, "join_res") == 0) {
			bet_push_join_info(jint(argjson, "peerid"));
			retval = bet_client_join_res(argjson, bet, vars);
		} else if (strcmp(method, "TableInfo") == 0) {
			bet_table_info(argjson, bet, vars);
		} else if (strcmp(method, "init") == 0) {
			if (jint(argjson, "peerid") == bet->myplayerid) {
				bet_player_blinds_info();
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
		} else if (strcmp(method, "live") == 0) {
			retval = bet_live_response(bet, "player", bet->myplayerid);
		} else if (strcmp(method, "status_info") == 0) {
			player_lws_write(argjson);
		} else if (strcmp(method, "stack_info_resp") == 0) {
			double funds_needed = jdouble(argjson, "table_stack_in_chips");
			if (chips_get_balance() < (funds_needed + chips_tx_fee)) {
				printf("%s::%d::Insufficient funds\n", __FUNCTION__, __LINE__);
				retval = -1;
			} else {
				cJSON *tx_info = cJSON_CreateObject();
				char *data = jstr(argjson,"rand_str");
				cJSON *txid = chips_transfer_funds_with_data(funds_needed, legacy_2_of_4_msig_Addr,data);
				cJSON_AddStringToObject(tx_info, "method", "tx");
				cJSON_AddItemToObject(tx_info, "tx_info", txid);
				printf("tx_info::%s\n",cJSON_Print(tx_info));
				
				while (chips_get_block_hash_from_txid(cJSON_Print(txid)) == NULL) {
					sleep(2);
				}
				cJSON_AddNumberToObject(tx_info, "block_height",
							chips_get_block_height_from_block_hash(
								chips_get_block_hash_from_txid(cJSON_Print(txid))));
				printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(tx_info));
				bytes = nn_send(bet->pushsock, cJSON_Print(tx_info), strlen(cJSON_Print(tx_info)), 0);
				if (bytes < 0)
					retval = -1;
			}
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
	cJSON *funds_info = NULL;
	int32_t bytes;
	
	funds_info = cJSON_CreateObject();
	cJSON_AddStringToObject(funds_info, "method", "stack_info_req");
	bytes = nn_send(bet->pushsock, cJSON_Print(funds_info), strlen(cJSON_Print(funds_info)), 0);
	if (bytes < 0) {
		printf("%s::%d::Failed to send data\n", __FUNCTION__, __LINE__);
		flag = 0;
	}
	while (flag) {
		if (bet->subsock >= 0 && bet->pushsock >= 0) {
			ptr = 0;
			char *tmp = NULL;
			recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0);
			if (recvlen > 0)
				tmp = clonestr(ptr);
			if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
				if (bet_player_backend(msgjson, bet, player_vars) < 0) {
					printf("\nFAILURE\n");
					// do something here, possibly this could be because unknown
					// commnad or because of encountering a special case which
					// state machine fails to handle
					break;
				}
				if (tmp)
					free(tmp);
				if (ptr)
					nn_freemsg(ptr);
			}
		}
	}
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
