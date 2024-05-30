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

#ifndef BET_H
#define BET_H

#include <stdint.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../crypto777/OS_portable.h"

#include <libwebsockets.h>

#include "../external/dlg/include/dlg/dlg.h"
#include "../external/iniparser/src/iniparser.h"

#include "../external/nng/include/nng/compat/nanomsg/bus.h"
#include "../external/nng/include/nng/compat/nanomsg/nn.h"
#include "../external/nng/include/nng/compat/nanomsg/pair.h"
#include "../external/nng/include/nng/compat/nanomsg/pipeline.h"
#include "../external/nng/include/nng/compat/nanomsg/pubsub.h"
#include "../external/nng/include/nng/compat/nanomsg/reqrep.h"
#include "../external/nng/include/nng/compat/nanomsg/tcp.h"

#include "common.h"
#include "vdxf.h"
enum action_type { small_blind = 1, big_blind, check, raise, call, allin, fold };

enum card_type { burn_card = 0, hole_card, flop_card_1, flop_card_2, flop_card_3, turn_card, river_card };

enum poker_card_types { no_card_drawn = 0, poker_h1, poker_h2, poker_f1, poker_f2, poker_f3, poker_t, poker_r };

enum bet_warnings { seat_already_taken, insufficient_funds, table_is_full };

enum be_status { backend_not_ready = 0, backend_ready };

enum gui_status { gui_not_ready = 0, gui_ready };

enum bet_node { player = 0, dealer, cashier };

extern int32_t bet_node_type;

int32_t num_of_players;
char player_ids[CARDS777_MAXPLAYERS][MAX_ID_LEN];

struct BET_shardsinfo {
	UT_hash_handle hh;
	int32_t numcards, numplayers;
	uint8_t key[sizeof(bits256) + 2 * sizeof(int32_t)];
	bits256 recover, data[];
};

struct gfshare_ctx_bet {
	uint32_t sharecount, threshold, size, buffersize, allocsize;
	uint8_t sharenrs[255], buffer[];
};

struct privatebet_info {
	char game[64];
	bits256 MofN[CARDS777_MAXCARDS * CARDS777_MAXPLAYERS], cardpubs[CARDS777_MAXCARDS],
		playerpubs[CARDS777_MAXPLAYERS + 1], tableid, deckid;
	int32_t numplayers, maxplayers, numrounds, range, myplayerid, maxchips, chipsize;
	int32_t pullsock, pubsock, subsock, pushsock;
	uint32_t timestamp;
	char peerids[CARDS777_MAXPLAYERS + 1][67];
	int32_t cardid, turni;
	int32_t no_of_turns;
	cJSON *msg;
};

struct dcv_bvv_sock_info {
	int32_t pullsock, pubsock, subsock, pushsock;
};

struct privatebet_rawpeerln {
	uint64_t msatoshi_to_us, msatoshi_total;
	uint32_t unique_id;
	char peerid[67], channel[64], netaddr[64], state[32];
};

struct privatebet_peerln {
	int32_t numrhashes, numpaid;
	bits256 hostrhash, clientrhash, clientpubkey;
	struct privatebet_rawpeerln raw;
};

struct privatebet_vars {
	bits256 myhash, hashes[CARDS777_MAXPLAYERS + 1][2];
	int32_t permis[CARDS777_MAXPLAYERS + 1][CARDS777_MAXCARDS];
	uint32_t endround[CARDS777_MAXPLAYERS + 1], evalcrcs[CARDS777_MAXPLAYERS + 1], consensus;
	cJSON *actions[CARDS777_MAXROUNDS][CARDS777_MAXPLAYERS + 1];
	int32_t mypermi[CARDS777_MAXCARDS], permi[CARDS777_MAXCARDS], turni, round, validperms, roundready, lastround,
		numconsensus;
	int32_t small_blind, big_blind;
	int32_t betamount[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS];
	int32_t bet_actions[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS];
	int32_t dealer, last_turn, last_raise;
	int32_t pot;
	int32_t player_funds;
	int32_t funds[CARDS777_MAXPLAYERS];
	int32_t funds_spent[CARDS777_MAXPLAYERS];
	int32_t win_funds[CARDS777_MAXPLAYERS];
	int32_t ini_funds[CARDS777_MAXPLAYERS];
	int32_t winners[CARDS777_MAXPLAYERS];
	char player_chips_addrs[CARDS777_MAXPLAYERS][64];
	int32_t req_id_to_player_id_mapping[CARDS777_MAXPLAYERS];
};

struct pair256 {
	bits256 priv, prod;
};

struct privatebet_share {
	int32_t numplayers, range, myplayerid;
	int32_t pullsock, pubsock, subsock, pushsock;
	bits256 bvv_public_key;
	struct pair256 player_key;
};

struct enc_share {
	uint8_t bytes[sizeof(bits256) + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES];
};

extern char bvv_unique_id[65];

extern struct enc_share *g_shares;

extern struct enc_share *all_g_shares[CARDS777_MAXPLAYERS];

struct b_deck_info_struct {
	bits256 game_id;
	int32_t b_permi[CARDS777_MAXCARDS];
	struct pair256 cashier_r[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
};
extern struct b_deck_info_struct b_deck_info;

struct d_deck_info_struct {
	bits256 game_id;
	int32_t d_permi[CARDS777_MAXCARDS];
	struct pair256 dealer_r[CARDS777_MAXCARDS];
};
extern struct d_deck_info_struct d_deck_info;

struct p_deck_info_struct {
	int32_t player_id;
	bits256 game_id;
	struct pair256 p_kp;
	struct pair256 player_r[CARDS777_MAXCARDS];
};
extern struct p_deck_info_struct p_deck_info;

struct p_game_info_struct {
	int32_t card_state;
	int32_t cards[hand_size];
};
extern struct p_game_info_struct p_game_info;

struct game_meta_info_struct {
	int32_t num_players;
	int32_t dealer_pos;
	int32_t turn;
	int32_t card_id;
	int32_t card_state[CARDS777_MAXPLAYERS][hand_size];
};
extern struct game_meta_info_struct game_meta_info;

struct deck_player_info {
	struct pair256 player_key;
	bits256 cardpubkeys[CARDS777_MAXCARDS], cardprivkeys[CARDS777_MAXCARDS];
	int32_t permis[CARDS777_MAXCARDS], r_permis[CARDS777_MAXCARDS];
	bits256 cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 dcvpubkey, bvvpubkey, deckid;
	uint32_t numplayers, maxplayers, numcards;
};
extern struct deck_player_info player_info;

struct deck_dcv_info {
	bits256 deckid;
	struct pair256 dcv_key;
	int32_t permis[CARDS777_MAXCARDS];
	bits256 cardpubkeys[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 dcvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 peerpubkeys[CARDS777_MAXPLAYERS];
	uint32_t numplayers, maxplayers;
	unsigned char uri[CARDS777_MAXPLAYERS][100];
	uint32_t betamount;
	uint32_t commision;
	uint32_t paidamount;
	unsigned char bvv_uri[100];
};

struct deck_bvv_info {
	bits256 deckid;
	int32_t permis[CARDS777_MAXCARDS];
	struct pair256 bvv_key;
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	uint32_t numplayers, maxplayers;
};

struct cashier {
	int32_t c_pullsock, c_pubsock, c_subsock, c_pushsock;
	char addr[67];
	cJSON *msg;
};

struct seat_info {
	char seat_name[20];
	int32_t seat;
	int32_t chips;
	int32_t empty;
	int32_t playing;
};

extern int32_t player_pos[CARDS777_MAXPLAYERS];

extern struct seat_info player_seats_info[CARDS777_MAXPLAYERS];
extern struct privatebet_info *bet_player;
extern struct privatebet_vars *player_vars;

extern int32_t heartbeat_on;

extern char blockchain_cli[1024];
extern char *chips_cli;
extern char *verus_chips_cli;

struct float_num {
	uint32_t mantisa : 23;
	uint32_t exponent : 8;
	uint32_t sign : 1;
};

struct verus_player_config {
	char dealer_id[16];
	char table_id[16];
	char wallet_addr[64];
	char txid[128];
	char verus_pid[128];
};
extern struct verus_player_config player_config;

struct table {
	uint8_t max_players;
	struct float_num min_stake;
	struct float_num max_stake;
	struct float_num big_blind;
	char table_id[16];
	char dealer_id[16];
};
extern struct table player_t;

extern bits256 game_id;

extern int32_t hole_cards_drawn, community_cards_drawn, flop_cards_drawn, turn_card_drawn, river_card_drawn;
extern int32_t card_matrix[CARDS777_MAXPLAYERS][hand_size];
extern int32_t turn, no_of_cards, no_of_rounds, no_of_bets;

bits256 xoverz_donna(bits256 a);
bits256 crecip_donna(bits256 a);
bits256 fmul_donna(bits256 a, bits256 b);
bits256 card_rand256(int32_t privkeyflag, int8_t index);
struct pair256 deckgen_common(struct pair256 *randcards, int32_t numcards);
struct pair256 deckgen_player(bits256 *playerprivs, bits256 *playercards, int32_t *permis, int32_t numcards);
int32_t sg777_deckgen_vendor(int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards,
			     bits256 *playercards, bits256 deckid);

struct pair256 p2p_bvv_init(bits256 *keys, struct pair256 b_key, bits256 *blindings, bits256 *blindedcards,
			    bits256 *finalcards, int32_t numcards, int32_t numplayers, int32_t playerid,
			    bits256 deckid);

bits256 curve25519_fieldelement(bits256 hash);
void bet_bvv_thrd(char *dcv_ip, const int32_t port);

#endif
