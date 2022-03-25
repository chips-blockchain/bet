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
#include "host.h"
#include "../log/macrologger.h"
#include "bet.h"
#include "cards777.h"
#include "client.h"
#include "commands.h"
#include "common.h"
#include "network.h"
#include "oracle.h"
#include "payment.h"
#include "poker.h"
#include "states.h"
#include "table.h"
#include "cashier.h"
#include "storage.h"
#include "misc.h"
#include "heartbeat.h"
#include "err.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>

#define LWS_PLUGIN_STATIC

struct lws *wsi_global_host = NULL;

int32_t players_joined = 0;
int32_t turn = 0, no_of_cards = 0, no_of_rounds = 0, no_of_bets = 0;
int32_t card_matrix[CARDS777_MAXPLAYERS][hand_size];
int32_t card_values[CARDS777_MAXPLAYERS][hand_size];
struct deck_dcv_info dcv_info;
int32_t player_ready[CARDS777_MAXPLAYERS];
int32_t hole_cards_drawn = 0, community_cards_drawn = 0, flop_cards_drawn = 0, turn_card_drawn = 0,
	river_card_drawn = 0;
int32_t bet_amount[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS];
int32_t eval_game_p[CARDS777_MAXPLAYERS], eval_game_c[CARDS777_MAXPLAYERS];

char player_chips_address[CARDS777_MAXPLAYERS][64];

char tx_rand_str[CARDS777_MAXPLAYERS][65];
int no_of_rand_str = 0;

int32_t invoiceID;

char *suit[NSUITS] = { "clubs", "diamonds", "hearts", "spades" };
char *face[NFACES] = { "two",  "three", "four", "five",  "six",  "seven", "eight",
		       "nine", "ten",   "jack", "queen", "king", "ace" };

struct privatebet_info *bet_dcv = NULL;
struct privatebet_vars *dcv_vars = NULL;
struct dcv_bvv_sock_info *bet_dcv_bvv = NULL;

static int dealerPosition;
int32_t no_of_signers, max_no_of_signers = 2, is_signed[CARDS777_MAXPLAYERS];

char lws_buf[65536];
int32_t lws_buf_length = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int32_t no_of_txs = 0;
char tx_ids[CARDS777_MAXPLAYERS][100];

int32_t threshold_time = 30; /* Time to send the beacon*/

int32_t dcv_data_exists = 0;
char dcv_gui_data[8196];
int ws_dcv_connection_status = 0;

double dcv_commission_percentage = 0.75;
char dcv_hosted_gui_url[128];

int32_t heartbeat_on = 0;

char table_id[65];
int32_t dcv_state = 0;

void bet_set_table_id()
{
	bits256 randval;
	memset(table_id, 0x00, sizeof(table_id));
	OS_randombytes(randval.bytes, sizeof(randval));
	bits256_str(table_id, randval);
	dlg_info("table_id::%s", table_id);
}

void bet_dcv_lws_write(cJSON *data)
{
	if (ws_dcv_connection_status == 1) {
		if (dcv_data_exists == 1) {
			dlg_info("There is more data\n");
			while (dcv_data_exists == 1)
				sleep(1);
		}
		memset(dcv_gui_data, 0, sizeof(dcv_gui_data));
		strncpy(dcv_gui_data, cJSON_Print(data), strlen(cJSON_Print(data)));
		dcv_data_exists = 1;
		lws_callback_on_writable(wsi_global_host);
		dlg_info("Data pushed to GUI\n");
	}
}

void bet_chat(struct lws *wsi, cJSON *argjson)
{
	cJSON *chat_info = NULL;

	chat_info = cJSON_CreateObject();
	cJSON_AddStringToObject(chat_info, "chat", jstr(argjson, "value"));
	lws_write(wsi, (unsigned char *)cJSON_Print(chat_info), strlen(cJSON_Print(chat_info)), 0);
}

void initialize_seat(cJSON *seat_info, char *name, int32_t seat, int32_t chips, int32_t empty, int32_t playing)
{
	cJSON_AddStringToObject(seat_info, "name", name);
	cJSON_AddNumberToObject(seat_info, "seat", seat);
	cJSON_AddNumberToObject(seat_info, "chips", chips);
	cJSON_AddNumberToObject(seat_info, "empty", empty);
	cJSON_AddNumberToObject(seat_info, "playing", playing);
}

int32_t bet_seats(struct lws *wsi, cJSON *argjson)
{
	cJSON *table_info = NULL, *seats_info = NULL;
	char *rendered = NULL;
	int32_t retval = 0, bytes;
	cJSON *seat[max_players];

	for (int i = 0; i < max_players; i++) {
		seat[i] = cJSON_CreateObject();
	}

	initialize_seat(seat[0], "player1", 0, 0, 0, 1);
	initialize_seat(seat[1], "player2", 1, 0, 0, 1);

	seats_info = cJSON_CreateArray();
	for (int i = 0; i < max_players; i++) {
		cJSON_AddItemToArray(seats_info, seat[i]);
	}

	table_info = cJSON_CreateObject();
	cJSON_AddStringToObject(table_info, "method", "seats");
	cJSON_AddItemToObject(table_info, "seats", seats_info);

	rendered = cJSON_Print(table_info);
	lws_write(wsi, (unsigned char *)rendered, strlen(rendered), 0);

	bytes = nn_send(bet_dcv->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_game(struct lws *wsi, cJSON *argjson)
{
	char buf[100] = { 0 };
	cJSON *game_info = NULL, *game_details = NULL, *pot_info = NULL;
	char *rendered = NULL;

	game_details = cJSON_CreateObject();
	cJSON_AddNumberToObject(game_details, "seats", max_players);

	pot_info = cJSON_CreateArray();
	cJSON_AddItemToArray(pot_info, cJSON_CreateNumber(0));

	cJSON_AddItemToObject(game_details, "pot", pot_info);
	sprintf(buf, "Texas Holdem Poker:%d/%d", small_blind_amount, big_blind_amount);

	cJSON_AddStringToObject(game_details, "gametype", buf);

	game_info = cJSON_CreateObject();
	cJSON_AddStringToObject(game_info, "method", "game");
	cJSON_AddItemToObject(game_info, "game", game_details);
	rendered = cJSON_Print(game_info);
	lws_write(wsi, (unsigned char *)rendered, strlen(rendered), 0);
	return 0;
}

int32_t bet_dcv_frontend(struct lws *wsi, cJSON *argjson)
{
	int retval = 1;
	char *rendered = NULL, *method = NULL;
	int32_t bytes = 0;

	method = jstr(argjson, "method");
	dlg_info("method::%s\n", method);
	if (strcmp(method, "game") == 0) {
		retval = bet_game(wsi, argjson);
	} else if (strcmp(method, "seats") == 0) {
		retval = bet_seats(wsi, argjson);

	} else if (strcmp(method, "chat") == 0) {
		bet_chat(wsi, argjson);
	} else if (strcmp(method, "reset") == 0) {
		bet_reset_all_dcv_params(bet_dcv, dcv_vars);
		rendered = cJSON_Print(argjson);
		bytes = nn_send(bet_dcv->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			retval = -1;
			dlg_error("nn_send failed\n");
		}
	} else if (strcmp(method, "get_bal_info") == 0) {
		cJSON *bal_info = cJSON_CreateObject();
		bal_info = bet_get_chips_ln_bal_info();
		lws_write(wsi, (unsigned char *)cJSON_Print(bal_info), strlen(cJSON_Print(bal_info)), 0);
	} else if (strcmp(method, "get_addr_info") == 0) {
		cJSON *addr_info = cJSON_CreateObject();
		addr_info = bet_get_chips_ln_addr_info();
		lws_write(wsi, (unsigned char *)cJSON_Print(addr_info), strlen(cJSON_Print(addr_info)), 0);
	} else {
		dlg_warn("Unknown Method::%s\n", method);
	}

	return retval;
}

int lws_callback_http_dummy(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	cJSON *argjson = NULL;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(lws_buf + lws_buf_length, in, len);
		lws_buf_length += len;
		if (!lws_is_final_fragment(wsi))
			break;
		argjson = cJSON_Parse(lws_buf);
		if (bet_dcv_frontend(wsi, argjson) != 0) {
			dlg_warn("Failed to process the host command");
		}
		memset(lws_buf, 0x00, sizeof(lws_buf));
		lws_buf_length = 0;
		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_host = wsi;
		dlg_info("LWS_CALLBACK_ESTABLISHED\n");
		ws_dcv_connection_status = 1;
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:

		if (dcv_data_exists) {
			if (strlen(dcv_gui_data) != 0) {
				lws_write(wsi, (unsigned char *)dcv_gui_data, strlen(dcv_gui_data), 0);
				dcv_data_exists = 0;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

static struct lws_protocols protocols[] = {
	{ "http", lws_callback_http_dummy, 0, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted;

static const struct lws_http_mount mount = {
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

void sigint_handler(int sig)
{
	interrupted = 1;
}

void bet_push_dcv_to_gui(cJSON *argjson)
{
	if (argjson) {
		bet_dcv_lws_write(argjson);
	}
}

int32_t bet_dcv_deck_init_info(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	char str[65] = { 0 };
	cJSON *deck_init_info = NULL, *cjson_cardprods = NULL, *cjson_dcv_blind_cards = NULL, *cjsong_hash = NULL,
	      *cjson_peer_pub_keys = NULL;

	deck_init_info = cJSON_CreateObject();
	cJSON_AddStringToObject(deck_init_info, "method", "init_d");
	jaddbits256(deck_init_info, "deckid", dcv_info.deckid);
	cJSON_AddItemToObject(deck_init_info, "cardprods", cjson_cardprods = cJSON_CreateArray());
	for (int i = 0; i < dcv_info.numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			cJSON_AddItemToArray(cjson_cardprods,
					     cJSON_CreateString(bits256_str(str, dcv_info.cardprods[i][j])));
		}
	}

	cJSON_AddItemToObject(deck_init_info, "dcvblindcards", cjson_dcv_blind_cards = cJSON_CreateArray());
	for (int i = 0; i < dcv_info.numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			cJSON_AddItemToArray(cjson_dcv_blind_cards,
					     cJSON_CreateString(bits256_str(str, dcv_info.dcvblindcards[i][j])));
		}
	}

	cJSON_AddItemToObject(deck_init_info, "g_hash", cjsong_hash = cJSON_CreateArray());
	for (int i = 0; i < dcv_info.numplayers; i++) {
		for (int j = 0; j < bet->range; j++) {
			cJSON_AddItemToArray(cjsong_hash, cJSON_CreateString(bits256_str(str, g_hash[i][j])));
		}
	}

	cJSON_AddItemToObject(deck_init_info, "peerpubkeys", cjson_peer_pub_keys = cJSON_CreateArray());
	for (int i = 0; i < dcv_info.numplayers; i++) {
		cJSON_AddItemToArray(cjson_peer_pub_keys,
				     cJSON_CreateString(bits256_str(str, dcv_info.peerpubkeys[i])));
	}

	//sending init_d to BVV
	retval = (nn_send(bet_dcv_bvv->pubsock, cJSON_Print(deck_init_info), strlen(cJSON_Print(deck_init_info)), 0) <
		  0) ?
			 ERR_NNG_SEND :
			 OK;
	//sending init_d to players
	retval = (nn_send(bet->pubsock, cJSON_Print(deck_init_info), strlen(cJSON_Print(deck_init_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;

	return retval;
}

int32_t bet_dcv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t peerid, retval = OK;
	bits256 card_pubvalues[CARDS777_MAXCARDS];
	cJSON *card_info = NULL;

	peerid = jint(argjson, "peerid");
	card_info = cJSON_GetObjectItem(argjson, "cardinfo");
	for (int i = 0; i < cJSON_GetArraySize(card_info); i++) {
		card_pubvalues[i] = jbits256i(card_info, i);
	}

	retval = sg777_deckgen_vendor(peerid, dcv_info.cardprods[peerid], dcv_info.dcvblindcards[peerid], bet->range,
				      card_pubvalues, dcv_info.deckid);
	dcv_info.numplayers = dcv_info.numplayers + 1;

	if ((peerid + 1) < bet->maxplayers) {
		retval = bet_dcv_start(bet, peerid + 1);
	}

	return retval;
}

static int32_t bet_dcv_bvv_join(cJSON *argjson, struct dcv_bvv_sock_info *bet_dcv_bvv, struct privatebet_vars *vars)
{
	int retval = OK;
	cJSON *config_info = NULL;

	config_info = cJSON_CreateObject();
	cJSON_AddStringToObject(config_info, "method", "config_data");
	cJSON_AddNumberToObject(config_info, "max_players", max_players);
	cJSON_AddNumberToObject(config_info, "bb_in_chips", BB_in_chips);
	cJSON_AddNumberToObject(config_info, "table_min_stake", table_min_stake);
	cJSON_AddNumberToObject(config_info, "table_max_stake", table_max_stake);
	cJSON_AddNumberToObject(config_info, "chips_tx_fee", chips_tx_fee);

	retval = (nn_send(bet_dcv_bvv->pubsock, cJSON_Print(config_info), strlen(cJSON_Print(config_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

int32_t bet_dcv_start(struct privatebet_info *bet, int32_t peerid)
{
	int32_t retval = OK;
	cJSON *init = NULL;

	init = cJSON_CreateObject();
	cJSON_AddStringToObject(init, "method", "init");
	cJSON_AddNumberToObject(init, "peerid", peerid);

	retval = (nn_send(bet->pubsock, cJSON_Print(init), strlen(cJSON_Print(init)), 0) < 0) ? ERR_NNG_SEND : OK;
	return retval;
}

cJSON *bet_get_seats_json(int32_t max_players)
{
	cJSON *seats_info = NULL;
	cJSON *seat[max_players];

	seats_info = cJSON_CreateArray();
	for (int i = 0; i < max_players; i++) {
		seat[i] = cJSON_CreateObject();
		initialize_seat(seat[i], player_seats_info[i].seat_name, player_seats_info[i].seat,
				player_seats_info[i].chips, player_seats_info[i].empty, player_seats_info[i].playing);
		cJSON_AddItemToArray(seats_info, seat[i]);
	}
	return seats_info;
}

int32_t bet_player_join_req(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *player_info = NULL;
	char *uri = NULL, *type = NULL;

	bet->numplayers = ++players_joined;
	dcv_info.peerpubkeys[jint(argjson, "gui_playerID")] = jbits256(argjson, "pubkey");

	if (bet_ln_config == BET_WITH_LN) {
		strcpy((char *)dcv_info.uri[jint(argjson, "gui_playerID")], jstr(argjson, "uri"));
		uri = (char *)malloc(ln_uri_length * sizeof(char));
		type = ln_get_uri(&uri);
		dlg_info("%s::\n%s", type, uri);
	}

	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, "method", "join_res");

	cJSON_AddNumberToObject(player_info, "playerid", jint(argjson, "gui_playerID"));
	jaddbits256(player_info, "pubkey", jbits256(argjson, "pubkey"));

	if (bet_ln_config == BET_WITH_LN) {
		cJSON_AddStringToObject(player_info, "uri", uri);
		cJSON_AddStringToObject(player_info, "type", type);
	}

	cJSON_AddNumberToObject(player_info, "dealer", dealerPosition);
	cJSON_AddNumberToObject(player_info, "pos_status", pos_on_table_empty);
	cJSON_AddStringToObject(player_info, "req_identifier", jstr(argjson, "req_identifier"));

	player_seats_info[jint(argjson, "gui_playerID")].empty = 0;
	player_seats_info[jint(argjson, "gui_playerID")].chips =
		vars->funds[vars->req_id_to_player_id_mapping[jint(argjson, "gui_playerID")]];
	if (jstr(argjson, "player_name") && (strlen(jstr(argjson, "player_name")) != 0))
		strcpy(player_seats_info[jint(argjson, "gui_playerID")].seat_name, jstr(argjson, "player_name"));
	cJSON *seats_info = NULL;

	dlg_info("bet->maxplayers::%d\n", bet->maxplayers);
	seats_info = bet_get_seats_json(bet->maxplayers);
	cJSON_AddItemToObject(player_info, "seats", seats_info);

	retval = (nn_send(bet->pubsock, cJSON_Print(player_info), strlen(cJSON_Print(player_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;

	if (uri)
		free(uri);
	return retval;
}

static int32_t bet_send_turn_info(struct privatebet_info *bet, int32_t playerid, int32_t cardid, int32_t card_type)
{
	cJSON *turn_info = NULL;
	int32_t retval = OK;

	turn_info = cJSON_CreateObject();
	cJSON_AddStringToObject(turn_info, "method", "turn");
	cJSON_AddNumberToObject(turn_info, "playerid", playerid);
	cJSON_AddNumberToObject(turn_info, "cardid", cardid);
	cJSON_AddNumberToObject(turn_info, "card_type", card_type);

	dlg_info("%s", cJSON_Print(turn_info));
	retval = (nn_send(bet->pubsock, cJSON_Print(turn_info), strlen(cJSON_Print(turn_info)), 0) < 0) ? ERR_NNG_SEND :
													  OK;
	return retval;
}

int32_t bet_dcv_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;

	if (hole_cards_drawn == 0) {
		for (int i = 0; i < no_of_hole_cards; i++) {
			for (int j = 0; j < bet->maxplayers; j++) {
				if (card_matrix[j][i] == 0) {
					retval = bet_send_turn_info(bet, j, (i * bet->maxplayers) + j, hole_card);
					goto end;
				}
			}
		}
	} else if (flop_cards_drawn == 0) {
		for (int i = no_of_hole_cards; i < no_of_hole_cards + no_of_flop_cards; i++) {
			for (int j = 0; j < bet->maxplayers; j++) {
				if (card_matrix[j][i] == 0) {
					if ((i - (no_of_hole_cards)) == 0) {
						retval = bet_send_turn_info(bet, j,
									    (no_of_hole_cards * bet->maxplayers) +
										    (i - no_of_hole_cards) + 1,
									    flop_card_1);
					} else if ((i - (no_of_hole_cards)) == 1) {
						retval = bet_send_turn_info(bet, j,
									    (no_of_hole_cards * bet->maxplayers) +
										    (i - no_of_hole_cards) + 1,
									    flop_card_2);
					} else if ((i - (no_of_hole_cards)) == 2) {
						retval = bet_send_turn_info(bet, j,
									    (no_of_hole_cards * bet->maxplayers) +
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
			for (int j = 0; j < bet->maxplayers; j++) {
				if (card_matrix[j][i] == 0) {
					retval = bet_send_turn_info(bet, j,
								    (no_of_hole_cards * bet->maxplayers) +
									    (i - no_of_hole_cards) + 2,
								    turn_card);
					goto end;
				}
			}
		}
	} else if (river_card_drawn == 0) {
		for (int i = no_of_hole_cards + no_of_flop_cards + no_of_turn_card;
		     i < no_of_hole_cards + no_of_flop_cards + no_of_turn_card + no_of_river_card; i++) {
			for (int j = 0; j < bet->maxplayers; j++) {
				if (card_matrix[j][i] == 0) {
					retval = bet_send_turn_info(bet, j,
								    (no_of_hole_cards * bet->maxplayers) +
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

int32_t bet_relay(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	retval = (nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0) < 0) ? ERR_NNG_SEND : OK;
	return retval;
}

static int32_t bet_check_bvv_ready(struct privatebet_info *bet)
{
	int32_t retval = OK;
	cJSON *bvv_ready = NULL, *uri_info = NULL;

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "check_bvv_ready");
	cJSON_AddItemToObject(bvv_ready, "uri_info", uri_info = cJSON_CreateArray());
	for (int i = 0; i < bet->maxplayers; i++) {
		jaddistr(uri_info, (char *)dcv_info.uri[i]);
	}
	dlg_info("%s\n", cJSON_Print(bvv_ready));
	retval = (nn_send(bet_dcv_bvv->pubsock, cJSON_Print(bvv_ready), strlen(cJSON_Print(bvv_ready)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

static int32_t bet_create_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t argc, retval = OK;
	char **argv = NULL, hexstr[65] = { 0 };
	cJSON *invoice_info = NULL, *invoice = NULL;

	invoiceID++;
	argc = 5;
	bet_alloc_args(argc, &argv);
	dcv_info.betamount += jint(argjson, "betAmount");

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "invoice");
	sprintf(argv[2], "%ld", (long int)jint(argjson, "betAmount")); //sg777 mchips_msatoshichips
	sprintf(argv[3], "%s_%d_%d_%d_%d", bits256_str(hexstr, dcv_info.deckid), invoiceID, jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "betAmount"));
	sprintf(argv[4], "\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"", jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "betAmount"));

	invoice = cJSON_CreateObject();
	retval = make_command(argc, argv, &invoice);

	if (retval == OK) {
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "invoice");
		cJSON_AddNumberToObject(invoice_info, "playerID", jint(argjson, "playerID"));
		cJSON_AddNumberToObject(invoice_info, "round", jint(argjson, "round"));
		cJSON_AddStringToObject(invoice_info, "label", argv[3]);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(invoice));
		retval = (nn_send(bet->pubsock, cJSON_Print(invoice_info), strlen(cJSON_Print(invoice_info)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	} else {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

static int32_t bet_create_betting_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t argc, retval = OK;
	char **argv = NULL, hexstr[65] = { 0 };
	cJSON *invoice_info = NULL, *invoice = NULL;

	invoiceID++;
	argc = 5;
	bet_alloc_args(argc, &argv);
	dcv_info.betamount += jint(argjson, "betAmount");

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "invoice");
	sprintf(argv[2], "%ld", (long int)jint(argjson, "invoice_amount") * mchips_msatoshichips);
	sprintf(argv[3], "%s_%d_%d_%d_%d", bits256_str(hexstr, dcv_info.deckid), invoiceID, jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "invoice_amount"));
	sprintf(argv[4], "\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"", jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "invoice_amount"));

	invoice = cJSON_CreateObject();
	retval = make_command(argc, argv, &invoice);
	if (retval == OK) {
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "bettingInvoice");
		cJSON_AddNumberToObject(invoice_info, "playerID", jint(argjson, "playerID"));
		cJSON_AddNumberToObject(invoice_info, "round", jint(argjson, "round"));
		cJSON_AddStringToObject(invoice_info, "label", argv[3]);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(invoice));
		cJSON_AddItemToObject(invoice_info, "actionResponse", cJSON_GetObjectItem(argjson, "actionResponse"));
		retval = (nn_send(bet->pubsock, cJSON_Print(invoice_info), strlen(cJSON_Print(invoice_info)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	} else {
		dlg_error("%s", bet_err_str(retval));
	}
	bet_dealloc_args(argc, &argv);
	return retval;
}

int32_t bet_check_player_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t flag = 1;

	player_ready[jint(argjson, "playerid")] = 1;
	for (int i = 0; i < bet->maxplayers; i++) {
		if (player_ready[i] == 0) {
			flag = 0;
			break;
		}
	}
	return flag;
}

int32_t bet_receive_card(cJSON *player_card_info, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = OK, playerid, cardid, card_type, flag = 1;

	playerid = jint(player_card_info, "playerid");
	cardid = jint(player_card_info, "cardid");
	card_type = jint(player_card_info, "card_type");

	eval_game_p[no_of_cards] = playerid;
	eval_game_c[no_of_cards] = cardid;
	no_of_cards++;

	if (card_type == hole_card) {
		card_matrix[(cardid % bet->maxplayers)][(cardid / bet->maxplayers)] = 1;
		card_values[(cardid % bet->maxplayers)][(cardid / bet->maxplayers)] =
			jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_1) {
		card_matrix[playerid][no_of_hole_cards] = 1;
		card_values[playerid][no_of_hole_cards] = jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_2) {
		card_matrix[playerid][no_of_hole_cards + 1] = 1;
		card_values[playerid][no_of_hole_cards + 1] = jint(player_card_info, "decoded_card");
	} else if (card_type == flop_card_3) {
		card_matrix[playerid][no_of_hole_cards + 2] = 1;
		card_values[playerid][no_of_hole_cards + 2] = jint(player_card_info, "decoded_card");
	} else if (card_type == turn_card) {
		card_matrix[playerid][no_of_hole_cards + no_of_flop_cards] = 1;
		card_values[playerid][no_of_hole_cards + no_of_flop_cards] = jint(player_card_info, "decoded_card");
	} else if (card_type == river_card) {
		card_matrix[playerid][no_of_hole_cards + no_of_flop_cards + no_of_turn_card] = 1;
		card_values[playerid][no_of_hole_cards + no_of_flop_cards + no_of_turn_card] =
			jint(player_card_info, "decoded_card");
	}

	if (hole_cards_drawn == 0) {
		flag = 1;
		for (int i = 0; ((i < no_of_hole_cards) && (flag)); i++) {
			for (int j = 0; ((j < bet->maxplayers) && (flag)); j++) {
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
			for (int j = 0; ((j < bet->maxplayers) && (flag)); j++) {
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
			for (int j = 0; ((j < bet->maxplayers) && (flag)); j++) {
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
			for (int j = 0; ((j < bet->maxplayers) && (flag)); j++) {
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
			retval = bet_dcv_small_blind(NULL, bet, vars);
		} else {
			retval = bet_dcv_round_betting(NULL, bet, vars);
		}
	} else {
		retval = bet_dcv_turn(player_card_info, bet, vars);
	}

	return retval;
}

void bet_reset_all_dcv_params(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	players_joined = 0;
	turn = 0;
	no_of_cards = 0;
	no_of_rounds = 0;
	no_of_bets = 0;
	hole_cards_drawn = 0;
	community_cards_drawn = 0;
	flop_cards_drawn = 0;
	turn_card_drawn = 0;
	river_card_drawn = 0;
	invoiceID = 0;

	heartbeat_on = 0;

	for (int i = 0; i < bet->maxplayers; i++)
		player_ready[i] = 0;

	for (int i = 0; i < hand_size; i++) {
		for (int j = 0; j < bet->maxplayers; j++) {
			card_matrix[j][i] = 0;
			card_values[j][i] = -1;
		}
	}

	dcv_info.numplayers = 0;
	dcv_info.maxplayers = bet->maxplayers;
	bet_permutation(dcv_info.permis, bet->range);
	dcv_info.deckid = rand256(0);
	dcv_info.dcv_key.priv = curve25519_keypair(&dcv_info.dcv_key.prod);
	for (int i = 0; i < bet->range; i++) {
		permis_d[i] = dcv_info.permis[i];
	}

	vars->turni = 0;
	vars->round = 0;
	vars->pot = 0;
	vars->last_turn = 0;
	vars->last_raise = 0;
	for (int i = 0; i < bet->maxplayers; i++) {
		vars->funds[i] = 0;
		for (int j = 0; j < CARDS777_MAXROUNDS; j++) {
			vars->bet_actions[i][j] = 0;
			vars->betamount[i][j] = 0;
		}
	}

	dealerPosition = (dealerPosition + 1) % bet->maxplayers;
	vars->dealer = dealerPosition;

	bet->numplayers = 0;
	bet->cardid = -1;
	bet->turni = -1;
	bet->no_of_turns = 0;

	for (int32_t i = 0; i < no_of_txs; i++) {
		memset(tx_ids[i], 0x00, sizeof(tx_ids[i]));
	}
	no_of_txs = 0;

	for (int32_t i = 0; i < no_of_rand_str; i++) {
		memset(tx_rand_str[i], 0x00, sizeof(tx_rand_str[i]));
	}
	no_of_rand_str = 0;
	dcv_state = 0;
	bet_set_table_id();
	bet_init_player_seats_info();
}

void bet_dcv_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *reset_info = NULL;
	bet_reset_all_dcv_params(bet, vars);

	reset_info = cJSON_CreateObject();
	cJSON_AddStringToObject(reset_info, "method", "reset");
	bet_push_dcv_to_gui(reset_info);
}

void bet_game_info(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *game_info = NULL, *game_details = NULL, *game_state = NULL;

	game_info = cJSON_CreateObject();
	cJSON_AddStringToObject(game_info, "method", "game_info");
	cJSON_AddStringToObject(game_info, "table_id", table_id);

	game_state = cJSON_CreateObject();
	cJSON_AddNumberToObject(game_state, "maxplayers", bet->maxplayers);
	cJSON_AddNumberToObject(game_state, "rounds", vars->round);

	game_details = cJSON_CreateArray();
	for (int32_t i = 0; i < bet->maxplayers; i++) {
		cJSON *temp = cJSON_CreateObject();

		cJSON *player_action_info = cJSON_CreateArray();

		for (int32_t j = 0; j <= vars->round; j++)
			cJSON_AddItemToArray(player_action_info, cJSON_CreateNumber(vars->bet_actions[i][j]));

		cJSON *player_card_info = cJSON_CreateArray();
		for (int32_t j = 0; j < hand_size; j++)
			cJSON_AddItemToArray(player_card_info, cJSON_CreateNumber(card_values[i][j]));

		cJSON_AddItemToObject(temp, "bet_actions", player_action_info);
		cJSON_AddItemToObject(temp, "player_cards", player_card_info);
		cJSON_AddItemToArray(game_details, temp);
	}
	cJSON_AddItemToObject(game_state, "game_details", game_details);
	cJSON_AddItemToObject(game_info, "game_state", game_state);

	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			bet_msg_cashier(game_info, notary_node_ips[i]);
		}
	}

	int argc = 3;
	char **argv = NULL;
	char *sql_query = calloc(sql_query_size, sizeof(char));
	bet_alloc_args(argc, &argv);
	strcpy(argv[0], "dcv_game_state");
	sprintf(argv[1], "\'%s\'", table_id);
	sprintf(argv[2], "\'%s\'", cJSON_Print(game_state));

	bet_make_insert_query(argc, argv, &sql_query);
	bet_run_query(sql_query);
	bet_dealloc_args(argc, &argv);
	if (sql_query)
		free(sql_query);

	int bytes = nn_send(bet->pubsock, cJSON_Print(game_info), strlen(cJSON_Print(game_info)), 0);
	if (bytes < 0)
		dlg_error("nn_send failed\n");
}

static cJSON *payout_tx_data_info(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *game_info = NULL, *game_details = NULL;
	cJSON *player_ids_info = NULL;

	game_info = cJSON_CreateObject();
	cJSON_AddStringToObject(game_info, "table_id", table_id);

	cJSON_AddNumberToObject(game_info, "maxplayers", bet->maxplayers);
	cJSON_AddNumberToObject(game_info, "rounds", vars->round);

	game_details = cJSON_CreateArray();
	for (int32_t i = 0; i < bet->maxplayers; i++) {
		cJSON *temp = cJSON_CreateObject();

		cJSON *player_action_info = cJSON_CreateArray();

		for (int32_t j = 0; j <= vars->round; j++)
			cJSON_AddItemToArray(player_action_info, cJSON_CreateNumber(vars->bet_actions[i][j]));

		cJSON *player_card_info = cJSON_CreateArray();
		for (int32_t j = 0; j < hand_size; j++)
			cJSON_AddItemToArray(player_card_info, cJSON_CreateNumber(card_values[i][j]));

		cJSON_AddItemToObject(temp, "bet_actions", player_action_info);
		cJSON_AddItemToObject(temp, "player_cards", player_card_info);
		cJSON_AddItemToArray(game_details, temp);
	}

	cJSON_AddItemToObject(game_info, "game_state", game_details);

	player_ids_info = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_rand_str; i++) {
		cJSON_AddItemToArray(player_ids_info, cJSON_CreateString(tx_rand_str[i]));
	}
	cJSON_AddItemToObject(game_info, "player_ids", player_ids_info);

	cJSON_AddNumberToObject(game_info, "threshold_value", threshold_value);
	cJSON *msig_addr_nodes = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			cJSON_AddItemToArray(msig_addr_nodes, cJSON_CreateString(notary_node_ips[i]));
		}
	}
	cJSON_AddItemToObject(game_info, "msig_addr_nodes", msig_addr_nodes);
	cJSON_AddStringToObject(game_info, "tx_type", "payout");
	return game_info;
}

static int32_t bet_dcv_poker_winner(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	double dcv_commission = 0, dev_commission = 0, player_amounts[bet->maxplayers];
	cJSON *payout_info = NULL, *dev_info = NULL, *dcv_info = NULL, *payout_tx_info = NULL, *data_info = NULL;
	char *hex_str = NULL;

	for (int32_t i = 0; i < bet->maxplayers; i++) {
		if (vars->win_funds[i] > 0) {
			double c_dcv, c_dev;
			c_dcv = ((dcv_commission_percentage * vars->win_funds[i]) / 100) * SB_in_chips;
			c_dev = ((dev_fund_commission * vars->win_funds[i]) / 100) * SB_in_chips;

			dcv_commission += c_dcv;
			dev_commission += c_dev;
			player_amounts[i] = (vars->win_funds[i] * SB_in_chips) - (c_dev + c_dcv);
			player_amounts[i] += (vars->funds[i] * SB_in_chips);
		} else {
			player_amounts[i] = (vars->funds[i] * SB_in_chips);
		}
	}

	if (dcv_commission > chips_tx_fee) {
		dcv_commission -= chips_tx_fee;
	} else if (dev_commission > chips_tx_fee) {
		dev_commission -= chips_tx_fee;
	} else {
		for (int32_t i = 0; i < bet->maxplayers; i++) {
			if (vars->winners[i] == 1) {
				player_amounts[i] -= chips_tx_fee;
				break;
			}
		}
	}
	payout_info = cJSON_CreateArray();

	dev_info = cJSON_CreateObject();
	dcv_info = cJSON_CreateObject();
	if (dcv_commission > chips_tx_fee) {
		cJSON_AddStringToObject(dcv_info, "address", chips_get_wallet_address());
		cJSON_AddNumberToObject(dcv_info, "amount", dcv_commission);
		cJSON_AddItemToArray(payout_info, dcv_info);
	}

	if (dev_commission > chips_tx_fee) {
		cJSON_AddStringToObject(dev_info, "address", dev_fund_addr);
		cJSON_AddNumberToObject(dev_info, "amount", dev_commission);
		cJSON_AddItemToArray(payout_info, dev_info);
	}

	for (int32_t i = 0; i < bet->maxplayers; i++) {
		cJSON *temp = cJSON_CreateObject();
		if (player_amounts[i] > 0) {
			cJSON_AddStringToObject(temp, "address",
						vars->player_chips_addrs[vars->req_id_to_player_id_mapping[i]]);
			cJSON_AddNumberToObject(temp, "amount", player_amounts[i]);
			cJSON_AddItemToArray(payout_info, temp);
		}
	}
	data_info = payout_tx_data_info(bet, vars);
	hex_str = calloc(tx_data_size * 2, sizeof(char));
	str_to_hexstr(cJSON_Print(data_info), hex_str);

	payout_tx_info = chips_create_payout_tx(payout_info, no_of_txs, tx_ids, hex_str);
	if (hex_str)
		free(hex_str);

	dlg_info("payout_tx_info::%s\n", cJSON_Print(payout_tx_info));
	if (payout_tx_info == NULL) {
		retval = ERR_PAYOUT_TX;
		return retval;
	}
	retval = (nn_send(bet->pubsock, cJSON_Print(payout_tx_info), strlen(cJSON_Print(payout_tx_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

int32_t det_dcv_pot_split(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK, eval_players[CARDS777_MAXPLAYERS] = { 0 }, min_win_amount, no_of_winners = 0, flag = 1,
		total_init_amount = 0, total_win_amount = 0;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS], max_score = 0;

	for (int i = 0; i < bet->maxplayers; i++) {
		if (vars->bet_actions[i][(vars->round)] == fold)
			scores[i] = 0;
		else {
			for (int j = 0; j < hand_size; j++) {
				h[j] = (unsigned char)card_values[i][j];
			}
			scores[i] = seven_card_draw_score(h);
		}
	}
	for (int32_t i = 0; i < bet->maxplayers; i++) {
		vars->funds_spent[i] = 0;
		for (int32_t j = 0; j <= vars->round; j++) {
			vars->funds_spent[i] += vars->betamount[i][j];
		}
	}
	dlg_info("Funds spent");
	for (int32_t i = 0; i < bet->maxplayers; i++) {
		dlg_info("player id ::%d::funds_spent::%d", i, vars->funds_spent[i]);
	}
	while (flag) {
		max_score = 0;
		no_of_winners = 0;
		for (int i = 0; ((i < bet->maxplayers) && (eval_players[i] == 0)); i++) {
			if (max_score < scores[i])
				max_score = scores[i];
		}
		min_win_amount = vars->pot;
		for (int i = 0; ((i < bet->maxplayers) && (eval_players[i] == 0)); i++) {
			dlg_info("max_score::%ld::score[%d]->%ld", max_score, i, scores[i]);
			if ((scores[i] == max_score) && (vars->bet_actions[i][(vars->round)] != fold)) {
				vars->winners[i] = 1;
				dlg_info("winner ::%d", i);
				no_of_winners++;
				if (min_win_amount > vars->funds_spent[i])
					min_win_amount = vars->funds_spent[i];
			}
		}
		int sub_pot = 0;
		for (int i = 0; ((i < bet->maxplayers) && (eval_players[i] == 0)); i++) {
			if (vars->funds_spent[i] >= min_win_amount) {
				sub_pot += min_win_amount;
				vars->funds_spent[i] -= min_win_amount;
			} else {
				sub_pot += vars->funds_spent[i];
				vars->funds_spent[i] = 0;
			}
		}
		for (int i = 0; ((i < bet->maxplayers) && (eval_players[i] == 0)); i++) {
			if (vars->winners[i] == 1) {
				vars->win_funds[i] += (sub_pot / no_of_winners);
			}
			if (vars->funds_spent[i] == 0) {
				eval_players[i] = 1;
			}
		}
		dlg_info("sub_pot::%d", sub_pot);
		flag = 0;
		int32_t eval_players_left = 0, eval_player;
		for (int i = 0; ((i < bet->maxplayers) && (eval_players[i] == 0)); i++) {
			flag = 1;
			eval_players_left++;
			eval_player = i;
		}
		if (eval_players_left == 1) {
			vars->win_funds[eval_player] = vars->funds_spent[eval_player];
			vars->funds_spent[eval_player] = 0;
			flag = 0;
		}
		for (int i = 0; i < bet->maxplayers; i++) {
			dlg_info("win_funds::%d, funds::%d, ini_funds::%d", vars->win_funds[i], vars->funds[i],
				 vars->ini_funds[i]);
		}
	}
	for (int i = 0; i < bet->maxplayers; i++) {
		total_init_amount += vars->ini_funds[i];
		total_win_amount += (vars->win_funds[i] + vars->funds[i]);
	}
	if (total_init_amount != total_win_amount) {
		dlg_error("Pay_in chips :: %d, Payout_chips :: %d", total_init_amount, total_win_amount);
		retval = ERR_BET_AMOUNTS_MISMATCH;
	}
	return retval;
}

int32_t bet_evaluate_hand(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *final_info = NULL, *all_hole_card_info = NULL, *board_card_info = NULL, *hole_card_info = NULL,
	      *show_info = NULL, *reset_info = NULL;
	char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };

	bet_game_info(bet, vars);

	if ((retval = det_dcv_pot_split(bet, vars)) != OK)
		return retval;
	if ((retval = bet_dcv_poker_winner(bet, vars)) != OK)
		return retval;

	final_info = cJSON_CreateObject();
	cJSON_AddStringToObject(final_info, "method", "finalInfo");
	all_hole_card_info = cJSON_CreateArray();
	board_card_info = cJSON_CreateArray();
	for (int i = 0; i < bet->maxplayers; i++) {
		hole_card_info = cJSON_CreateArray();
		for (int j = 0; j < no_of_hole_cards; j++) {
			if (card_values[i][j] != -1)
				cJSON_AddItemToArray(hole_card_info, cJSON_CreateString(cards[card_values[i][j]]));
			else
				cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
		}
		cJSON_AddItemToArray(all_hole_card_info, hole_card_info);
	}

	for (int j = no_of_hole_cards; j < hand_size; j++) {
		if (card_values[0][j] != -1)
			cJSON_AddItemToArray(board_card_info, cJSON_CreateString(cards[card_values[0][j]]));
		else
			cJSON_AddItemToArray(board_card_info, cJSON_CreateNull());
	}

	show_info = cJSON_CreateObject();
	cJSON_AddItemToObject(show_info, "allHoleCardsInfo", all_hole_card_info);
	cJSON_AddItemToObject(show_info, "boardCardInfo", board_card_info);

	cJSON_AddItemToObject(final_info, "showInfo", show_info);
	cJSON_AddNumberToObject(final_info, "win_amount", vars->pot);

	cJSON *winnersInfo = cJSON_CreateArray();
	cJSON *winningAmounts = cJSON_CreateArray();
	cJSON *settleAmounts = cJSON_CreateArray();
	for (int i = 0; i < bet->maxplayers; i++) {
		if (vars->winners[i] == 1) {
			cJSON_AddItemToArray(winnersInfo, cJSON_CreateNumber(i));
			cJSON_AddItemToArray(winningAmounts, cJSON_CreateNumber(vars->win_funds[i]));
		}
		cJSON_AddItemToArray(settleAmounts, cJSON_CreateNumber(vars->win_funds[i] + vars->funds[i]));
	}
	cJSON_AddItemToObject(final_info, "winners", winnersInfo);
	cJSON_AddItemToObject(final_info, "winningAmounts", winningAmounts);
	cJSON_AddItemToObject(final_info, "settleAmounts", settleAmounts);

	dlg_info("Final Info :: %s\n", cJSON_Print(final_info));

	retval = (nn_send(bet->pubsock, cJSON_Print(final_info), strlen(cJSON_Print(final_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	if (retval != OK)
		return retval;

	sleep(5);
	if (wsi_global_host) {
		lws_write(wsi_global_host, (unsigned char *)cJSON_Print(final_info), strlen(cJSON_Print(final_info)),
			  0);
	}

	// Reset the params and continue the next hand automatically
	find_bvv();
	reset_info = cJSON_CreateObject();
	cJSON_AddStringToObject(reset_info, "method", "reset");
	retval = (nn_send(bet->pubsock, cJSON_Print(reset_info), strlen(cJSON_Print(reset_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	bet_dcv_reset(bet, vars);

	return retval;
}

int32_t bet_ln_check(struct privatebet_info *bet)
{
	char channel_id[100];
	int32_t retval = OK, channel_state;
	char uri[100];

	for (int32_t i = 0; i < bet_dcv->maxplayers; i++) {
		strcpy(uri, (const char *)dcv_info.uri[i]);
		strcpy(channel_id, strtok(uri, "@"));

		while ((channel_state = ln_get_channel_status(channel_id)) != CHANNELD_NORMAL) {
			dlg_info("Channel state::%d\n", channel_state);
			if (channel_state == CHANNELD_AWAITING_LOCKIN) {
				dlg_info("CHANNELD AWAITING LOCKIN\r");
				fflush(stdout);
				sleep(1);
			} else if ((channel_state != CHANNELD_AWAITING_LOCKIN) && (channel_state != CHANNELD_NORMAL)) {
				dlg_warn("Player: %d -> DCV LN Channel is not established, current channel_state=%d\n",
					 i, channel_state);
				sleep(1);
			}
		}
		dlg_info("Player %d --> DCV channel ready\n", i);
	}
	return retval;
}

static int32_t bet_award_winner(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t argc, retval = OK;
	char **argv = NULL, channel_id[100], *invoice = NULL;
	cJSON *pay_response = NULL, *invoice_info = NULL, *fund_channel_info = NULL;

	argc = 4;
	bet_alloc_args(argc, &argv);
	strcpy(channel_id, strtok((char *)dcv_info.uri[jint(argjson, "playerid")], "@"));
	if (ln_get_channel_status(channel_id) != CHANNELD_NORMAL) {
		argv = bet_copy_args(argc, "lightning-cli", "fundchannel", channel_id, "500000");

		fund_channel_info = cJSON_CreateObject();
		retval = make_command(argc, argv, &fund_channel_info);
		if (retval != OK) {
			dlg_error("%s", bet_err_str(retval));
			goto end;
		}
		dlg_info("Fund channel response:%s\n", cJSON_Print(fund_channel_info));
		int state;
		while ((state = ln_get_channel_status(channel_id)) != CHANNELD_NORMAL) {
			if (state == CHANNELD_AWAITING_LOCKIN) {
				dlg_info("CHANNELD_AWAITING_LOCKIN");
			} else
				dlg_info("LN Channel state::%d\n", state);

			dlg_info("LN Channel state::%d\n", state);
			sleep(10);
		}
		dlg_info("LN Channel state::%d\n", state);
	}
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);

	bet_dealloc_args(argc, &argv);
	argc = 3;
	bet_alloc_args(argc, &argv);
	argv = bet_copy_args(argc, "lightning-cli", "pay", jstr(invoice_info, "bolt11"));
	pay_response = cJSON_CreateObject();
	retval = make_command(argc, argv, &pay_response);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		goto end;
	}

end:
	bet_dealloc_args(argc, &argv);
	return retval;
}
static void bet_push_joinInfo(cJSON *argjson, int32_t numplayers)
{
	cJSON *join_info = NULL;

	join_info = cJSON_CreateObject();
	cJSON_AddStringToObject(join_info, "method", "join_info");
	cJSON_AddNumberToObject(join_info, "joined_playerid", jint(argjson, "gui_playerID"));
	cJSON_AddNumberToObject(join_info, "tot_players_joined", numplayers);
	bet_dcv_lws_write(join_info);
}

static int32_t bet_dcv_stack_info_resp(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK;
	cJSON *stack_info_resp = NULL;
	cJSON *msig_addr_nodes = NULL;

	stack_info_resp = cJSON_CreateObject();
	if ((is_table_private) && ((NULL == jstr(argjson, "table_password")) ||
				   (0 != strcmp(table_password, jstr(argjson, "table_password"))))) {
		cJSON_AddStringToObject(stack_info_resp, "method", "game_abort");
		cJSON_AddNumberToObject(stack_info_resp, "err_no", ERR_PT_PLAYER_UNAUTHORIZED);
	} else {
		cJSON_AddStringToObject(stack_info_resp, "method", "stack_info_resp");
		cJSON_AddStringToObject(stack_info_resp, "id", jstr(argjson, "id"));
		cJSON_AddNumberToObject(stack_info_resp, "max_players", max_players);
		cJSON_AddNumberToObject(stack_info_resp, "bb_in_chips", BB_in_chips);
		cJSON_AddNumberToObject(stack_info_resp, "table_min_stake", table_min_stake);
		cJSON_AddNumberToObject(stack_info_resp, "table_max_stake", table_max_stake);
		cJSON_AddNumberToObject(stack_info_resp, "dcv_commission", dcv_commission_percentage);
		cJSON_AddNumberToObject(stack_info_resp, "chips_tx_fee", chips_tx_fee);
		cJSON_AddStringToObject(stack_info_resp, "legacy_m_of_n_msig_addr", legacy_m_of_n_msig_addr);
		cJSON_AddStringToObject(stack_info_resp, "table_id", table_id);
		cJSON_AddNumberToObject(stack_info_resp, "threshold_value", threshold_value);
		cJSON_AddStringToObject(stack_info_resp, "gui_url", dcv_hosted_gui_url);
		msig_addr_nodes = cJSON_CreateArray();
		for (int32_t i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1) {
				cJSON_AddItemToArray(msig_addr_nodes, cJSON_CreateString(notary_node_ips[i]));
			}
		}
		cJSON_AddItemToObject(stack_info_resp, "msig_addr_nodes", msig_addr_nodes);
	}
	retval = (nn_send(bet->pubsock, cJSON_Print(stack_info_resp), strlen(cJSON_Print(stack_info_resp)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

static int32_t bet_dcv_process_signed_raw_tx(cJSON *argjson)
{
	int32_t retval = OK;
	cJSON *raw_tx = NULL;

	no_of_signers++;
	if (no_of_signers < max_no_of_signers) {
		is_signed[jint(argjson, "playerid")] = 1;
		retval = chips_publish_multisig_tx(jstr(argjson, "tx"));
	} else {
		raw_tx = cJSON_CreateObject();
		cJSON_AddStringToObject(raw_tx, "hex", jstr(argjson, "tx"));
		chips_send_raw_tx(raw_tx);
		if (raw_tx == NULL)
			retval = ERR_CHIPS_TX_FAILED;
	}
	return retval;
}

static void bet_send_tx_reverse_rqst(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *tx_reverse_rqst_info = NULL;
	int32_t bytes;

	tx_reverse_rqst_info = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_reverse_rqst_info, "method", "tx_reverse");
	cJSON_AddStringToObject(tx_reverse_rqst_info, "txid", jstr(argjson, "tx_info"));
	cJSON_AddNumberToObject(tx_reverse_rqst_info, "dcv_state", dcv_state);
	cJSON_AddNumberToObject(tx_reverse_rqst_info, "players_joined", players_joined);
	cJSON_AddNumberToObject(tx_reverse_rqst_info, "max_players", bet->maxplayers);
	cJSON_AddStringToObject(tx_reverse_rqst_info, "id", jstr(argjson, "id"));
	bytes = nn_send(bet->pubsock, cJSON_Print(tx_reverse_rqst_info), strlen(cJSON_Print(tx_reverse_rqst_info)), 0);
	if (bytes < 0) {
		dlg_error("Error in sending data");
	}
}

static int32_t bet_dcv_verify_tx(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t block_height, retval = OK;
	char *hex_data = NULL, *data = NULL;
	cJSON *data_info = NULL, *tx_info = NULL;

	tx_info = cJSON_CreateObject();
	tx_info = cJSON_GetObjectItem(argjson, "tx_info");
	if (tx_info == NULL)
		return ERR_CHIPS_INVALID_TX;

	dlg_info("%s", cJSON_Print(argjson));

	block_height = jint(argjson, "block_height");
	while (chips_get_block_count() < block_height) {
		sleep(1);
	}

	if (chips_check_if_tx_unspent(cJSON_Print(tx_info)) == 1) {
		hex_data = calloc(tx_data_size * 2, sizeof(char));
		retval = chips_extract_data(cJSON_Print(tx_info), &hex_data);
		if (retval != OK)
			goto end;
		data = calloc(tx_data_size, sizeof(char));
		hexstr_to_str(hex_data, data);
		data_info = cJSON_CreateObject();
		data_info = cJSON_Parse(data);
		if (strcmp(table_id, jstr(data_info, "table_id")) == 0) {
			if (strcmp(jstr(argjson, "id"), jstr(data_info, "player_id")) == 0) {
				//pthread_mutex_lock(&mutex);
				if (no_of_txs == bet->maxplayers) {
					bet_send_tx_reverse_rqst(argjson, bet);
					retval = ERR_DEALER_TABLE_FULL;
				} else {
					strcpy(tx_ids[no_of_txs++], unstringify(cJSON_Print(tx_info)));
					if (no_of_txs == bet_dcv->maxplayers)
						dcv_state = dealer_table_full;
				}
				//pthread_mutex_unlock(&mutex);
			}
		}
	}
end:
	if (data)
		free(data);
	if (hex_data)
		free(hex_data);

	return retval;
}

void bet_init_player_seats_info()
{
	for (int i = 0; i < max_players; i++) {
		sprintf(player_seats_info[i].seat_name, "player%d", i + 1);
		player_seats_info[i].seat = i;
		player_seats_info[i].chips = 0;
		player_seats_info[i].empty = pos_on_table_empty;
		player_seats_info[i].playing = 0;
	}
}

static int32_t bet_dcv_check_pos_status(cJSON *argjson, struct privatebet_info *bet, int32_t *pos_status)
{
	int32_t gui_playerID, retval = OK;
	cJSON *join_res = NULL;

	gui_playerID = jint(argjson, "gui_playerID");
	if ((gui_playerID < 0) && (gui_playerID >= bet->maxplayers)) {
		retval = ERR_INVALID_POS;
	}
	dlg_info("%d", player_seats_info[gui_playerID].empty);
	*pos_status =
		(player_seats_info[gui_playerID].empty == pos_on_table_empty) ? pos_on_table_empty : pos_on_table_full;
	dlg_info("%d %d", player_seats_info[gui_playerID].empty, *pos_status);

	if (*pos_status == pos_on_table_full) {
		dlg_warn("Seat Taken\n");
		join_res = cJSON_CreateObject();
		cJSON_AddStringToObject(join_res, "method", "join_res");
		cJSON_AddNumberToObject(join_res, "playerid", gui_playerID);
		cJSON_AddNumberToObject(join_res, "pos_status", *pos_status);
		cJSON_AddStringToObject(join_res, "req_identifier", jstr(argjson, "req_identifier"));
		retval = (nn_send(bet->pubsock, cJSON_Print(join_res), strlen(cJSON_Print(join_res)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
	}
	return retval;
}

static int32_t bet_dcv_process_join_req(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = OK, pos_status;

	if (((retval = bet_dcv_check_pos_status(argjson, bet, &pos_status)) == ERR_INVALID_POS) ||
	    (pos_status == pos_on_table_full)) {
		dlg_info("%d", retval);
		return retval;
	}
	dlg_info("%s", cJSON_Print(argjson));
	if (bet->numplayers < bet->maxplayers) {
		char *req_id = jstr(argjson, "req_identifier");
		for (int32_t i = 0; i < no_of_rand_str; i++) {
			if (strcmp(tx_rand_str[i], req_id) == 0) {
				vars->req_id_to_player_id_mapping[jint(argjson, "gui_playerID")] = i;
				break;
			}
		}
		retval = bet_player_join_req(argjson, bet, vars);
		if (retval != OK)
			return retval;

		bet_push_joinInfo(argjson, bet->numplayers);
		if (bet->numplayers == bet->maxplayers) {
			cJSON *stakes_info = cJSON_CreateObject();
			cJSON *stakes = cJSON_CreateArray();
			for (int i = 0; i < bet->maxplayers; i++) {
				dlg_info("player::%d::funds::%d::ini_funds::%d", i, vars->funds[i], vars->ini_funds[i]);
			}
			for (int i = 0; i < bet->maxplayers; i++) {
				vars->funds[i] = vars->ini_funds[vars->req_id_to_player_id_mapping[i]];
			}
			for (int i = 0; i < bet->maxplayers; i++) {
				vars->ini_funds[i] = vars->funds[i];
			}
			for (int i = 0; i < bet->maxplayers; i++) {
				dlg_info("player::%d::funds::%d::ini_funds::%d", i, vars->funds[i], vars->ini_funds[i]);
			}
			cJSON_AddStringToObject(stakes_info, "method", "player_stakes_info");
			for (int i = 0; i < bet->maxplayers; i++) {
				cJSON_AddItemToArray(stakes, cJSON_CreateNumber(vars->funds[i]));
			}
			cJSON_AddItemToObject(stakes_info, "stakes", stakes);
			dlg_info("%s", cJSON_Print(stakes_info));
			retval = (nn_send(bet->pubsock, cJSON_Print(stakes_info), strlen(cJSON_Print(stakes_info)), 0) <
				  0) ?
					 ERR_NNG_SEND :
					 OK;

			heartbeat_on = 1;
			if (bet_ln_config == BET_WITH_LN) {
				retval = bet_ln_check(bet);
				if (retval < 0) {
					dlg_error("Issue in establishing the LN channels");
					return retval;
				}
			}
			retval = bet_check_bvv_ready(bet);
		}
	}
	return retval;
}

static int32_t bet_dcv_process_tx(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, char *addr)
{
	int32_t retval = OK;
	double payin_tx_amount;
	char *sql_stmt = NULL, *rand_str = NULL;
	cJSON *msig_addr_nodes = NULL, *tx_status = NULL;

	retval = (dcv_state == 1) ? ERR_DEALER_TABLE_FULL : bet_dcv_verify_tx(argjson, bet);
	if (retval != OK)
		return retval;

	strcpy(vars->player_chips_addrs[no_of_rand_str], jstr(argjson, "chips_addr"));
	strcpy(tx_rand_str[no_of_rand_str++], jstr(argjson, "id"));

	payin_tx_amount = chips_get_balance_on_address_from_tx(addr, jstr(argjson, "tx_info"));
	rand_str = jstr(argjson, "id");
	for (int i = 0; i < no_of_rand_str; i++) {
		if (strcmp(tx_rand_str[i], rand_str) == 0) {
			vars->funds[i] = payin_tx_amount / SB_in_chips;
			vars->ini_funds[i] = payin_tx_amount / SB_in_chips;
			vars->win_funds[i] = 0;
			vars->winners[i] = 0;
		}
	}
	sql_stmt = calloc(sql_query_size, sizeof(char));

	msig_addr_nodes = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			cJSON_AddItemToArray(msig_addr_nodes, cJSON_CreateString(notary_node_ips[i]));
		}
	}

	sprintf(sql_stmt, "INSERT INTO dcv_tx_mapping values(\'%s\',\'%s\',\'%s\',\'%s\',%d,%d);",
		jstr(argjson, "tx_info"), table_id, rand_str, cJSON_Print(msig_addr_nodes), tx_unspent,
		threshold_value);
	retval = bet_run_query(sql_stmt);

	tx_status = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_status, "method", "tx_status");
	cJSON_AddStringToObject(tx_status, "id", jstr(argjson, "id"));
	cJSON_AddNumberToObject(tx_status, "tx_validity", retval);
	cJSON_AddNumberToObject(tx_status, "player_funds", (payin_tx_amount / SB_in_chips));

	retval = (nn_send(bet->pubsock, cJSON_Print(tx_status), strlen(cJSON_Print(tx_status)), 0) < 0) ? ERR_NNG_SEND :
													  OK;
	if (sql_stmt)
		free(sql_stmt);
	return retval;
}

static int32_t bet_get_dcv_state(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK;
	cJSON *dcv_state_info = NULL;

	dcv_state_info = cJSON_CreateObject();
	cJSON_AddStringToObject(dcv_state_info, "method", "dcv_state");
	cJSON_AddNumberToObject(dcv_state_info, "dcv_state", dcv_state);
	cJSON_AddNumberToObject(dcv_state_info, "players_joined", players_joined);
	cJSON_AddNumberToObject(dcv_state_info, "max_players", bet->maxplayers);
	cJSON_AddStringToObject(dcv_state_info, "id", jstr(argjson, "id"));

	retval = (nn_send(bet->pubsock, cJSON_Print(dcv_state_info), strlen(cJSON_Print(dcv_state_info)), 0) < 0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

static int32_t bet_get_dcv_state_for_bvv(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK;
	cJSON *dcv_state_info = NULL;

	dcv_state_info = cJSON_CreateObject();
	cJSON_AddStringToObject(dcv_state_info, "method", "dcv_state");
	cJSON_AddNumberToObject(dcv_state_info, "dcv_state", dcv_state);
	cJSON_AddNumberToObject(dcv_state_info, "players_joined", players_joined);
	cJSON_AddNumberToObject(dcv_state_info, "max_players", bet->maxplayers);
	cJSON_AddStringToObject(dcv_state_info, "id", jstr(argjson, "id"));

	retval = (nn_send(bet_dcv_bvv->pubsock, cJSON_Print(dcv_state_info), strlen(cJSON_Print(dcv_state_info)), 0) <
		  0) ?
			 ERR_NNG_SEND :
			 OK;
	return retval;
}

static int32_t bet_handle_player_errs_by_dealer(cJSON *argjson, struct privatebet_info *bet)
{
	int32_t retval = OK;
	cJSON *game_abort = NULL;

	dlg_warn("Player :: %d encounters the error ::%s", jint(argjson, "playerid"),
		 bet_err_str(jint(argjson, "err_no")));
	game_abort = cJSON_CreateObject();
	switch (jint(argjson, "err_no")) {
	case ERR_DECRYPTING_OWN_SHARE:
	case ERR_DECRYPTING_OTHER_SHARE:
	case ERR_CARD_RETRIEVING_USING_SS:
		cJSON_AddStringToObject(game_abort, "method", "game_abort");
		cJSON_AddNumberToObject(game_abort, "err_no", jint(argjson, "err_no"));
		cJSON_AddNumberToObject(game_abort, "playerid", jint(argjson, "playerid"));
		if ((retval = nn_send(bet->pubsock, cJSON_Print(game_abort), strlen(cJSON_Print(game_abort)), 0)) < 0) {
			dlg_error("%s", bet_err_str(retval));
		}
		exit(-1);
	case ERR_DEALER_TABLE_FULL:
	case ERR_CHIPS_INVALID_TX:
		cJSON_AddStringToObject(game_abort, "method", "game_abort_player");
		cJSON_AddStringToObject(game_abort, "id", jstr(argjson, "id"));
		cJSON_AddNumberToObject(game_abort, "err_no", jint(argjson, "err_no"));
		retval = (nn_send(bet->pubsock, cJSON_Print(game_abort), strlen(cJSON_Print(game_abort)), 0) < 0) ?
				 ERR_NNG_SEND :
				 OK;
		break;
	case ERR_LN_ADDRESS_TYPE_MISMATCH:
		break;
	default:
		dlg_warn("The error %s is not yet handled by the dealer or no action is needed from dealer",
			 bet_err_str(jint(argjson, "err_no")));
	}
	return retval;
}

void bet_dcv_backend_thrd(void *_ptr)
{
	int32_t retval = OK;
	char *method = NULL;
	cJSON *argjson = NULL;
	struct privatebet_info *bet = bet_dcv;
	struct privatebet_vars *vars = dcv_vars;

	argjson = cJSON_Parse(_ptr);
	pthread_mutex_lock(&mutex);
	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("%s", method);
		if (strcmp(method, "join_req") == 0) {
			retval = bet_dcv_process_join_req(argjson, bet, vars);
		} else if (strcmp(method, "init_p") == 0) {
			retval = bet_dcv_init(argjson, bet, vars);
			if (dcv_info.numplayers == dcv_info.maxplayers) {
				retval = bet_dcv_deck_init_info(argjson, bet, vars);
			}
		} else if (strcmp(method, "player_ready") == 0) {
			if (bet_check_player_ready(argjson, bet, vars)) {
				retval = bet_initiate_statemachine(argjson, bet, vars);
			}
		} else if (strcmp(method, "dealer_ready") == 0) {
			retval = bet_dcv_turn(argjson, bet, vars);

		} else if (strcmp(method, "playerCardInfo") == 0) {
			retval = bet_receive_card(argjson, bet, vars);
		} else if (strcmp(method, "invoiceRequest") == 0) {
			retval = bet_create_invoice(argjson, bet, vars);
		} else if (strcmp(method, "bettingInvoiceRequest") == 0) {
			retval = bet_create_betting_invoice(argjson, bet, vars);
		} else if (strcmp(method, "claim") == 0) {
			retval = bet_award_winner(argjson, bet, vars);
		} else if (strcmp(method, "requestShare") == 0) {
			retval = bet_relay(argjson, bet, vars);
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_betting_statemachine(argjson, bet, vars);
		} else if (strcmp(method, "display_current_state") == 0) {
			retval = bet_display_current_state(argjson, bet, vars);
		} else if (strcmp(method, "signedrawtransaction") == 0) {
			retval = bet_dcv_process_signed_raw_tx(argjson);
		} else if (strcmp(method, "stack_info_req") == 0) {
			if (no_of_rand_str < bet->maxplayers) {
				retval = bet_dcv_stack_info_resp(argjson, bet, vars);
			} else {
				bet_get_dcv_state(argjson, bet);
			}
		} else if (strcmp(method, "tx") == 0) {
			retval = bet_dcv_process_tx(argjson, bet, vars, legacy_m_of_n_msig_addr);
		} else if (strcmp(method, "live") == 0) {
			cJSON *live_info = cJSON_CreateObject();
			cJSON_AddStringToObject(live_info, "method", "live");
			cJSON_AddStringToObject(live_info, "id", jstr(argjson, "id"));
			retval =
				(nn_send(bet->pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0) < 0) ?
					ERR_NNG_SEND :
					OK;
		} else if (strcmp(method, "dcv_state") == 0) {
			retval = bet_get_dcv_state(argjson, bet);
		} else if (strcmp(method, "req_seats_info") == 0) {
			cJSON *seats_info_resp = NULL;
			cJSON *seats_info = bet_get_seats_json(bet->maxplayers);

			seats_info_resp = cJSON_CreateObject();
			cJSON_AddStringToObject(seats_info_resp, "method", "seats_info_resp");
			cJSON_AddStringToObject(seats_info_resp, "req_identifier", jstr(argjson, "req_identifier"));
			cJSON_AddItemToObject(seats_info_resp, "seats", seats_info);
			retval = (nn_send(bet->pubsock, cJSON_Print(seats_info_resp),
					  strlen(cJSON_Print(seats_info_resp)), 0) < 0) ?
					 ERR_NNG_SEND :
					 OK;
		} else if (strcmp(method, "player_active") == 0) {
			bet_dcv_update_player_status(argjson);
		} else if (strcmp(method, "share_info") == 0) {
			retval = bet_relay(argjson, bet, vars);
		} else if (strcmp(method, "player_error") == 0) {
			retval = bet_handle_player_errs_by_dealer(argjson, bet);
		} else {
			dlg_warn("unknown method :: %s", cJSON_Print(argjson));
			retval = (nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0) < 0) ?
					 ERR_NNG_SEND :
					 OK;
		}
	}
	pthread_mutex_unlock(&mutex);
	if (retval != OK) {
		cJSON_AddNumberToObject(argjson, "err_no", retval);
		bet_handle_player_errs_by_dealer(argjson, bet);
	}
}

void bet_dcv_backend_loop(void *_ptr)
{
	int32_t recvlen;
	cJSON *argjson = NULL;
	void *ptr = NULL;
	struct privatebet_info *bet = _ptr;

	dcv_info.numplayers = 0;
	dcv_info.maxplayers = bet->maxplayers;

	bet_permutation(dcv_info.permis, bet->range);
	dcv_info.deckid = rand256(0);
	dcv_info.dcv_key.priv = curve25519_keypair(&dcv_info.dcv_key.prod);

	for (int i = 0; i < bet->maxplayers; i++)
		player_ready[i] = 0;

	invoiceID = 0;
	for (int i = 0; i < hand_size; i++) {
		for (int j = 0; j < bet->maxplayers; j++) {
			card_matrix[j][i] = 0;
			card_values[j][i] = -1;
		}
	}

	for (int i = 0; i < bet->range; i++) {
		permis_d[i] = dcv_info.permis[i];
	}

	srand(time(0));
	dealerPosition = rand() % bet->maxplayers;
	dcv_vars->dealer = dealerPosition;

	while (bet->pullsock >= 0 && bet->pubsock >= 0) {
		ptr = 0;
		if ((recvlen = nn_recv(bet->pullsock, &ptr, NN_MSG, 0)) > 0) {
			char *tmp = clonestr(ptr);
			argjson = cJSON_CreateObject();
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				pthread_t server_thrd;
				if (OS_thread_create(&server_thrd, NULL, (void *)bet_dcv_backend_thrd,
						     (void *)cJSON_Print(argjson)) != 0) {
					dlg_error("Error in launching the bet_cashier_backend_thrd");
					exit(-1);
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

int32_t bet_dcv_bvv_backend(cJSON *argjson, struct dcv_bvv_sock_info *bet_dcv_bvv)
{
	char *method = NULL;
	int32_t retval = OK;

	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("Received method from BVV :: %s", method);
		if (strcmp(method, "bvv_ready") == 0) {
			retval = bet_dcv_start(bet_dcv, 0);
		} else if (strcmp(method, "bvv_join") == 0) {
			retval = bet_dcv_bvv_join(argjson, bet_dcv_bvv, dcv_vars);
		} else if (strcmp(method, "init_b") == 0) {
			retval = bet_relay(argjson, bet_dcv, dcv_vars);
		} else if (strcmp(method, "dcv_state") == 0) {
			retval = bet_get_dcv_state_for_bvv(argjson, bet_dcv);
		} else if (strcmp(method, "live") == 0) {
			cJSON *live_info = cJSON_CreateObject();
			cJSON_AddStringToObject(live_info, "method", "live");
			cJSON_AddStringToObject(live_info, "id", jstr(argjson, "id"));
			retval = (nn_send(bet_dcv_bvv->pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)),
					  0) < 0) ?
					 ERR_NNG_SEND :
					 OK;
		}
	}
	if (retval != OK) {
		dlg_error("Delaer encountered the error::%s with BVV", bet_err_str(retval));
	}
	return retval;
}

void bet_dcv_bvv_backend_loop(void *_ptr)
{
	int32_t recvlen = 0;
	void *ptr = NULL;
	cJSON *msgjson = NULL;
	struct dcv_bvv_sock_info *bet_dcv_bvv = _ptr;

	while (bet_dcv_bvv->pubsock >= 0 && bet_dcv_bvv->pullsock >= 0) {
		ptr = 0;
		char *tmp = NULL;
		recvlen = nn_recv(bet_dcv_bvv->pullsock, &ptr, NN_MSG, 0);
		if (recvlen > 0)
			tmp = clonestr(ptr);
		if ((recvlen > 0) && ((msgjson = cJSON_Parse(tmp)) != 0)) {
			if (bet_dcv_bvv_backend(msgjson, bet_dcv_bvv) < 0) {
				// This error case scenario needs to be handled...
				dlg_error("Some error occured during the communication between dealer and BVV");
				break;
			}

			if (tmp)
				free(tmp);
			if (ptr)
				nn_freemsg(ptr);
		}
	}
}
void bet_dcv_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:1234");

	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
	dcv_info.port = gui_ws_port;
	dcv_info.mounts = &mount;
	dcv_info.protocols = protocols;
	dcv_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	dcv_context = lws_create_context(&dcv_info);
	if (!dcv_context) {
		lwsl_err("lws init failed");
		dlg_error("lws_context error");
	}
	while (n >= 0 && !interrupted) {
		n = lws_service(dcv_context, 1000);
	}
}
