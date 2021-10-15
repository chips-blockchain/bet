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
double dev_fund_percentage = 0.25;

int32_t req_id_to_player_id_mapping[CARDS777_MAXPLAYERS];
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

	return 0;
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
	cJSON *deck_init_info = NULL, *cjson_cardprods = NULL, *cjson_dcv_blind_cards = NULL, *cjsong_hash = NULL,
	      *cjson_peer_pub_keys = NULL;
	char str[65] = { 0 }, *rendered = NULL;
	int32_t bytes, retval = 1;

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
	rendered = cJSON_Print(deck_init_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_dcv_init(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t peerid, retval = 1;
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

static int32_t bet_dcv_bvv_join(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, bytes;
	cJSON *config_info = NULL;
	char *rendered = NULL;

	config_info = cJSON_CreateObject();
	cJSON_AddStringToObject(config_info, "method", "config_data");
	cJSON_AddNumberToObject(config_info, "max_players", max_players);
	cJSON_AddNumberToObject(config_info, "table_stack_in_chips", table_stack_in_chips);
	cJSON_AddNumberToObject(config_info, "chips_tx_fee", chips_tx_fee);

	rendered = cJSON_Print(config_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;
	return retval;
}

int32_t bet_dcv_start(struct privatebet_info *bet, int32_t peerid)
{
	int32_t bytes, retval = 1;
	cJSON *init = NULL;
	char *rendered = NULL;

	init = cJSON_CreateObject();
	cJSON_AddStringToObject(init, "method", "init");
	cJSON_AddNumberToObject(init, "peerid", peerid);

	rendered = cJSON_Print(init);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

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
	cJSON *player_info = NULL;
	uint32_t bytes, retval = 1;
	char *rendered = NULL, *uri = NULL, *type = NULL;

	bet->numplayers = ++players_joined;
	dcv_info.peerpubkeys[jint(argjson, "gui_playerID")] = jbits256(argjson, "pubkey");
	strcpy((char *)dcv_info.uri[jint(argjson, "gui_playerID")], jstr(argjson, "uri"));

	uri = (char *)malloc(ln_uri_length * sizeof(char));
	type = ln_get_uri(&uri);

	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, "method", "join_res");

	cJSON_AddNumberToObject(player_info, "playerid", jint(argjson, "gui_playerID"));
	jaddbits256(player_info, "pubkey", jbits256(argjson, "pubkey"));
	cJSON_AddStringToObject(player_info, "uri", uri);
	cJSON_AddStringToObject(player_info, "type", type);
	cJSON_AddNumberToObject(player_info, "dealer", dealerPosition);
	cJSON_AddNumberToObject(player_info, "seat_taken", 0);
	cJSON_AddStringToObject(player_info, "req_identifier", jstr(argjson, "req_identifier"));

	player_seats_info[jint(argjson, "gui_playerID")].empty = 0;
	player_seats_info[jint(argjson, "gui_playerID")].chips =
		vars->funds[req_id_to_player_id_mapping[jint(argjson, "gui_playerID")]];

	cJSON *seats_info = NULL;

	dlg_info("bet->maxplayers::%d\n", bet->maxplayers);
	seats_info = bet_get_seats_json(bet->maxplayers);
	cJSON_AddItemToObject(player_info, "seats", seats_info);

	rendered = cJSON_Print(player_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed\n");
		goto end;
	}
end:
	if (uri)
		free(uri);
	return retval;
}

static int32_t bet_send_turn_info(struct privatebet_info *bet, int32_t playerid, int32_t cardid, int32_t card_type)
{
	cJSON *turn_info = NULL;
	int retval = 1, bytes;
	char *rendered = NULL;

	dlg_info("cardid::%d", cardid);
	while(cardid > 0) {
		
	}
	turn_info = cJSON_CreateObject();
	cJSON_AddStringToObject(turn_info, "method", "turn");
	cJSON_AddNumberToObject(turn_info, "playerid", playerid);
	cJSON_AddNumberToObject(turn_info, "cardid", cardid);
	cJSON_AddNumberToObject(turn_info, "card_type", card_type);
	rendered = cJSON_Print(turn_info);
	dlg_info("%s",cJSON_Print(turn_info));
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

int32_t bet_dcv_turn(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1;

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
		retval = 2;
end:
	return retval;
}

int32_t bet_relay(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1, bytes;
	char *rendered = NULL;

	rendered = cJSON_Print(argjson);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed\n");
		goto end;
	}
end:
	return retval;
}

static int32_t bet_check_bvv_ready(struct privatebet_info *bet)
{
	int32_t bytes, retval = 1;
	char *rendered = NULL;
	cJSON *bvv_ready = NULL, *uri_info = NULL;

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "check_bvv_ready");
	cJSON_AddItemToObject(bvv_ready, "uri_info", uri_info = cJSON_CreateArray());
	for (int i = 0; i < bet->maxplayers; i++) {
		jaddistr(uri_info, (char *)dcv_info.uri[i]);
	}
	rendered = cJSON_Print(bvv_ready);
	dlg_info("%s\n", cJSON_Print(bvv_ready));
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed\n");
	}

	return retval;
}

static int32_t bet_create_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, bytes, retval = 1;
	char **argv = NULL, *rendered = NULL;
	char hexstr[65] = { 0 };
	cJSON *invoice_info = NULL, *invoice = NULL;

	argc = 5;
	argv = (char **)malloc(argc * sizeof(char *));
	invoiceID++;
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(sizeof(char) * 1000);
	}
	dcv_info.betamount += jint(argjson, "betAmount");

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "invoice");
	sprintf(argv[2], "%ld", (long int)jint(argjson, "betAmount")); //sg777 mchips_msatoshichips
	sprintf(argv[3], "%s_%d_%d_%d_%d", bits256_str(hexstr, dcv_info.deckid), invoiceID, jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "betAmount"));
	sprintf(argv[4], "\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"", jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "betAmount"));

	invoice = cJSON_CreateObject();
	make_command(argc, argv, &invoice);

	if (jint(invoice, "code") != 0)
		retval = -1;
	else {
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "invoice");
		cJSON_AddNumberToObject(invoice_info, "playerID", jint(argjson, "playerID"));
		cJSON_AddNumberToObject(invoice_info, "round", jint(argjson, "round"));
		cJSON_AddStringToObject(invoice_info, "label", argv[3]);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(invoice));

		rendered = cJSON_Print(invoice_info);
		bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

		if (bytes < 0)
			retval = -1;
	}

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}

	return retval;
}

static int32_t bet_create_betting_invoice(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, bytes, retval = 1;
	char **argv = NULL, *rendered = NULL;
	char hexstr[65] = { 0 };
	cJSON *invoice_info = NULL, *invoice = NULL;

	argc = 5;
	argv = (char **)malloc(argc * sizeof(char *));
	invoiceID++;
	for (int i = 0; i < argc; i++) {
		argv[i] = (char *)malloc(sizeof(char) * 1000);
	}
	dcv_info.betamount += jint(argjson, "betAmount");

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "invoice");
	sprintf(argv[2], "%ld", (long int)jint(argjson, "invoice_amount") * mchips_msatoshichips);
	sprintf(argv[3], "%s_%d_%d_%d_%d", bits256_str(hexstr, dcv_info.deckid), invoiceID, jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "invoice_amount"));
	sprintf(argv[4], "\"Invoice_details_playerID:%d,round:%d,betting Amount:%d\"", jint(argjson, "playerID"),
		jint(argjson, "round"), jint(argjson, "invoice_amount"));

	invoice = cJSON_CreateObject();
	make_command(argc, argv, &invoice);

	if (jint(invoice, "code") != 0) {
		dlg_error("Failed to create the chips-ln invoice::\n%s\n", cJSON_Print(argjson));
		retval = -1;
	} else {
		invoice_info = cJSON_CreateObject();
		cJSON_AddStringToObject(invoice_info, "method", "bettingInvoice");
		cJSON_AddNumberToObject(invoice_info, "playerID", jint(argjson, "playerID"));
		cJSON_AddNumberToObject(invoice_info, "round", jint(argjson, "round"));
		cJSON_AddStringToObject(invoice_info, "label", argv[3]);
		cJSON_AddStringToObject(invoice_info, "invoice", cJSON_Print(invoice));
		cJSON_AddItemToObject(invoice_info, "actionResponse", cJSON_GetObjectItem(argjson, "actionResponse"));

		rendered = cJSON_Print(invoice_info);
		bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

		if (bytes < 0)
			retval = -1;
		dlg_error("nn_send failed\n");
	}

	if (argv) {
		for (int i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}

int32_t bet_check_player_ready(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int flag = 1;
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
	int retval = 1, playerid, cardid, card_type, flag = 1;

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
	char *sql_query = calloc(1, arg_size);
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
	return game_info;
}

static int32_t bet_dcv_poker_winner(struct privatebet_info *bet, struct privatebet_vars *vars, int winners[], int pot)
{
	int32_t no_of_winners = 0, retval = 1, bytes;
	double dcv_commission = 0, dev_commission = 0, winning_pot = 0, chips_conversion_factor = 0.001,
	       amount_in_txs = 0.0, player_amounts[bet->maxplayers], pot_in_chips = 0.0;
	cJSON *payout_info = NULL, *dev_info = NULL, *dcv_info = NULL, *payout_tx_info = NULL, *data_info = NULL;
	char *hex_str = NULL;
	for (int i = 0; i < bet->maxplayers; i++) {
		if (winners[i] == 1)
			no_of_winners++;
	}

	for (int32_t i = 0; i < no_of_txs; i++) {
		amount_in_txs += chips_get_balance_on_address_from_tx(legacy_m_of_n_msig_addr, tx_ids[i]);
	}

	pot_in_chips = pot * chips_conversion_factor;
	amount_in_txs = amount_in_txs - pot_in_chips;

	if (pot_in_chips > chips_tx_fee) {
		pot_in_chips = pot_in_chips - chips_tx_fee;
	} else {
		amount_in_txs = amount_in_txs - chips_tx_fee;
	}

	amount_in_txs = amount_in_txs / bet->numplayers;

	dcv_commission = ((dcv_commission_percentage * pot_in_chips) / 100);
	dev_commission = ((dev_fund_percentage * pot_in_chips) / 100);
	winning_pot = pot_in_chips - (dcv_commission + dev_commission);
	winning_pot = winning_pot / no_of_winners;

	payout_info = cJSON_CreateArray();

	dev_info = cJSON_CreateObject();
	dcv_info = cJSON_CreateObject();

	cJSON_AddStringToObject(dcv_info, "address", chips_get_wallet_address());
	cJSON_AddNumberToObject(dcv_info, "amount", dcv_commission);

	cJSON_AddStringToObject(dev_info, "address", dev_fund_addr);
	cJSON_AddNumberToObject(dev_info, "amount", dev_commission);

	cJSON_AddItemToArray(payout_info, dev_info);
	cJSON_AddItemToArray(payout_info, dcv_info);

	for (int32_t i = 0; i < bet->maxplayers; i++) {
		player_amounts[i] = amount_in_txs;
		if (winners[i] == 1)
			player_amounts[i] += winning_pot;
	}

	for (int32_t i = 0; i < bet->maxplayers; i++) {
		cJSON *temp = cJSON_CreateObject();
		if (player_amounts[i] > 0) {
			cJSON_AddStringToObject(
				temp, "address",
				vars->player_chips_addrs[req_id_to_player_id_mapping[i]]); //req_id_to_player_id_mapping[i]
			cJSON_AddNumberToObject(temp, "amount", player_amounts[i]);
			cJSON_AddItemToArray(payout_info, temp);
		}
	}
	data_info = payout_tx_data_info(bet, vars);
	hex_str = calloc(1, 2 * tx_data_size);
	str_to_hexstr(cJSON_Print(data_info), hex_str);

	payout_tx_info = chips_create_payout_tx(payout_info, no_of_txs, tx_ids, hex_str);
	dlg_info("payout_tx_info::%s\n", cJSON_Print(payout_tx_info));
	bytes = nn_send(bet->pubsock, cJSON_Print(payout_tx_info), strlen(cJSON_Print(payout_tx_info)), 0);
	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed\n");
	}
	if (hex_str)
		free(hex_str);
	return retval;
}

int32_t bet_evaluate_hand(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int retval = 1, max_score = 0, no_of_winners = 0, bytes;
	unsigned char h[7];
	unsigned long scores[CARDS777_MAXPLAYERS];
	int p[CARDS777_MAXPLAYERS];
	int winners[CARDS777_MAXPLAYERS], players_left = 0, only_winner = -1;
	cJSON *reset_info = NULL;
	char *rendered = NULL;
	int fold_flag = 0;
	cJSON *final_info = NULL, *all_hole_card_info = NULL, *board_card_info = NULL, *hole_card_info = NULL,
	      *show_info = NULL;
	char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };

	bet_game_info(bet, vars);

	for (int i = 0; i < bet->maxplayers; i++) {
		p[i] = vars->bet_actions[i][(vars->round)];

		if (vars->bet_actions[i][vars->round] == fold) //|| (vars->bet_actions[i][vars->round]==allin))
			players_left++;
		else
			only_winner = i;
	}

	players_left = bet->maxplayers - players_left;
	if (players_left < 2) {
		if (only_winner != -1) {
			fold_flag = 1;
			no_of_winners = 1;
			for (int i = 0; i < bet->maxplayers; i++) {
				if (vars->bet_actions[i][vars->round] == fold)
					winners[i] = 0;
				else
					winners[i] = 1;
			}
			//retval = bet_dcv_invoice_pay(bet, vars, only_winner, vars->pot);
			retval = bet_dcv_poker_winner(bet, vars, winners, vars->pot);
		}
	} else {
		for (int i = 0; i < bet->maxplayers; i++) {
			if (p[i] == fold)
				scores[i] = 0;
			else {
				for (int j = 0; j < hand_size; j++) {
					h[j] = (unsigned char)card_values[i][j];
				}
				scores[i] = seven_card_draw_score(h);
			}
		}
		for (int i = 0; i < bet->maxplayers; i++) {
			if (max_score < scores[i])
				max_score = scores[i];
		}
		for (int i = 0; i < bet->maxplayers; i++) {
			if (scores[i] == max_score) {
				winners[i] = 1;
				no_of_winners++;
			} else
				winners[i] = 0;
		}

		dlg_info("Winning Amount:%d", (vars->pot / no_of_winners));
		dlg_info("Winning Players Are:");
		for (int i = 0; i < bet->maxplayers; i++) {
			if (winners[i] == 1) {
				//retval = bet_dcv_invoice_pay(bet, vars, i, (vars->pot / no_of_winners));
				retval = bet_dcv_poker_winner(bet, vars, winners, vars->pot);
				dlg_info("%d\t", i);
				if (retval == -1)
					goto end;
			}
		}
	}

	final_info = cJSON_CreateObject();
	cJSON_AddStringToObject(final_info, "method", "finalInfo");
	all_hole_card_info = cJSON_CreateArray();
	board_card_info = cJSON_CreateArray();
	for (int i = 0; i < bet->maxplayers; i++) {
		hole_card_info = cJSON_CreateArray();
		for (int j = 0; j < no_of_hole_cards; j++) {
			if (fold_flag == 1) {
				cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
			} else {
				if (card_values[i][j] != -1)
					cJSON_AddItemToArray(hole_card_info,
							     cJSON_CreateString(cards[card_values[i][j]]));
				else
					cJSON_AddItemToArray(hole_card_info, cJSON_CreateNull());
			}
		}
		cJSON_AddItemToArray(all_hole_card_info, hole_card_info);
	}

	for (int j = no_of_hole_cards; j < hand_size; j++) {
		if (fold_flag == 1) {
			cJSON_AddItemToArray(board_card_info, cJSON_CreateNull());
		} else {
			if (card_values[0][j] != -1)
				cJSON_AddItemToArray(board_card_info, cJSON_CreateString(cards[card_values[0][j]]));
			else
				cJSON_AddItemToArray(board_card_info, cJSON_CreateNull());
		}
	}

	show_info = cJSON_CreateObject();
	cJSON_AddItemToObject(show_info, "allHoleCardsInfo", all_hole_card_info);
	cJSON_AddItemToObject(show_info, "boardCardInfo", board_card_info);

	cJSON_AddItemToObject(final_info, "showInfo", show_info);
	cJSON_AddNumberToObject(final_info, "win_amount", vars->pot / no_of_winners);

	cJSON *winnersInfo = cJSON_CreateArray();
	for (int i = 0; i < bet->maxplayers; i++) {
		if (winners[i] == 1) {
			cJSON_AddItemToArray(winnersInfo, cJSON_CreateNumber(i));
		}
	}
	cJSON_AddItemToObject(final_info, "winners", winnersInfo);

	dlg_info("Final Info :: %s\n", cJSON_Print(final_info));
	rendered = cJSON_Print(final_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		dlg_error("nn_send failed\n");
		goto end;
	}

	sleep(5);
	if (wsi_global_host) {
		lws_write(wsi_global_host, (unsigned char *)cJSON_Print(final_info), strlen(cJSON_Print(final_info)),
			  0);
	}
end:
	if (retval != -1) {
		reset_info = cJSON_CreateObject();
		cJSON_AddStringToObject(reset_info, "method", "reset");
		rendered = cJSON_Print(reset_info);
		find_bvv();
		bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			retval = -1;
			dlg_error("nn_send failed\n");
		}
		bet_dcv_reset(bet, vars);
	}
	return retval;
}

int32_t bet_ln_check(struct privatebet_info *bet)
{
	char channel_id[100];
	int32_t retval = 1, channel_state;
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
	retval = 1;
end:
	return retval;
}

static int32_t bet_award_winner(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int argc, retval = 1;
	char **argv = NULL, channel_id[100], *invoice = NULL;
	cJSON *pay_response = NULL, *invoice_info = NULL, *fund_channel_info = NULL;

	argc = 5;
	argv = (char **)malloc(sizeof(char *) * argc);
	for (int32_t i = 0; i < argc; i++)
		argv[i] = (char *)malloc(1000 * sizeof(char));
	strcpy(channel_id, strtok((char *)dcv_info.uri[jint(argjson, "playerid")], "@"));
	if (ln_get_channel_status(channel_id) != CHANNELD_NORMAL) {
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "fundchannel");
		strcpy(argv[2], channel_id);
		strcpy(argv[3], "500000");
		argv[4] = NULL;
		argc = 4;

		fund_channel_info = cJSON_CreateObject();
		make_command(argc, argv, &fund_channel_info);

		if (jint(fund_channel_info, "code") != 0) {
			retval = -1;
			dlg_error("LN Error ::%s", jstr(fund_channel_info, "message"));
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

	for (int32_t i = 0; i < argc; i++)
		memset(&argv[i], 0, sizeof(argv[i]));

	argc = 3;
	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "pay");
	sprintf(argv[2], "%s", jstr(invoice_info, "bolt11"));
	argv[3] = NULL;

	pay_response = cJSON_CreateObject();
	make_command(argc, argv, &pay_response);

	if (jint(pay_response, "code") != 0) {
		retval = -1;
		dlg_info("LN Error :: %s", jstr(pay_response, "message"));
		goto end;
	}

	if (strcmp(jstr(pay_response, "status"), "complete") == 0) {
		dlg_info("Payment Success");
	}

end:
	if (argv) {
		for (int i = 0; i < 5; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
	return retval;
}
static void bet_push_joinInfo(cJSON *argjson, int32_t numplayers)
{
	cJSON *join_info = NULL;

	join_info = cJSON_CreateObject();
	cJSON_AddStringToObject(join_info, "method", "join_info");
	cJSON_AddNumberToObject(join_info, "joined_playerid", jint(argjson, "gui_playerID"));
	cJSON_AddNumberToObject(join_info, "tot_players_joined", numplayers);
	//bet_push_dcv_to_gui(join_info);
	bet_dcv_lws_write(join_info);
}

static int32_t bet_dcv_stack_info_resp(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t bytes, retval = 1;
	cJSON *stack_info_resp = NULL;
	cJSON *msig_addr_nodes = NULL;

	stack_info_resp = cJSON_CreateObject();
	cJSON_AddStringToObject(stack_info_resp, "method", "stack_info_resp");
	cJSON_AddStringToObject(stack_info_resp, "id", jstr(argjson, "id"));
	/*
	pthread_mutex_lock(&mutex);
	strcpy(vars->player_chips_addrs[no_of_rand_str], jstr(argjson, "chips_addr"));
	strcpy(tx_rand_str[no_of_rand_str++], jstr(argjson, "req_identifier"));
	pthread_mutex_unlock(&mutex);
	*/
	cJSON_AddNumberToObject(stack_info_resp, "max_players", max_players);
	cJSON_AddNumberToObject(stack_info_resp, "table_stack_in_chips", table_stack_in_chips);
	cJSON_AddNumberToObject(stack_info_resp, "chips_tx_fee", chips_tx_fee);
	cJSON_AddStringToObject(stack_info_resp, "legacy_m_of_n_msig_addr", legacy_m_of_n_msig_addr);
	cJSON_AddStringToObject(stack_info_resp, "table_id", table_id);
	cJSON_AddNumberToObject(stack_info_resp, "threshold_value", threshold_value);
	msig_addr_nodes = cJSON_CreateArray();
	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			cJSON_AddItemToArray(msig_addr_nodes, cJSON_CreateString(notary_node_ips[i]));
		}
	}
	cJSON_AddItemToObject(stack_info_resp, "msig_addr_nodes", msig_addr_nodes);
	dlg_info("stack info resp::%s", cJSON_Print(stack_info_resp));
	bytes = nn_send(bet->pubsock, cJSON_Print(stack_info_resp), strlen(cJSON_Print(stack_info_resp)), 0);
	if (bytes < 0)
		retval = -1;

	return retval;
}

static void bet_dcv_process_signed_raw_tx(cJSON *argjson)
{
	cJSON *raw_tx = NULL;

	no_of_signers++;
	if (no_of_signers < max_no_of_signers) {
		is_signed[jint(argjson, "playerid")] = 1;
		chips_publish_multisig_tx(jstr(argjson, "tx"));
	} else {
		raw_tx = cJSON_CreateObject();
		cJSON_AddStringToObject(raw_tx, "hex", jstr(argjson, "tx"));
		chips_send_raw_tx(raw_tx);
	}
}

static int32_t bet_dcv_verify_rand_str(char *rand_str)
{
	int32_t retval = 0;
	for (int i = 0; i < no_of_rand_str; i++) {
		if (strcmp(tx_rand_str[i], rand_str) == 0) {
			retval = 1;
			break;
		}
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
}

static int32_t bet_dcv_verify_tx(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *tx_info = NULL;
	int32_t block_height, retval = 0;
	char *hex_data = NULL, *data = NULL;
	cJSON *data_info = NULL;

	tx_info = cJSON_CreateObject();
	tx_info = cJSON_GetObjectItem(argjson, "tx_info");
	if (tx_info == NULL)
		return retval;

	block_height = jint(argjson, "block_height");
	while (chips_get_block_count() < block_height) {
		sleep(1);
	}

	if (chips_check_if_tx_unspent(cJSON_Print(tx_info)) == 1) {
		hex_data = calloc(1, tx_data_size * 2);
		chips_extract_data(cJSON_Print(tx_info), &hex_data);
		data = calloc(1, tx_data_size);
		hexstr_to_str(hex_data, data);
		data_info = cJSON_CreateObject();
		data_info = cJSON_Parse(data);
		if (strcmp(table_id, jstr(data_info, "table_id")) == 0) {
			if (strcmp(jstr(argjson, "id"), jstr(data_info, "player_id")) == 0) {
				retval = 1;
				pthread_mutex_lock(&mutex);
				if (no_of_txs == bet->maxplayers) {
					bet_send_tx_reverse_rqst(argjson, bet);
					retval = 2;
				} else {
					strcpy(tx_ids[no_of_txs++], unstringify(cJSON_Print(tx_info)));
					if (no_of_txs == bet_dcv->maxplayers)
						dcv_state = 1;
				}
				pthread_mutex_unlock(&mutex);
			}
		}
	}
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
		player_seats_info[i].empty = 1;
		player_seats_info[i].playing = 0;
	}
}

static int32_t bet_dcv_check_pos_status(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *join_res = NULL;
	char *rendered = NULL;
	int32_t bytes, pos_status = 0, gui_playerID;

	gui_playerID = jint(argjson, "gui_playerID");

	if (player_seats_info[gui_playerID].empty == 0)
		pos_status = 1;

	if (pos_status == 1) {
		dlg_warn("Seat Taken\n");
		join_res = cJSON_CreateObject();
		cJSON_AddStringToObject(join_res, "method", "join_res");
		cJSON_AddNumberToObject(join_res, "playerid", gui_playerID);
		cJSON_AddNumberToObject(join_res, "seat_taken", pos_status);
		cJSON_AddStringToObject(join_res, "req_identifier", jstr(argjson, "req_identifier"));
		rendered = cJSON_Print(join_res);
		bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0) {
			dlg_error("nn_send failed");
		}
	}
	return pos_status;
}

static int32_t bet_dcv_process_join_req(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	int32_t retval = 1;

	if (1 == bet_dcv_check_pos_status(argjson, bet))
		return retval; // the seat is already taken just inform this to player.

	if (bet->numplayers < bet->maxplayers) {
		char *req_id = jstr(argjson, "req_identifier");
		for (int32_t i = 0; i < no_of_rand_str; i++) {
			if (strcmp(tx_rand_str[i], req_id) == 0) {
				req_id_to_player_id_mapping[jint(argjson, "gui_playerID")] = i;
				break;
			}
		}
		retval = bet_player_join_req(argjson, bet, vars);
		if (retval < 0)
			return retval;

		bet_push_joinInfo(argjson, bet->numplayers);

		if (bet->numplayers == bet->maxplayers) {
			heartbeat_on = 1;
			for (int32_t i = 0; i < bet->maxplayers; i++) {
				dlg_info("%d::%s\n", req_id_to_player_id_mapping[i], vars->player_chips_addrs[i]);
			}
			retval = bet_ln_check(bet);
			if (retval < 0) {
				dlg_error("Issue in establishing the LN channels");
				return retval;
			}
			retval = bet_check_bvv_ready(bet);
		}
	}
	return retval;
}

static int32_t bet_dcv_process_tx(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, char *addr)
{
	int32_t funds = 0, bytes, retval;
	cJSON *tx_status = NULL;
	char *sql_stmt = NULL;
	cJSON *msig_addr_nodes = NULL;

	retval = bet_dcv_verify_tx(argjson, bet);
	if (retval == 2) {
		return retval;
	}

	if (retval == 1) {
		pthread_mutex_lock(&mutex);
		strcpy(vars->player_chips_addrs[no_of_rand_str], jstr(argjson, "chips_addr"));
		strcpy(tx_rand_str[no_of_rand_str++], jstr(argjson, "id"));
		pthread_mutex_unlock(&mutex);

		double balance = chips_get_balance_on_address_from_tx(addr, jstr(argjson, "tx_info"));
		funds = (balance * satoshis) / (satoshis_per_unit * normalization_factor);

		char *rand_str = jstr(argjson, "id");
		for (int i = 0; i < no_of_rand_str; i++) {
			if (strcmp(tx_rand_str[i], rand_str) == 0)
				vars->funds[i] = funds;
		}
		sql_stmt = calloc(1, sql_query_size);

		msig_addr_nodes = cJSON_CreateArray();
		for (int32_t i = 0; i < no_of_notaries; i++) {
			if (notary_status[i] == 1) {
				cJSON_AddItemToArray(msig_addr_nodes, cJSON_CreateString(notary_node_ips[i]));
			}
		}

		sprintf(sql_stmt, "INSERT INTO dcv_tx_mapping values(\'%s\',\'%s\',\'%s\',\'%s\',%d,%d);",
			jstr(argjson, "tx_info"), table_id, rand_str, cJSON_Print(msig_addr_nodes), tx_unspent,
			threshold_value);
		int32_t ret = bet_run_query(sql_stmt);
		if (ret != 0)
			dlg_error("Error in fetching the results of the sql query::%s\n", sql_stmt);
	}

	tx_status = cJSON_CreateObject();
	cJSON_AddStringToObject(tx_status, "method", "tx_status");
	cJSON_AddStringToObject(tx_status, "id", jstr(argjson, "id"));
	cJSON_AddNumberToObject(tx_status, "tx_validity", retval);
	cJSON_AddNumberToObject(tx_status, "player_funds", funds);
	bytes = nn_send(bet->pubsock, cJSON_Print(tx_status), strlen(cJSON_Print(tx_status)), 0);

	if (sql_stmt)
		free(sql_stmt);
	return retval;
}

static void bet_get_dcv_state(cJSON *argjson, struct privatebet_info *bet)
{
	cJSON *dcv_state_info = NULL;
	int32_t bytes;

	dcv_state_info = cJSON_CreateObject();
	cJSON_AddStringToObject(dcv_state_info, "method", "dcv_state");
	cJSON_AddNumberToObject(dcv_state_info, "dcv_state", dcv_state);
	cJSON_AddNumberToObject(dcv_state_info, "players_joined", players_joined);
	cJSON_AddNumberToObject(dcv_state_info, "max_players", bet->maxplayers);
	cJSON_AddStringToObject(dcv_state_info, "id", jstr(argjson, "id"));
	bytes = nn_send(bet->pubsock, cJSON_Print(dcv_state_info), strlen(cJSON_Print(dcv_state_info)), 0);
}

void bet_dcv_backend_thrd(void *_ptr)
{
	struct privatebet_info *bet = bet_dcv;
	char *method = NULL;
	int32_t bytes, retval = 1;
	cJSON *argjson = NULL;
	struct privatebet_vars *vars = dcv_vars;

	argjson = cJSON_Parse(_ptr);
	if ((method = jstr(argjson, "method")) != 0) {
		dlg_info("%s", method);
		if (strcmp(method, "join_req") == 0) {
			retval = bet_dcv_process_join_req(argjson, bet, vars);
		} else if (strcmp(method, "bvv_ready") == 0) {
			retval = bet_dcv_start(bet, 0);
		} else if (strcmp(method, "init_p") == 0) {
			retval = bet_dcv_init(argjson, bet, vars);
			if (dcv_info.numplayers == dcv_info.maxplayers) {
				retval = bet_dcv_deck_init_info(argjson, bet, vars);
			}
		} else if (strcmp(method, "bvv_join") == 0) {
			retval = bet_dcv_bvv_join(argjson, bet, vars);
		} else if ((strcmp(method, "init_b") == 0) || (strcmp(method, "next_turn") == 0)) {
			if (strcmp(method, "init_b") == 0) {
				retval = bet_relay(argjson, bet, vars);
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
		    dlg_info("%s", cJSON_Print(argjson));
			bytes = nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
			if (bytes < 0) {
				retval = -1;
				dlg_error("nn_send failed");
			}
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_betting_statemachine(argjson, bet, vars);
		} else if (strcmp(method, "display_current_state") == 0) {
			retval = bet_display_current_state(argjson, bet, vars);
		} else if (strcmp(method, "stack") == 0) {
			vars->funds[jint(argjson, "playerid")] = jint(argjson, "stack_value");
			retval = bet_relay(argjson, bet, vars);
		} else if (strcmp(method, "signedrawtransaction") == 0) {
			bet_dcv_process_signed_raw_tx(argjson);
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
			bytes = nn_send(bet->pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
			if (bytes < 0) {
				retval = -1;
				dlg_error("nn_send failed");
			}
		} else if (strcmp(method, "dcv_state") == 0) {
			bet_get_dcv_state(argjson, bet);
		} else if (strcmp(method, "req_seats_info") == 0) {
			cJSON *seats_info_resp = NULL;
			cJSON *seats_info = bet_get_seats_json(bet->maxplayers);

			seats_info_resp = cJSON_CreateObject();
			cJSON_AddStringToObject(seats_info_resp, "method", "seats_info_resp");
			cJSON_AddStringToObject(seats_info_resp, "req_identifier", jstr(argjson, "req_identifier"));
			cJSON_AddItemToObject(seats_info_resp, "seats", seats_info);
			bytes = nn_send(bet->pubsock, cJSON_Print(seats_info_resp),
					strlen(cJSON_Print(seats_info_resp)), 0);
		} else if (strcmp(method, "player_active") == 0) {
			bet_dcv_update_player_status(argjson);
		} else if (strcmp(method, "share_info") == 0) {
			if (jint(argjson, "error") == -2) { //Decryption Error
				cJSON *game_abort = cJSON_CreateObject();
				cJSON_AddStringToObject(game_abort, "method", "game_abort");
				cJSON_AddStringToObject(
					game_abort, "message",
					"Player node failed to decrypt the share, so reversing the tx's and stopping the game...");

				bytes = nn_send(bet->pubsock, cJSON_Print(game_abort), strlen(cJSON_Print(game_abort)),
						0);
				if (bytes < 0) {
					retval = -1;
					dlg_error("nn_send failed");
				}
				exit(0);
			} else {
				bytes = nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
				if (bytes < 0) {
					retval = -1;
					dlg_error("nn_send failed");
				}
			}
		} else if (strcmp(method, "game_abort") == 0) {
			dlg_error("%s", jstr(argjson, "message"));
			bytes = nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
			if (bytes < 0) {
				retval = -1;
				dlg_error("nn_send failed");
			}
			exit(0);

		} else {
			bytes = nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
			if (bytes < 0) {
				retval = -1;
				dlg_error("nn_send failed");
			}
		}
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

void bet_dcv_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:1234");

	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
	dcv_info.port = 9000;
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
