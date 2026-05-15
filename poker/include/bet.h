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
#include <pthread.h>

// Include crypto headers directly (no OS_portable.h to avoid math.h conflicts)
#include "../../includes/curve25519.h"
#include "../../includes/tweetnacl.h"
#include "../../includes/utlist.h"
#include "../../includes/uthash.h"
#include "../../includes/bits256_utils.h"
#include "../../includes/cJSON.h"

#include <libwebsockets.h>

#include "dlg/dlg.h"
#include "iniparser.h"

// Nanomsg/nano sockets support completely removed - no longer used

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

/* These are globals; define them in exactly one .c file */
extern int32_t num_of_players;
extern char player_ids[CARDS_MAXPLAYERS][MAX_ID_LEN];

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
	bits256 MofN[CARDS_MAXCARDS * CARDS_MAXPLAYERS], cardpubs[CARDS_MAXCARDS],
		playerpubs[CARDS_MAXPLAYERS + 1], tableid, deckid;
	int32_t numplayers, maxplayers, numrounds, range, myplayerid, maxchips, chipsize;
	// Nanomsg sockets removed - no longer used
	uint32_t timestamp;
	char peerids[CARDS_MAXPLAYERS + 1][67];
	int32_t cardid, turni;
	int32_t no_of_turns;
	cJSON *msg;
};

// struct dcv_bvv_sock_info removed - nanomsg sockets no longer used

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
	bits256 myhash, hashes[CARDS_MAXPLAYERS + 1][2];
	int32_t permis[CARDS_MAXPLAYERS + 1][CARDS_MAXCARDS];
	uint32_t endround[CARDS_MAXPLAYERS + 1], evalcrcs[CARDS_MAXPLAYERS + 1], consensus;
	cJSON *actions[CARDS_MAXROUNDS][CARDS_MAXPLAYERS + 1];
	int32_t mypermi[CARDS_MAXCARDS], permi[CARDS_MAXCARDS], turni, round, validperms, roundready, lastround,
		numconsensus;
	/* All chip-denominated fields are integer "table chips" (1 CHIP = 100
	 * table chips). Conversion happens only at the on-chain boundaries -
	 * see chips_to_table_chips() / table_chips_to_chips() in common.h.
	 * Default blinds: small_blind=1, big_blind=2 (i.e. 0.01 / 0.02 CHIPS). */
	int64_t small_blind, big_blind;
	int64_t betamount[CARDS_MAXPLAYERS][CARDS_MAXROUNDS];
	int32_t bet_actions[CARDS_MAXPLAYERS][CARDS_MAXROUNDS];
	int32_t dealer, last_turn;
	int64_t last_raise;
	int64_t turn_start_time;    // Unix timestamp when current turn started
	int32_t turn_start_block;   // Block height when current turn started
	int64_t pot;
	int64_t player_funds;
	int64_t funds[CARDS_MAXPLAYERS];       // Available funds (table chips)
	int64_t funds_spent[CARDS_MAXPLAYERS]; // Total spent this game
	int64_t win_funds[CARDS_MAXPLAYERS];   // Winnings from pot
	int64_t ini_funds[CARDS_MAXPLAYERS];   // Initial funds (payin amount)
	int32_t winners[CARDS_MAXPLAYERS];
	char player_chips_addrs[CARDS_MAXPLAYERS][64];
	int32_t req_id_to_player_id_mapping[CARDS_MAXPLAYERS];
};

struct pair256 {
	bits256 priv, prod;
};

// struct privatebet_share removed - nanomsg sockets no longer used, struct was only for socket communication

struct enc_share {
	uint8_t bytes[sizeof(bits256) + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES];
};

extern char bvv_unique_id[65];

extern struct enc_share *g_shares;

extern struct enc_share *all_g_shares[CARDS_MAXPLAYERS];

struct b_deck_info_struct {
	bits256 game_id;
	int32_t b_permi[CARDS_MAXCARDS];
	struct pair256 cashier_r[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
};
extern struct b_deck_info_struct b_deck_info;

struct d_deck_info_struct {
	bits256 game_id;
	int32_t d_permi[CARDS_MAXCARDS];
	struct pair256 dealer_r[CARDS_MAXCARDS];
};
extern struct d_deck_info_struct d_deck_info;

struct p_deck_info_struct {
	int32_t player_id;
	bits256 game_id;
	struct pair256 p_kp;
	struct pair256 player_r[CARDS_MAXCARDS];
};
extern struct p_deck_info_struct p_deck_info;

// Player's local game state - persisted to local DB for rejoin
struct p_local_state_struct {
	char game_id[65];              // Current game ID
	char table_id[64];             // Table we're playing at
	char payin_tx[128];            // TX where player sent funds to cashier
	int32_t player_id;             // Our position at the table (0-8)
	int32_t decoded_cards[hand_size];  // Card values we've decoded (-1 = not decoded)
	int32_t cards_decoded_count;   // How many cards we've decoded
	int32_t last_card_id;          // Last card_id we processed
	int32_t last_game_state;       // Last known game state
	/* Hole cards stored by per-player slot (0 = first hole, 1 = second hole),
	 * not by global card_id. The global card_id encoding for hole cards is
	 * (hole_index * num_players) + player_index, so a 2-player table puts
	 * p1's hole cards at global ids 0,2 and p2's at 1,3 - which leaves holes
	 * in decoded_cards[] when keyed by card_id. The hole_cards[] array is the
	 * authoritative source for "what are my two hole cards" used to drive the
	 * GUI deal message and showdown display. -1 = not yet decoded.
	 */
	int32_t hole_cards[2];
	/* Community cards stored by board position (0..2 = flop, 3 = turn,
	 * 4 = river), not by global card_id. Like hole_cards[], this avoids
	 * the decoded_cards[card_id] indexing which collides with hole-card
	 * slots for multi-player games (community card_ids overlap slots
	 * already filled by hole_cards under the global-id-as-slot scheme).
	 * The board position is derived from card_type:
	 *     position = card_type - flop_card_1
	 * Used only by the GUI deal-board emit path. -1 = not yet decoded. */
	int32_t community_cards[5];
	/* Latest betting_state.turn_start_time observed by the player.
	 * Echoed back in every betting action write so the dealer can
	 * dedup: dealer's verus_poll_player_action ignores actions whose
	 * turn_start_time != current vars->turn_start_time. Without this
	 * the dealer reads the same stale on-chain action every 2 s poll
	 * and re-applies it, causing the pot to leak by `amount` per poll.
	 *
	 * Not persisted - resets to 0 on restart and is repopulated by
	 * the next betting_state read. */
	int64_t last_betting_turn_start_time;
};
extern struct p_local_state_struct p_local_state;

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
	int32_t card_state[CARDS_MAXPLAYERS][hand_size];
};
extern struct game_meta_info_struct game_meta_info;

struct deck_player_info {
	struct pair256 player_key;
	bits256 cardpubkeys[CARDS_MAXCARDS], cardprivkeys[CARDS_MAXCARDS];
	int32_t permis[CARDS_MAXCARDS], r_permis[CARDS_MAXCARDS];
	bits256 cardprods[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	bits256 bvvblindcards[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	bits256 dcvpubkey, bvvpubkey, deckid;
	uint32_t numplayers, maxplayers, numcards;
};
extern struct deck_player_info player_info;

struct deck_dcv_info {
	bits256 deckid;
	struct pair256 dcv_key;
	int32_t permis[CARDS_MAXCARDS];
	bits256 cardpubkeys[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	bits256 dcvblindcards[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	bits256 cardprods[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	bits256 peerpubkeys[CARDS_MAXPLAYERS];
	uint32_t numplayers, maxplayers;
	unsigned char uri[CARDS_MAXPLAYERS][100];
	uint32_t betamount;
	uint32_t commision;
	uint32_t paidamount;
	unsigned char bvv_uri[100];
};

struct deck_bvv_info {
	bits256 deckid;
	int32_t permis[CARDS_MAXCARDS];
	struct pair256 bvv_key;
	bits256 bvvblindcards[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
	uint32_t numplayers, maxplayers;
};

struct cashier {
	// Nanomsg sockets removed - no longer used
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

extern int32_t player_pos[CARDS_MAXPLAYERS];

extern struct seat_info player_seats_info[CARDS_MAXPLAYERS];
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
	char dealer_id[128];
	char table_id[128];
	char wallet_addr[64];
	char txid[128];
	char verus_pid[128];
	int32_t ws_port;  // WebSocket port for GUI mode
};
extern struct verus_player_config player_config;

struct table {
	uint8_t max_players;
	struct float_num min_stake;
	struct float_num max_stake;
	struct float_num big_blind;
	char table_id[64];
	char dealer_id[64];
	char cashier_id[64];  // Trusted cashier handling player funds
	int32_t start_block;  // Block height when table was started (for polling)
};
extern struct table player_t;

extern bits256 game_id;

extern int32_t hole_cards_drawn, community_cards_drawn, flop_cards_drawn, turn_card_drawn, river_card_drawn;
extern int32_t card_matrix[CARDS_MAXPLAYERS][hand_size];
extern int32_t turn, no_of_cards, no_of_rounds, no_of_bets;

bits256 xoverz_donna(bits256 a);
bits256 crecip_donna(bits256 a);
bits256 fmul_donna(bits256 a, bits256 b);
bits256 card_rand256(int32_t privkeyflag, int8_t index);
struct pair256 deckgen_common(struct pair256 *randcards, int32_t numcards);
struct pair256 deckgen_player(bits256 *playerprivs, bits256 *playercards, int32_t *permis, int32_t numcards);
int32_t deckgen_vendor(int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards,
			     bits256 *playercards, bits256 deckid);

struct pair256 p2p_bvv_init(bits256 *keys, struct pair256 b_key, bits256 *blindings, bits256 *blindedcards,
			    bits256 *finalcards, int32_t numcards, int32_t numplayers, int32_t playerid,
			    bits256 deckid);

bits256 curve25519_fieldelement(bits256 hash);
// bet_bvv_thrd removed - nanomsg/pub-sub communication no longer used

#endif
