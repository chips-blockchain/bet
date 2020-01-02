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

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>

#define LWS_PLUGIN_STATIC

struct lws *wsi_global_host = NULL;

cJSON *dcv_data_to_write = NULL;
int32_t dcv_data_exists = 0;

int32_t players_joined = 0;
int32_t turn = 0, no_of_cards = 0, no_of_rounds = 0, no_of_bets = 0;
int32_t card_matrix[CARDS777_MAXPLAYERS][hand_size];
int32_t card_values[CARDS777_MAXPLAYERS][hand_size];
int32_t all_player_cards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
struct deck_dcv_info dcv_info;
int32_t player_ready[CARDS777_MAXPLAYERS];
int32_t hole_cards_drawn = 0, community_cards_drawn = 0, flop_cards_drawn = 0, turn_card_drawn = 0,
	river_card_drawn = 0;
int32_t bet_amount[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS];
int32_t eval_game_p[CARDS777_MAXPLAYERS], eval_game_c[CARDS777_MAXPLAYERS];

int32_t player_status[CARDS777_MAXPLAYERS], bvv_status;
char player_chips_address[CARDS777_MAXPLAYERS][64];

int32_t invoiceID;

char *suit[NSUITS] = { "clubs", "diamonds", "hearts", "spades" };
char *face[NFACES] = { "two",  "three", "four", "five",	 "six",	 "seven", "eight",
		       "nine", "ten",	"jack", "queen", "king", "ace" };

struct privatebet_info *bet_dcv = NULL;
struct privatebet_vars *dcv_vars = NULL;

static int dealerPosition;
int32_t no_of_signers, max_no_of_signers = 2, is_signed[CARDS777_MAXPLAYERS];

char lws_buf[65536];
int32_t lws_buf_length = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void dcv_lws_write(cJSON *data)
{
	if (!dcv_data_to_write)
		dcv_data_to_write = cJSON_CreateObject();

	memset(dcv_data_to_write, 0, sizeof(struct cJSON));
	dcv_data_to_write = data;
	dcv_data_exists = 1;
	lws_callback_on_writable(wsi_global_host);
}

void bet_chat(struct lws *wsi, cJSON *argjson)
{
	cJSON *chat_info = NULL;

	chat_info = cJSON_CreateObject();
	cJSON_AddStringToObject(chat_info, "chat", jstr(argjson, "value"));
	lws_write(wsi, cJSON_Print(chat_info), strlen(cJSON_Print(chat_info)), 0);
}

static inline void initialize_seat(cJSON *seat_info, char *name, int32_t seat, int32_t stack, int32_t empty,
				   int32_t playing)
{
	cJSON_AddStringToObject(seat_info, "name", name);
	cJSON_AddNumberToObject(seat_info, "seat", seat);
	cJSON_AddNumberToObject(seat_info, "stack", stack);
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
	lws_write(wsi, rendered, strlen(rendered), 0);

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
	lws_write(wsi, rendered, strlen(rendered), 0);
	return 0;
}

int32_t bet_dcv_frontend(struct lws *wsi, cJSON *argjson)
{
	int retval = 1;
	char *rendered = NULL, *method = NULL;
	int32_t bytes = 0;

	method = jstr(argjson, "method");
	if (strcmp(method, "game") == 0) {
		retval = bet_game(wsi, argjson);
	} else if (strcmp(method, "seats") == 0) {
		retval = bet_seats(wsi, argjson);

	} else if (strcmp(method, "chat") == 0) {
		bet_chat(wsi, argjson);
	} else if (strcmp(method, "reset") == 0) {
		bet_dcv_force_reset(bet_dcv, dcv_vars);
		rendered = cJSON_Print(argjson);
		bytes = nn_send(bet_dcv->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0)
			retval = -1;
	} else {
		printf("%s::%d::Method::%s is not known to the system\n", __FUNCTION__, __LINE__, method);
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
			printf("\n%s:%d:Failed to process the host command", __FUNCTION__, __LINE__);
		}
		memset(lws_buf, 0x00, sizeof(lws_buf));
		lws_buf_length = 0;
		break;
	case LWS_CALLBACK_ESTABLISHED:
		wsi_global_host = wsi;
		printf("%s::%d::LWS_CALLBACK_ESTABLISHED\n", __FUNCTION__, __LINE__);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (dcv_data_exists == 1) {
			lws_write(wsi, cJSON_Print(dcv_data_to_write), strlen(cJSON_Print(dcv_data_to_write)), 0);
			dcv_data_exists = 0;
		}
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
		dcv_lws_write(argjson);
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
	int argc, retval = 1, state, buf_size = 100;
	char **argv = NULL, uri[100] = { 0 }, buf[100] = { 0 };
	cJSON *connect_info = NULL, *fund_channel_info = NULL;

	strcpy(uri, jstr(argjson, "uri"));
	strcpy(dcv_info.bvv_uri, uri);
	if ((ln_get_channel_status(strtok(jstr(argjson, "uri"), "@")) !=
	     3)) // 3 means channel is already established with the peer
	{
		argc = 5;
		argv = (char **)malloc(argc * sizeof(char *));
		if (argv == NULL) {
			printf("%s::%d::Memory allocation failed\n", __FUNCTION__, __LINE__);
			goto end;
		}
		memset(argv, 0x00, (argc * sizeof(char *)));
		for (int i = 0; i < argc; i++) {
			argv[i] = (char *)malloc(buf_size * sizeof(char));
			if (argv[i] == NULL) {
				printf("%s::%d::Memory allocation failed\n", __FUNCTION__, __LINE__);
				goto end;
			}
		}
		argc = 3;
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "connect");
		strcpy(argv[2], uri);
		connect_info = cJSON_CreateObject();
		make_command(argc, argv, &connect_info);
		cJSON_Print(connect_info);

		if (jint(connect_info, "code") != 0) {
			retval = -1;
			printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(connect_info, "message"));
			goto end;
		}

		argc = 5;
		for (int i = 0; i < argc; i++) {
			memset(argv[i], 0x00, buf_size);
		}
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "fundchannel");
		strcpy(argv[2], jstr(connect_info, "id"));
		sprintf(buf, "%d", channel_fund_satoshis);
		strcpy(argv[3], buf);
		argc = 4;
		fund_channel_info = cJSON_CreateObject();
		make_command(argc, argv, &fund_channel_info);

		cJSON_Print(fund_channel_info);
		if (jint(fund_channel_info, "code") != 0) {
			retval = -1;
			printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(fund_channel_info, "message"));
			goto end;
		}
		while ((state = ln_get_channel_status(jstr(connect_info, "id"))) != 3) {
			if (state == 2) {
				printf("CHANNELD_AWAITING_LOCKIN\r");
				fflush(stdout);
			} else if (state == 8) {
				printf("\nONCHAIN");
			} else
				printf("\n%s:%d:channel-state:%d\n", __FUNCTION__, __LINE__, state);

			sleep(2);
		}
		printf("DCV-->BVV LN Channel established\n");
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
int32_t bet_player_join_req(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *player_info = NULL, *get_info = NULL, *addresses = NULL, *address = NULL;
	uint32_t bytes, retval = 1;
	char *rendered = NULL, *uri = NULL;
	int argc;
	char **argv = NULL;

	bet->numplayers = ++players_joined;
	dcv_info.peerpubkeys[jint(argjson, "gui_playerID")] = jbits256(argjson, "pubkey");
	strcpy(dcv_info.uri[jint(argjson, "gui_playerID")], jstr(argjson, "uri"));

	argc = 2;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
		argv[i] = (char *)malloc(100 * sizeof(char));

	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "getinfo");
	get_info = cJSON_CreateObject();
	make_command(argc, argv, &get_info);

	uri = (char *)malloc(100 * sizeof(char));

	addresses = cJSON_GetObjectItem(get_info, "address");
	address = cJSON_GetArrayItem(addresses, 0);

	strcpy(uri, jstr(get_info, "id"));
	strcat(uri, "@");
	strcat(uri, jstr(address, "address"));

	player_info = cJSON_CreateObject();
	cJSON_AddStringToObject(player_info, "method", "join_res");

	cJSON_AddNumberToObject(player_info, "peerid", jint(argjson, "gui_playerID"));
	jaddbits256(player_info, "pubkey", jbits256(argjson, "pubkey"));
	cJSON_AddStringToObject(player_info, "uri", uri);
	cJSON_AddNumberToObject(player_info, "dealer", dealerPosition);

	rendered = cJSON_Print(player_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
		goto end;
	}
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

static int32_t bet_send_turn_info(struct privatebet_info *bet, int32_t playerid, int32_t cardid, int32_t card_type)
{
	cJSON *turn_info = NULL;
	int retval = 1, bytes;
	char *rendered = NULL;

	turn_info = cJSON_CreateObject();
	cJSON_AddStringToObject(turn_info, "method", "turn");
	cJSON_AddNumberToObject(turn_info, "playerid", playerid);
	cJSON_AddNumberToObject(turn_info, "cardid", cardid);
	cJSON_AddNumberToObject(turn_info, "card_type", card_type);
	rendered = cJSON_Print(turn_info);
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
		printf("\n%s :%d Failed to send data", __FUNCTION__, __LINE__);
		goto end;
	}
end:
	return retval;
}

static int32_t bet_check_bvv_ready(struct privatebet_info *bet)
{
	int32_t bytes, retval = -1;
	char *rendered = NULL;
	cJSON *bvv_ready = NULL, *uri_info = NULL;

	bvv_ready = cJSON_CreateObject();
	cJSON_AddStringToObject(bvv_ready, "method", "check_bvv_ready");
	cJSON_AddItemToObject(bvv_ready, "uri_info", uri_info = cJSON_CreateArray());
	for (int i = 0; i < bet->maxplayers; i++) {
		jaddistr(uri_info, dcv_info.uri[i]);
	}
	rendered = cJSON_Print(bvv_ready);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0)
		retval = -1;

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
	sprintf(argv[2], "%ld", (long int)jint(argjson, "betAmount") * mchips_msatoshichips);
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

	if (jint(invoice, "code") != 0)
		retval = -1;
	else {
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

void bet_dcv_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
{
	cJSON *reset_info = NULL;

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

	reset_info = cJSON_CreateObject();
	cJSON_AddStringToObject(reset_info, "method", "reset");
	bet_push_dcv_to_gui(reset_info);
}

void bet_dcv_force_reset(struct privatebet_info *bet, struct privatebet_vars *vars)
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

	bet->numplayers = 0;
	bet->cardid = -1;
	bet->turni = -1;
	bet->no_of_turns = 0;
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
			retval = bet_dcv_invoice_pay(bet, vars, only_winner, vars->pot);
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

		printf("\nWinning Amount:%d", (vars->pot / no_of_winners));
		printf("\nWinning Players Are:");
		for (int i = 0; i < bet->maxplayers; i++) {
			if (winners[i] == 1) {
				retval = bet_dcv_invoice_pay(bet, vars, i, (vars->pot / no_of_winners));
				printf("%d\t", i);
				if (retval == -1)
					goto end;
			}
		}
		printf("\n");
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

	printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(final_info));
	rendered = cJSON_Print(final_info);
	bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

	if (bytes < 0) {
		retval = -1;
		printf("%s::%d::Failed to send data\n", __FUNCTION__, __LINE__);
		goto end;
	}
	sleep(5);
	lws_write(wsi_global_host, cJSON_Print(final_info), strlen(cJSON_Print(final_info)), 0);
end:
	if (retval != -1) {
		reset_info = cJSON_CreateObject();
		cJSON_AddStringToObject(reset_info, "method", "reset");
		rendered = cJSON_Print(reset_info);
		bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
		if (bytes < 0)
			retval = -1;
		bet_dcv_reset(bet, vars);
	}
	return retval;
}

int32_t bet_ln_check(struct privatebet_info *bet)
{
	char channel_id[100], channel_state;
	int argc, retval = 1;
	char **argv = NULL, uri[100];
	cJSON *fund_channel_info = NULL, *connect_info = NULL;

	argc = 6;
	argv = (char **)malloc(argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
		argv[i] = (char *)malloc(100);
	strncpy(uri, dcv_info.bvv_uri, sizeof(uri));
	strncpy(channel_id, strtok(uri, "@"), sizeof(channel_id));
	channel_state = ln_get_channel_status(channel_id);
	if ((channel_state != 2) && (channel_state != 3)) {
		argc = 6;
		for (int i = 0; i < argc; i++)
			memset(argv[i], 0x00, sizeof(argv[i]));
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "connect");
		strcpy(argv[2], dcv_info.bvv_uri);
		argc = 3;

		connect_info = cJSON_CreateObject();
		make_command(argc, argv, &connect_info);

		argc = 6;
		for (int i = 0; i < argc; i++)
			memset(argv[i], 0x00, sizeof(argv[i]));
		strcpy(argv[0], "lightning-cli");
		strcpy(argv[1], "fundchannel");
		strcpy(argv[2], channel_id);
		strcpy(argv[3], "500000");
		argc = 4;

		fund_channel_info = cJSON_CreateObject();
		make_command(argc, argv, &fund_channel_info);

		if (jint(fund_channel_info, "code") == -1) {
			retval = -1;
			printf("\n%s:%d: Message: %s", __FUNCTION__, __LINE__, jstr(fund_channel_info, "message"));
			goto end;
		}
	}

	while ((channel_state = ln_get_channel_status(channel_id)) != 3) {
		if (channel_state == 2) {
			printf("CHANNELD AWAITING LOCKIN\r");
			fflush(stdout);
			sleep(2);
		} else {
			retval = -1;
			printf("\n%s:%d: DCV is failed to establish the channel with BVV", __FUNCTION__, __LINE__);
			goto end;
		}
	}
	printf("DCV-->BVV channel ready\n");

	for (int i = 0; i < bet_dcv->maxplayers; i++) {
		strcpy(uri, dcv_info.uri[i]);
		strcpy(channel_id, strtok(uri, "@"));

		while ((channel_state = ln_get_channel_status(channel_id)) != 3) {
			if (channel_state == 2) {
				printf("CHANNELD AWAITING LOCKIN\r");
				fflush(stdout);
				sleep(2);
			} else if ((channel_state != 2) && (channel_state != 3)) {
				retval = -1;
				printf("\n%s:%d: Player: %d is failed to establish the channel "
				       "with "
				       "DCV, channel_state=%d, JUST WAIT\n",
				       __FUNCTION__, __LINE__, i, channel_state);
				sleep(2);
				// break;
			}
		}

		printf("Player %d --> DCV channel ready\n", i);
	}
	retval = 1;
end:
	if (argv) {
		for (int i = 0; i < 6; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free(argv);
	}
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
	strcpy(channel_id, strtok(dcv_info.uri[jint(argjson, "playerid")], "@"));
	if (ln_get_channel_status(channel_id) != 3) {
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
			printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(fund_channel_info, "message"));
			goto end;
		}

		printf("\nFund channel response:%s\n", cJSON_Print(fund_channel_info));
		int state;
		while ((state = ln_get_channel_status(channel_id)) != 3) {
			if (state == 2) {
				printf("\nCHANNELD_AWAITING_LOCKIN");
			} else if (state == 8) {
				printf("\nONCHAIN");
			} else
				printf("\n%s:%d:channel-state:%d\n", __FUNCTION__, __LINE__, state);

			printf("%s::%d::%d\n", __FUNCTION__, __LINE__, state);
			sleep(10);
		}
		printf("%s::%d::%d\n", __FUNCTION__, __LINE__, state);
	}
	invoice = jstr(argjson, "invoice");
	invoice_info = cJSON_Parse(invoice);

	for (int32_t i = 0; i < argc; i++)
		memset(argv[i], 0, sizeof(argv[i]));

	argc = 3;
	strcpy(argv[0], "lightning-cli");
	strcpy(argv[1], "pay");
	sprintf(argv[2], "%s", jstr(invoice_info, "bolt11"));
	argv[3] = NULL;

	pay_response = cJSON_CreateObject();
	make_command(argc, argv, &pay_response);

	if (jint(pay_response, "code") != 0) {
		retval = -1;
		printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__, jstr(pay_response, "message"));
		goto end;
	}

	if (strcmp(jstr(pay_response, "status"), "complete") == 0) {
		printf("\nPayment Success\n");
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
	dcv_lws_write(join_info);
}
int32_t bet_dcv_backend(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)
{
	char *method;
	int32_t bytes, retval = 1;
	char *rendered = NULL;
	if ((method = jstr(argjson, "method")) != 0) {
		printf("%s::%d::%s\n", __FUNCTION__, __LINE__, method);
		if (strcmp(method, "join_req") == 0) {
			if (bet->numplayers < bet->maxplayers) {
				retval = bet_player_join_req(argjson, bet, vars);
				if (retval < 0)
					goto end;
				bet_push_joinInfo(argjson, bet->numplayers);
				if (bet->numplayers == bet->maxplayers) {
					printf("Table is filled\n");
					retval = bet_ln_check(bet);
					if (retval < 0) {
						printf("%s::%d::something wrong with bet_ln_check\n", __FUNCTION__,
						       __LINE__);
						goto end;
					}
					bet_check_bvv_ready(bet);
				}
			}

		} else if (strcmp(method, "bvv_ready") == 0) {
			retval = bet_dcv_start(bet, 0);
		} else if (strcmp(method, "init_p") == 0) {
			retval = bet_dcv_init(argjson, bet, vars);
			if (dcv_info.numplayers == dcv_info.maxplayers) {
				printf("%s::%d::dcv_info.numplayers::%d\n", __FUNCTION__, __LINE__,
				       dcv_info.numplayers);
				retval = bet_dcv_deck_init_info(argjson, bet, vars);
			}
		} else if (strcmp(method, "bvv_join") == 0) {
			retval = bet_dcv_bvv_join(argjson, bet, vars);
		} else if ((strcmp(method, "init_b") == 0) || (strcmp(method, "next_turn") == 0)) {
			if (strcmp(method, "init_b") == 0) {
				retval = bet_relay(argjson, bet, vars);
				if (retval < 0)
					goto end;
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
			rendered = cJSON_Print(argjson);
			for (int i = 0; i < 2; i++) {
				bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);
				if (bytes < 0) {
					retval = -1;
					printf("\nMehtod: %s Failed to send data", method);
					goto end;
				}
			}
		} else if (strcmp(method, "betting") == 0) {
			retval = bet_player_betting_statemachine(argjson, bet, vars);
		} else if (strcmp(method, "display_current_state") == 0) {
			retval = bet_display_current_state(argjson, bet, vars);
		} else if (strcmp(method, "stack") == 0) {
			vars->funds[jint(argjson, "playerid")] = jint(argjson, "stack_value");
			retval = bet_relay(argjson, bet, vars);
		} else if (strcmp(method, "signedrawtransaction") == 0) {
			printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));
			no_of_signers++;
			if (no_of_signers < max_no_of_signers) {
				is_signed[jint(argjson, "playerid")] = 1;
				chips_publish_multisig_tx(jstr(argjson, "tx"));
			} else {
				cJSON *temp = cJSON_CreateObject();
				cJSON_AddStringToObject(temp, "hex", jstr(argjson, "tx"));
				cJSON *finaltx = chips_send_raw_tx(temp);
				printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(finaltx));
			}

		} else if (strcmp(method, "live") == 0) {
			if (strcmp(jstr(argjson, "node_type"), "bvv") == 0)
				bvv_status = 1;
			else if (strcmp(jstr(argjson, "node_type"), "player") == 0)
				player_status[jint(argjson, "playerid")] = 1;
		} else {
			bytes = nn_send(bet->pubsock, cJSON_Print(argjson), strlen(cJSON_Print(argjson)), 0);
			if (bytes < 0) {
				retval = -1;
				printf("\nMehtod: %s Failed to send data", method);
				goto end;
			}
		}
	}
end:
	return retval;
}

void bet_dcv_backend_loop(void *_ptr)
{
	int32_t recvlen;
	cJSON *argjson = NULL;
	void *ptr = NULL;
	struct privatebet_info *bet = _ptr;

	pthread_mutex_lock(&mutex);

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
			if ((argjson = cJSON_Parse(tmp)) != 0) {
				if (bet_dcv_backend(argjson, bet, dcv_vars) != 0) // usually just relay to players
				{
					// Do Something
				}
				free_json(argjson);
			}
			if (tmp)
				free(tmp);
			if (ptr)
				nn_freemsg(ptr);
		}
	}
	pthread_mutex_unlock(&mutex);
}

void bet_dcv_frontend_loop(void *_ptr)
{
	struct lws_context_creation_info dcv_info;
	struct lws_context *dcv_context = NULL;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

	printf("\n%s::%d", __FUNCTION__, __LINE__);
	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws broker | visit http://localhost:1234\n");

	// for DCV
	memset(&dcv_info, 0, sizeof dcv_info); /* otherwise uninitialized garbage */
	dcv_info.port = 9000;
	dcv_info.mounts = &mount;
	dcv_info.protocols = protocols;
	dcv_info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	dcv_context = lws_create_context(&dcv_info);
	if (!dcv_context) {
		lwsl_err("lws init failed\n");
		printf("%s::%d::lws_context error", __FUNCTION__, __LINE__);
	}

	while (n >= 0 && !interrupted) {
		n = lws_service(dcv_context, 1000);
	}
	lws_context_destroy(dcv_context);
}

static int32_t bet_send_status(struct privatebet_info *bet)
{
	char name[10];
	cJSON *status_info = NULL;
	int bytes;
	int retval = 1;

	status_info = cJSON_CreateObject();
	cJSON_AddStringToObject(status_info, "method", "status_info");
	cJSON_AddNumberToObject(status_info, "bvv_status", bvv_status);

	for (int i = 0; i < bet->maxplayers; i++) {
		memset(name, 0x00, sizeof(name));
		snprintf(name, sizeof(name), "player_%d", i);
		cJSON_AddNumberToObject(status_info, name, player_status[i]);
	}

	bytes = nn_send(bet->pubsock, cJSON_Print(status_info), strlen(cJSON_Print(status_info)), 0);

	if (bytes < 0)
		retval = -1;

	return retval;
}

void bet_dcv_live_loop(void *_ptr)
{
	struct privatebet_info *bet = _ptr;
	cJSON *live_info = NULL;
	int32_t retval = 1;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "live");
	while (1) {
		bvv_status = 0;
		for (int i = 0; i < bet->maxplayers; i++)
			player_status[i] = 0;

		int bytes = nn_send(bet->pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
		if (bytes < 0)
			retval = -1;

		sleep(10);
		retval = bet_send_status(bet);

		if (retval < 0)
			break;
	}
}
